#pragma once

#include "EventCLoop/EventCLoop.hpp"
#include "define.hpp"

namespace simplemsgq
{
    class ServerConsumer{

        EventCLoop::Epoll & epoll;
        EventCLoop::Acceptor acceptor;
        std::map<int, std::shared_ptr<EventCLoop::TcpSession>> sessions;
        std::shared_ptr<FileManager> fm;
    public:
        ServerConsumer(EventCLoop::Epoll & epoll, uint16_t port, const std::string & ip, std::shared_ptr<FileManager> fm)
            : epoll{epoll}
            , acceptor{epoll, port, ip} 
            , sessions{}
            , fm{fm} { }
    
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
                std::cout << "[" << fd << "] handle_read len: " << len << std::endl;
                if(len == 0){
                    sessions.erase(fd);
                    return;
                }
                auto session = sessions.at(fd);

                while(1){
                    char * p = nullptr;
                    using std::placeholders::_1;
                    using std::placeholders::_2;
                    auto mylen = session->buffer.dispatch_chunk(p, std::bind(&ServerConsumer::dispatch_packet_header, this, _1, _2));
                    std::cout << "[" << fd << "] dispatch_chunk mylen: " << mylen << ", p: " << (void *)p << std::endl;

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
                    
                    std::cout << "[RECV] offset:" << header->offset << ", count:" << header->count << std::endl;

                    if(fm){
                        auto sendinfo = (*fm).select_data(header->offset, header->count);
                        auto filefd = std::get<0>(sendinfo);
                        auto send_position = std::get<1>(sendinfo);
                        auto send_size = std::get<2>(sendinfo);

                        SIMPLEMSGQ_HEADER response;
                        response.init();
                        response.frame.packet_len = sizeof(SIMPLEMSGQ_HEADER) + send_size;
                        response.sequence = header->sequence;
                        response.type = header->type;
                        response.name = header->name;
                        response.code = filefd == -1 ? -1 : 0;
                        response.offset = header->offset;
                        response.count = send_size == 0 ? 0 : 1;
                        response.hton();

                        session->async_write(&response, sizeof(SIMPLEMSGQ_HEADER), 
                            [=](EventCLoop::Error & error, int fd, ssize_t len){
                                if(error){
                                    if(filefd > 0) close(filefd);
                                    return;
                                }
                                if(send_size <= 0 || filefd == -1) { 
                                    return ; 
                                }
                                auto ref_send_pos = send_position;
                                auto result = ::sendfile(fd, filefd, &ref_send_pos, send_size);
                                close(filefd);
                            });
                    }
                }
            });
        }

        int 
        dispatch_packet_header(char * first, size_t len){
            SIMPLEMSGQ_HEADER * header = (SIMPLEMSGQ_HEADER *) first;
            if(len < FRAME_SIZE){
                return 0;
            }

            if(!header->check()){
                throw std::runtime_error("TODO header check fail ");
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