#pragma once

#include "EventCLoop/EventCLoop.hpp"
#include "define.hpp"

namespace simplemsgq
{
    class ClientConsumer{
        EventCLoop::Epoll & epoll;
        EventCLoop::TcpConnect connector;
        std::function<void(EventCLoop::Error & )> connect_callback;
        std::function<void(int , char *, size_t)> read_callback;


    public:
        ClientConsumer(EventCLoop::Epoll & epoll, uint16_t port, const std::string & ip)
            : epoll{epoll}
            , connector{epoll, port, ip}
            , connect_callback{nullptr}
            , read_callback{nullptr} { }

        void
        send_consume(int offset, int count){
            SIMPLEMSGQ_HEADER request;
            request.init();
            request.frame.packet_len = sizeof(SIMPLEMSGQ_HEADER);
            request.sequence = 0;
            request.type = 0;
            request.name = 0;
            request.code = 0;
            request.offset = offset;
            request.count = count;
            request.hton();

            connector.async_write(&request, sizeof(request), 
                [](EventCLoop::Error & error, int fd, ssize_t len){
                    if(error){
                        std::cout << "write error : " << error.what() << std::endl;
                    }
                });
            
        }
        void
        set_connect_callback(std::function<void(EventCLoop::Error & )> cb){
            connect_callback = cb;
        }
        void
        set_read_callback(std::function<void(int , char *, size_t)> cb){
            read_callback = cb;
        }

        void 
        run(){
            async_connect();
        }
        void
        async_connect() {
            connector.async_connect([this](EventCLoop::Error & error){
                if(connect_callback != nullptr)
                    connect_callback(error);
            });
        }
        void
        async_read(){
            connector.async_read([this](int fd, char * buffer, size_t len){
                handle_read(fd, buffer, len);
            });
        }

    private:
        void
        handle_connect(EventCLoop::Error error){
            if(error){
                std::cout << "[CONNECT] error..." << error.what() << std::endl;
                auto timer = std::make_shared<EventCLoop::Timer>(epoll);
                timer->initOneTimer(1, 0);
                timer->async_wait([timer, this](EventCLoop::Error & error) {
                });
                return;
            }
            async_read();
        }

        void
        handle_read(int sessionfd, char * buffer, size_t len){
            if(len == 0){
                async_connect();
                return;
                // async_connect();
                // throw std::runtime_error("TODO retry connect handle_read len == 0");
            }
            std::cout << "[" << sessionfd << "] handle_read len: " << len << std::endl;

            while(1){
                std::cout << "while start" << std::endl;
                char * p = nullptr;
                using std::placeholders::_1;
                using std::placeholders::_2;
                auto mylen = connector.buffer.dispatch_chunk(p, std::bind(&ClientConsumer::dispatch_packet_header, this, _1, _2));
                std::cout << "[" << sessionfd << "] dispatch_chunk mylen: " << mylen << ", p: " << (void *)p << std::endl;

                if(mylen == 0) { // TODO Need more
                    break;
                }
                if(mylen == FRAME_SIZE){ // ! Frame Header Fail
                    continue;
                }

                if(p == nullptr){
                    throw std::runtime_error("TODO p == nullptr ");
                }

                SIMPLEMSGQ_HEADER * header = (SIMPLEMSGQ_HEADER *)p;
                header->ntoh();
                
                if(read_callback != nullptr)
                    read_callback(sessionfd, p, mylen);
                
                std::cout << "[CLIENT_CONSUMER][RECV] offset:" << header->offset << ", count:" << header->count << std::endl;
            }
            std::cout << "while exit.." << std::endl;
        }

        int 
        dispatch_packet_header(char * first, size_t len){
            SIMPLEMSGQ_HEADER * header = (SIMPLEMSGQ_HEADER *) first;
            if(len < FRAME_SIZE){
                return 0;
            }

            if(!header->check()){
                // throw std::runtime_error("TODO header check fail ");
                return FRAME_SIZE; // TODO error?
            }

            auto packetlen = ntohl(header->frame.packet_len);

            if(len >= packetlen){
                return static_cast<int>(packetlen);
            }
            else {
                return 0;
            }
        }

    };
}