#pragma once

#include "EventCLoop.hpp"

namespace EventCLoop
{
    class TcpSession{
    public:
        Epoll & epoll;
        Event event;
        int sessionfd;
        TcpBuffer buffer;

        TcpSession(Epoll & epoll, int sessionfd)
            : epoll{epoll}
            , event{}
            , sessionfd{sessionfd}
            , buffer{} {  }


        ~TcpSession(){
            // std::cout << "default TcpSession Delete\n";
            // epoll.DelEvent(sessionfd);
            if(!event.isCleared()){
                epoll.DelEvent(event.fd);
                close(event.fd);
                event.clear();
            }
        }

        void
        async_read(std::function<void(int, char *, size_t len)> callback){
            using std::placeholders::_1;
            event.fd = sessionfd;
            event.pop = std::bind(&TcpSession::async_read_pop, this, _1, callback);

            struct epoll_event ev;
            ev.data.fd = sessionfd;
            ev.events = EPOLLIN;

            epoll.AddEvent(event, ev);
        }
        void
        async_read_pop(
            const struct epoll_event & ev, 
            std::function<void(int /*fd*/, char * /*buffer*/, size_t /*len*/)> callback){

            ssize_t len = buffer.read_chunk(ev.data.fd);
            callback(ev.data.fd, buffer.get_buf(), len);
        }
        void
        clear_session(){
            if(!event.isCleared()){
                epoll.DelEvent(event.fd);
                close(event.fd);
                event.clear();
            }
        }
        void
        async_writev(
            const struct iovec *iovecs, 
            int count, 
            std::function<void(Error & /*error*/, int /*fd*/, size_t /*len */)> callback){
                
            Error error;
            auto result = writev(sessionfd, iovecs, count);
            if(result == -1){
                error = Error{strerror(errno)};
            }
            callback(error, sessionfd, result);
        }
        void
        async_write(
            void * data, 
            size_t len, 
            std::function<void(Error & /*error*/, int /*fd*/, size_t /*len */)> callback){
                
            Error error;
            auto result = write(sessionfd, data, len);
            if(result == -1){
                error = Error{strerror(errno)};
            }
            callback(error, sessionfd, result);
        }

        void
        make_sockaddr_struct(
            struct sockaddr_in & server_addr, 
            const std::string & ip, 
            const uint16_t port){
            if(inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0){
                throw std::logic_error("socket create fail" + std::string{strerror(errno)});
            }
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
        }
    };
}
