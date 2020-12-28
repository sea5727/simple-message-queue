#pragma once

#include "EventCLoop/EventCLoop.hpp"
#include "define.hpp"

namespace simplemsgq
{
    class ServerProducer{
        EventCLoop::Epoll & epoll;
        EventCLoop::Acceptor acceptor;
        std::map<int, std::shared_ptr<EventCLoop::TcpSession>> sessions;
    public:
        ServerProducer(EventCLoop::Epoll & epoll, uint16_t port, const std::string & ip)
            : epoll{epoll}
            , acceptor{epoll, port, ip} 
            , sessions{}
            {}

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
            std::cout << "[" << sessionfd << "] handle_read len: " << len << std::endl;
            if(len == 0){
                sessions.erase(sessionfd);
                return;
            }
            auto session = sessions.at(sessionfd);

            while(1){
                std::cout << "while start" << std::endl;
                char * p = nullptr;
                using std::placeholders::_1;
                using std::placeholders::_2;
                auto mylen = session->buffer.dispatch_chunk(p, std::bind(&ServerProducer::dispatch_packet_header, this, _1, _2));
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
                
                std::cout << "[RECV] offset:" << header->offset << ", count:" << header->count << std::endl;
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