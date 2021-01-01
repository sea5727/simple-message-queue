#pragma once

#include "EventCLoop.hpp"

namespace EventCLoop
{
    class Eventfd{
    public:
        Epoll & epoll;
        int event_fd;
        Event event;
    public:
        Eventfd(Epoll & epoll)
            : epoll{epoll}
            , event_fd{-1}
            , event{} {

            std::cout << "[Eventfd] create eventfd : " << event_fd << std::endl;

            event_fd = eventfd(0, EFD_NONBLOCK);
            if(event_fd == -1)
                throw std::runtime_error(std::string{"eventfd create fail "} + std::string{strerror(errno)});

            std::cout << "[Eventfd] create eventfd : " << event_fd << std::endl;

            event.fd = event_fd;
            event.pop = nullptr;

            struct epoll_event ev;
            ev.data.fd = event_fd;
            ev.events = EPOLLIN;

            epoll.AddEvent(event, ev);

        }
        ~Eventfd(){
            std::cout << "Destructor Eventfd " << std::endl;
            if(!event.isCleared()){
                epoll.DelEvent(event.fd);
                close(event.fd);
                event.clear();
            }
        }

        void
        SendEvent(std::function<void()> callback){
            std::cout << "[Eventfd] SendEvent event_fd :" << event_fd << std::endl;
            uint64_t count = 1;
            Error error;
            ssize_t ret = write(event_fd, &count, sizeof(uint64_t));
            if(ret == -1){
                error = Error{strerror(errno)};
            }

            using std::placeholders::_1;
            event.fd = event_fd;
            event.pop = std::bind(&Eventfd::SendEventPop, this, _1, callback);

            struct epoll_event ev;
            ev.data.fd = event_fd;
            ev.events = EPOLLIN ;

            epoll.ModEvent(event_fd, ev);

        }

        void
        SendEventPop(const struct epoll_event & ev, std::function<void()> callback){
            std::cout << "[Eventfd] SendEventPop event_fd :" << event_fd << std::endl;
            uint64_t res;
            int ret = read(event_fd, &res, sizeof(uint64_t));
            std::cout << "SendEventPop : " << ret << ", res : " << res << std::endl;
            for(int i = 0; i < res ; ++i){
                callback();
            }

            
        }

    };
}