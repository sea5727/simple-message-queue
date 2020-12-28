#pragma once

#include "Epoll.hpp"
#include "Event.hpp"
#include "TcpBuffer.hpp"



namespace EventCLoop
{
    class TcpSession{
    public:
        Epoll & epoll;
        int sessionfd;
        TcpBuffer buffer;

        TcpSession(Epoll & epoll, int sessionfd)
            : epoll{epoll}
            , sessionfd{sessionfd} {
                std::cout << "default TcpSession Create\n";
        }

        TcpSession(Epoll & epoll)
            : epoll{epoll}
            , sessionfd{sessionfd} {
            
            sessionfd = socket(AF_INET, SOCK_STREAM, 0);
            if(sessionfd < 0){
                throw std::logic_error("socket create fail " + std::string{strerror(errno)});
            }
        }

        ~TcpSession(){
            std::cout << "~Delete TcpSession" << std::endl;
            // epoll.DelEvent(sessionfd);
        }

        void
        async_read(std::function<void(int, char *, size_t len)> callback){
            auto event = std::make_shared<Event>();
            event->fd = sessionfd;
            event->pop = [this, callback](struct epoll_event ev){
                std::cout << "[TcpSession] epoll pop !! ev : " << ev.events << std::endl;
                ssize_t len = buffer.read_chunk(ev.data.fd);
                callback(ev.data.fd, buffer.get_buf(), len);
                if(len == 0){
                    auto ret = epoll.DelEvent(ev.data.fd);
                    close(ev.data.fd);
                    // callback(ev.data.fd, buffer.get_buf(), len); 
                    return;
                }
                else {
                    // callback(ev.data.fd, buffer.get_buf(), len); 
                }

                
            };

            struct epoll_event ev;
            ev.data.fd = sessionfd;
            ev.events = EPOLLIN;

            epoll.AddEvent(event, ev);

        }
    };
}
