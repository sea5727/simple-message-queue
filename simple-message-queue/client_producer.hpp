#pragma once

#include "EventCLoop/EventCLoop.hpp"
#include "simplemsgq.hpp"

namespace simplemsgq
{
    class ClientProducer{
        EventCLoop::Epoll & epoll;
        std::string ip;
        uint16_t port;
        EventCLoop::TcpConnector connector;
        std::unique_ptr<EventCLoop::TcpSession> session;
        std::function<void(EventCLoop::Error & /*error*/ , int fd)> connect_callback;
        std::function<void(int , char *, size_t)> read_callback;

    public:
        ClientProducer(EventCLoop::Epoll & epoll, uint16_t port, const std::string & ip)
            : epoll{epoll}
            , ip{ip}
            , port{port}
            , connector{epoll}
            , connect_callback{nullptr}
            , read_callback{nullptr} { }

        void
        send_produce(void * data, size_t datalen){
            SIMPLEMSGQ_HEADER request;
            request.init();
            request.frame.packet_len = sizeof(SIMPLEMSGQ_HEADER) + datalen;
            request.sequence = 0;
            request.type = 0;
            request.name = 0;
            request.code = 0;
            request.offset = 0;
            request.count = 1;
            request.hton();

            struct iovec iovec[2];
            iovec[0].iov_base = &request;
            iovec[0].iov_len = sizeof(SIMPLEMSGQ_HEADER);
            iovec[1].iov_base = data;
            iovec[1].iov_len = datalen;
            
            session->async_writev(iovec, 2, 
                [](EventCLoop::Error & error, int fd, ssize_t len){
                    if(error){
                        std::cout << "write error : " << error.what() << std::endl;
                        return;
                    }
                    std::cout << "write success len:" << len << std::endl; 
                });
            
        }
        void
        set_connect_callback(std::function<void(EventCLoop::Error & /*error*/, int fd )> cb){
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
            connector.async_connect(ip, port, 
                [this](EventCLoop::Error & error, int fd){

                if(error){
                    std::cout << "[CONNECT] retry connect because : " << error.what() << std::endl;
                    do_reconnect_with_interval(1, 0);
                    return;
                }
                if(connect_callback != nullptr)
                    connect_callback(error, fd);
                session = std::make_unique<EventCLoop::TcpSession>(epoll, fd);
                do_read();
            });
        }
        void
        do_reconnect_with_interval(unsigned int sec, unsigned int nsec){
            auto timer = std::make_shared<EventCLoop::Timer>(epoll);
            timer->initOneTimer(sec, nsec);
            timer->async_wait(
                [timer, this](EventCLoop::Error & error) { 
                    async_connect();
                });
        }
        void
        do_read(){
            session->async_read(
                [this](int fd, char * buffer, size_t len){
                    std::cout << "read... fd : " << fd << ", len : " << len << std::endl;
                    if(len == 0){
                        std::cout << "[CONNECT] retry connect because : closed peer " << std::endl;
                        session.reset();
                        do_reconnect_with_interval(1, 0);
                        return;
                    }
                    handle_read(fd, buffer, len);
                });
        }
    private:
        void
        handle_read(int sessionfd, char * buffer, size_t len){
            while(1){
                char * p = nullptr;
                using std::placeholders::_1;
                using std::placeholders::_2;
                auto mylen = session->buffer.dispatch_chunk(p, std::bind(&ClientProducer::dispatch_packet_header, this, _1, _2));
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
                
                std::cout << "[CLIENT_PRODUCER][RECV] offset:" << header->offset << ", count:" << header->count << std::endl;
            }
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