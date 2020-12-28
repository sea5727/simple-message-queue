#pragma once

#include "Epoll.hpp"
#include "Event.hpp"

namespace EventCLoop
{
    class Acceptor {
        Epoll & epoll;
        std::string ip;
        uint16_t port;
        int listenfd;
        
    public:
        Acceptor(Epoll & epoll, uint16_t port, const std::string ip, bool reuse = true)
            : epoll{epoll}
            , port{port}
            , ip{ip} {
            
            listenfd = socket(AF_INET, SOCK_STREAM, 0);
            if(listenfd == -1){
                throw std::logic_error(std::string{"Acceptor socket error : "} + std::string{strerror(errno)});
            }

            if(reuse) {
                int opt_value = 1;
                setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_value, sizeof(opt_value));
            }
            struct sockaddr_in _server_addr;
            
            _server_addr.sin_family = AF_INET;
            _server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // TODO BIND IP 할당 필요
            _server_addr.sin_port = htons(port);


            if(bind(listenfd, (struct sockaddr *)&_server_addr, sizeof(_server_addr)) == -1) {
                close(listenfd);
                throw std::logic_error(std::string{"Acceptor bind error : "} + std::string{strerror(errno)});
            }

            if(listen(listenfd, 5) == -1) {
                close(listenfd);
                throw std::logic_error(std::string{"Acceptor listen error : "} + std::string{strerror(errno)});
            }
        }

        void 
        async_accept(std::function<void(int, std::string, uint16_t)> callback){
            auto event = std::make_shared<Event>();
            event->fd = listenfd;
            event->pop = [this, callback](struct epoll_event ev){
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(struct sockaddr_in);
                auto sessionfd = ::accept(listenfd, (struct sockaddr *)&client_addr, &len);
                if(sessionfd == -1)
                    throw std::logic_error(std::string{"accept fail "} + std::string{strerror(errno)});
                char *ip = inet_ntoa(client_addr.sin_addr);
                uint16_t port = htons(client_addr.sin_port);

                std::cout << "tcp accept : " << ip << ":" << port << std::endl;
                callback(sessionfd, ip, port);
            };

            struct epoll_event ev;
            ev.data.fd = listenfd;
            ev.events = EPOLLIN;

            epoll.AddEvent(event, ev);
        }

        void
        accept(struct epoll_event ev){

            struct sockaddr_in client_addr;
            socklen_t len = sizeof(struct sockaddr_in);
            auto sessionfd = ::accept(listenfd, (struct sockaddr *)&client_addr, &len);
            if(sessionfd == -1)
                throw std::logic_error(std::string{"accept fail "} + std::string{strerror(errno)});
            char *ip = inet_ntoa(client_addr.sin_addr);
            uint16_t port = htons(client_addr.sin_port);

            std::cout << "tcp accept : " << ip << ":" << port << std::endl;
            // callback(sessionfd, ip, port);
        }
    };



}
