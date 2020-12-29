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
        async_read_pop(const struct epoll_event & ev, std::function<void(int, char *, size_t len)> callback){
            ssize_t len = buffer.read_chunk(ev.data.fd);
            callback(ev.data.fd, buffer.get_buf(), len);
        }
        void
        clear_session(){
            epoll.DelEvent(sessionfd);
            close(sessionfd);
        }

        void
        async_write(void * data, size_t len, std::function<void(Error & /*error*/, int /*fd*/, ssize_t /*len */)> callback){
            Error error;
            auto result = write(sessionfd, data, len);
            if(result == -1){
                error = Error{strerror(errno)};
            }
            callback(error, sessionfd, result);
        }
    };
}
