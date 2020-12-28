#pragma once

#include "Event.hpp"

namespace EventCLoop{
    class Epoll {
        constexpr static unsigned int EpollSize = 1024;
        unsigned int index[EpollSize * 64] = {0,};
        int epollfd;
        std::map<int, std::shared_ptr<Event>> events;
    public:
        Epoll(){
            epollfd = epoll_create1(0);
            if(epollfd == -1){
                throw std::logic_error("epoll_create1 fail");
            }
        }

        void
        AddEvent(std::shared_ptr<Event> event, struct epoll_event ev){
            std::cout << " ========== Call AddEvent : " << event->fd << std::endl;
            if(epoll_ctl(epollfd, EPOLL_CTL_ADD, event->fd, &ev) == -1){
                throw std::logic_error(std::string{"epoll_ctl EPOLL_CTL_ADD fail"} + std::string{strerror(errno)});
            }
            events.insert(std::make_pair(event->fd, event));
        }
        std::shared_ptr<Event>
        DelEvent(int eventfd){
            std::cout << " ========== Call DelEvent : " << eventfd << std::endl;
            auto ret = events.at(eventfd);
            events.erase(eventfd);
            if(epoll_ctl(epollfd, EPOLL_CTL_DEL, eventfd, nullptr) == -1){
                std::cout << "epoll_ctl EPOLL_CTL_ADD fail " << strerror(errno) << std::endl;
                // throw std::logic_error(std::string{"epoll_ctl EPOLL_CTL_ADD fail"} + std::string{strerror(errno)});
            }
            return ret;
        }
        std::shared_ptr<Event>
        ModEvent(std::shared_ptr<Event> event, struct epoll_event ev){
            std::cout << " ========== Call ModEvent : " << event->fd << std::endl;
            auto ret = events.at(event->fd);
            events.erase(event->fd);
            events[event->fd] = event;

            if(epoll_ctl(epollfd, EPOLL_CTL_MOD, event->fd, &ev) == -1){
                throw std::logic_error(std::string{"epoll_ctl EPOLL_CTL_ADD fail"} + std::string{strerror(errno)});
                std::cout << "epoll_ctl EPOLL_CTL_MOD fail " << strerror(errno) << std::endl;
            }
            return ret;
        }

        void
        Run(){
            struct epoll_event ev[1024];
            auto count = epoll_wait(epollfd, ev, 1024, 1000);
            for(int i = 0 ; i < count ; ++i){
                std::cout << "[" << i << "] Epoll event.. event: " << ev[i].events << std::endl;
                int key = ev[i].data.fd;
                auto value = events.find(key);
                if(value == events.end()){
                    std::cout << "events key: " << key << " : NOT FOUND\n";
                    continue;
                }
                if(value->second->pop != nullptr)
                    value->second->pop(ev[i]);
                
            }
        }


    };



}
