#pragma once



#include "Error.hpp"
#include "Epoll.hpp"
#include "Event.hpp"

namespace EventCLoop
{
    class TcpConnect{
    private:
        Epoll & epoll;
        uint16_t port;
        std::string ip;        
        int sessionfd;
    public:
        TcpBuffer buffer;
    public:
          TcpConnect() = default;
          TcpConnect(Epoll & epoll, uint16_t port, const std::string & ip)
            : epoll{epoll}
            , port{port}
            , ip{ip}
            , buffer{}
            , sessionfd{-1} { }

        void
        async_connect(std::function<void(Error)> callback){

            sessionfd = socket(AF_INET, SOCK_STREAM, 0);
            if(sessionfd < 0){
                throw std::logic_error("socket create fail" + std::string{strerror(errno)});
            }
            std::cout << "[CONNECT] fd : " << sessionfd << std::endl;

            struct sockaddr_in server_addr;
            make_sockaddr_struct(server_addr);

            int flags = fcntl(sessionfd, F_GETFL, 0);
            fcntl(sessionfd, F_SETFL, flags | O_NONBLOCK );

            auto ret = ::connect(sessionfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
            auto event = std::make_shared<Event>();
            event->fd = sessionfd;
            event->pop = [this, callback](struct epoll_event ev){
                struct sockaddr_in server_addr;
                if(ev.events & EPOLLERR){
                    std::cout << "[CONNECT] ERROR ? \n";
                    int nerror;
                    socklen_t len = sizeof(nerror);
                    if(getsockopt(sessionfd, SOL_SOCKET, SO_ERROR, &nerror, &len) < 0){
                        throw std::logic_error("TcpConnect EPOLLERR getsockopt error:" + std::string{strerror(errno)});
                    }
                    if(nerror != 0){
                        make_sockaddr_struct(server_addr);
                        ::connect(sessionfd,  (struct sockaddr *)&server_addr, sizeof(server_addr));
                        auto error = Error{strerror(errno)};
                        auto ret = epoll.DelEvent(sessionfd);
                        close(sessionfd);
                        callback(error);
                        return;
                        
                    }
                    else{

                        auto ret = epoll.DelEvent(sessionfd);
                        close(sessionfd);
                        auto error = Error{"Unknown error"};
                        callback(error);
                        // noerror?
                    }
                    
                    return;
                }
                else if(ev.events & EPOLLIN){ // already connected
                    make_sockaddr_struct(server_addr);
                    auto ret = ::connect(sessionfd,  (struct sockaddr *)&server_addr, sizeof(server_addr));
                    auto error = Error{strerror(errno)};
                    auto save = epoll.DelEvent(sessionfd);
                    close(sessionfd);
                    callback(error);
                }
                else{ // success
                    auto error = Error{};
                    callback(error);
                }
            };


            struct epoll_event ev;
            ev.data.fd = sessionfd;
            ev.events = EPOLLIN | EPOLLOUT | EPOLLERR;

            epoll.AddEvent(event, ev);
        }

        void
        async_read(std::function<void(int , char *, size_t len)> callback){
            auto event = std::make_shared<Event>();
            event->fd = sessionfd;
            event->pop = [this, callback](struct epoll_event ev){
                std::cout << "session pop.. event : " << ev.events << std::endl; 
                ssize_t len = buffer.read_chunk(ev.data.fd);
                std::cout << "len : " << len << std::endl;
                if(len == 0){
                    auto ret = epoll.DelEvent(ev.data.fd);
                    close(ev.data.fd);
                    callback(ev.data.fd, buffer.get_buf(), len); 
                    return;
                }
                else {
                    callback(ev.data.fd, buffer.get_buf(), len); 
                }
            };

            struct epoll_event ev;
            ev.data.fd = sessionfd;
            ev.events = EPOLLIN;

            epoll.ModEvent(event, ev);

        }

        void
        make_sockaddr_struct(struct sockaddr_in & server_addr){
            if(inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0){
                throw std::logic_error("socket create fail" + std::string{strerror(errno)});
            }
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
        }

    };
}
