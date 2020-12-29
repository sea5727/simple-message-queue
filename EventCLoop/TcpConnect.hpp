#pragma once

#include "EventCLoop.hpp"

namespace EventCLoop
{
    class TcpConnect{
        Epoll & epoll;
        TcpBuffer buffer;
        Event event;
        uint16_t port;
        std::string ip;
        int sessionfd;

        
    public:
          TcpConnect() = default;
          TcpConnect(Epoll & epoll, uint16_t port, const std::string & ip)
            : epoll{epoll}
            , buffer{}
            , event{}
            , port{port} 
            , ip{ip}
            , sessionfd{-1}
            { }

        void
        async_connect(std::function<void(Error)> callback){

            sessionfd = socket(AF_INET, SOCK_STREAM, 0);
            if(sessionfd < 0){
                throw std::logic_error("socket create fail" + std::string{strerror(errno)});
            }
            std::cout << "[CONNECT] fd : " << sessionfd << std::endl;

            int flags = fcntl(sessionfd, F_GETFL, 0);
            fcntl(sessionfd, F_SETFL, flags | O_NONBLOCK );

            struct sockaddr_in server_addr;
            make_sockaddr_struct(server_addr, ip, port);

            auto ret = ::connect(sessionfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

            event.fd = sessionfd;
            using std::placeholders::_1;
            event.pop = std::bind(&TcpConnect::async_connect_pop, this, _1, callback);

            struct epoll_event ev;
            ev.data.fd = sessionfd;
            ev.events = EPOLLIN | EPOLLOUT | EPOLLERR; // TODO EPOLLERr 은 하지 않아도 될거같은데

            epoll.AddEvent(event, ev);
        }

        void
        async_connect_pop(const struct epoll_event & ev, std::function<void(Error)> callback){
            struct sockaddr_in server_addr;
            auto error = Error{};
            if(ev.events & EPOLLERR){
                std::cout << "[CONNECT] ERROR ? \n";
                // TODO 주석 제거 
                error = Error{strerror(errno)};
                epoll.DelEvent(sessionfd);
                event.clear();
                close(sessionfd);
                callback(error);
            }
            else if(ev.events & EPOLLIN){ // already connected ?? 
                std::cout << "[CONNECT] EPOLLIN ? \n";
                // // make_sockaddr_struct(server_addr);
                // // auto ret = ::connect(sessionfd,  (struct sockaddr *)&server_addr, sizeof(server_addr));
                // // auto error = Error{strerror(errno)};
                epoll.DelEvent(sessionfd);
                event.clear();
                close(sessionfd);
                callback(error);
            }
            else{
                epoll.DelEvent(sessionfd);
                event.clear();
                callback(error);
            }
        }

        void
        async_read(std::function<void(int , char *, size_t len)> callback){
            using std::placeholders::_1;
            event.fd = sessionfd;
            event.pop = std::bind(&TcpConnect::async_read_pop, this, _1, callback);

            struct epoll_event ev;
            ev.data.fd = sessionfd;
            ev.events = EPOLLIN;

            epoll.AddEvent(event, ev);
        }
        void
        async_read_pop(const struct epoll_event & ev, std::function<void(int , char *, size_t len)> callback){
            ssize_t len = buffer.read_chunk(ev.data.fd);
            callback(ev.data.fd, buffer.get_buf(), len); 
        }
        void
        clear_session(){
            epoll.DelEvent(sessionfd);
            close(sessionfd);
        }


        void
        async_write(char * data, size_t len, std::function<void(Error, int)> callback){
            Error error;
            auto result = write(sessionfd, data, len);
            if(result == -1){
                error = Error{strerror(errno)};
            }
            callback(error, result);
        }

        void
        make_sockaddr_struct(struct sockaddr_in & server_addr, const std::string & ip, const uint16_t port){
            if(inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0){
                throw std::logic_error("socket create fail" + std::string{strerror(errno)});
            }
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
        }

    };
}
