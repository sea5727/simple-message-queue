#pragma once

#include "EventCLoop/EventCLoop.hpp"
#include "simplemsgq.hpp"

namespace simplemsgq
{
    class ServerProducer{
        EventCLoop::Epoll & epoll;
        EventCLoop::Acceptor acceptor;
        std::map<int, std::shared_ptr<EventCLoop::TcpSession>> sessions;
        std::shared_ptr<FileManager> fm;
    public:
        ServerProducer(EventCLoop::Epoll & epoll, uint16_t port, const std::string & ip, std::shared_ptr<FileManager> fm)
            : epoll{epoll}
            , acceptor{epoll, port, ip} 
            , sessions{}
            , fm{fm} {}

        void
        run(){
            do_accept();
        }
        void
        do_accept(){
            acceptor.async_accept([this](int sessionfd, std::string ip, uint16_t port){
                handle_accept(sessionfd, ip, port);
            });
        }
        void
        handle_accept(int sessionfd, std::string ip, uint16_t port){

            auto session = std::make_shared<EventCLoop::TcpSession>(epoll, sessionfd);
            sessions.insert(std::make_pair(sessionfd, session));
            session->async_read([this](int fd, char * buffer, size_t len){
                handle_read(fd, buffer, len);
            });
        }

        void
        handle_read(int sessionfd, char * buffer, size_t len){
            std::cout << "[SERVER_PRODUCER][" << sessionfd << "] handle_read len: " << len << std::endl;
            auto session = sessions.at(sessionfd);
            if(len == 0){
                session->clear_session();
                sessions.erase(sessionfd);
                return;
            }
            

            while(1){
                char * p = nullptr;
                using std::placeholders::_1;
                using std::placeholders::_2;
                auto mylen = session->buffer.dispatch_chunk(p, std::bind(&ServerProducer::dispatch_packet_header, this, _1, _2));
                std::cout << "[SERVER_PRODUCER][" << sessionfd << "] dispatch_chunk mylen: " << mylen << ", p: " << (void *)p << std::endl;

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
                auto bodylen = header->get_body_len();

                std::cout << "[SERVER_PRODUCER][RECV] bodylen : " << bodylen << ", offset:" << header->offset << ", count:" << header->count << std::endl;

                if(bodylen > 0 && fm){
                    char * data = (char *)header + sizeof(SIMPLEMSGQ_HEADER); 
                    auto ret = (*fm).insert_data(data, bodylen);

                    SIMPLEMSGQ_HEADER response;
                    response.init();
                    response.frame.packet_len = sizeof(SIMPLEMSGQ_HEADER);
                    response.sequence = header->sequence;
                    response.type = header->type;
                    response.name = header->name;
                    response.code = ret == bodylen ? 0 : -1;
                    response.offset = header->offset;
                    response.count = header->count;
                    response.hton();

                    session->async_write(&response, sizeof(SIMPLEMSGQ_HEADER), 
                        [=](EventCLoop::Error & error, int fd, ssize_t len){
                            if(error){
                                std::cout << "[SERVER_PRODUCER] async_write fail " << error.what() << std::endl;
                                return;
                            }
                            std::cout << "[SERVER_PRODUCER] async_write success len:" << len << std::endl;
                        });

                }
                
                
                
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