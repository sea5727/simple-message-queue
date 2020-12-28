#pragma once



#include "Error.hpp"
#include "Epoll.hpp"
#include "Event.hpp"

namespace EventCLoop
{
    class Timer{
        enum class TimerType{
            OneTimer = 0,
            IntervalTimer = 1,
        };
    public:
        Epoll & epoll;
        int timerfd;
        TimerType type;
        Timer(Epoll & epoll)
            : epoll{epoll} {
            timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
            if(timerfd == -1)
                throw std::runtime_error("timerfd_create fail " + std::string{strerror(errno)});
            std::cout << "[TIMER] fd : " << timerfd << std::endl;
        }
        ~Timer(){
            std::cout << "[TIMER] Delete Timer..." << std::endl;

        }

        void
        initOneTimer(unsigned int sec, unsigned int nsec){
            
            struct itimerspec ts;
            memset(&ts, 0, sizeof(struct itimerspec));
            ts.it_interval.tv_sec = 0;
            ts.it_interval.tv_nsec = 0;
            ts.it_value.tv_sec = sec;
            ts.it_value.tv_nsec = nsec;

            if(timerfd_settime(timerfd, 0, &ts, NULL) < 0){
                throw std::runtime_error("timerfd_settime fail " + std::string{strerror(errno)});
            }
            type = TimerType::OneTimer;
        }

        void
        initIntervalTimer(
            unsigned int interval_sec,
            unsigned int interval_nsec,
            unsigned int sec, 
            unsigned int nsec){

            struct itimerspec ts;
            memset(&ts, 0, sizeof(struct itimerspec));
            ts.it_interval.tv_sec = interval_sec;
            ts.it_interval.tv_nsec = interval_nsec;
            ts.it_value.tv_sec = sec;
            ts.it_value.tv_nsec = nsec;

            if(timerfd_settime(timerfd, 0, &ts, NULL) < 0){
                throw std::runtime_error("timerfd_settime fail " + std::string{strerror(errno)});
            }
            type = TimerType::IntervalTimer;
        }
        void
        async_wait(std::function<void()> callback){
            auto event = std::make_shared<Event>();
            event->fd = timerfd;
            std::cout << "async_wait start?" << std::endl;
            event->pop = [this, callback](struct epoll_event ev){
                
                std::cout << "[TIMER] poll pop!!  ev : " << ev.events << std::endl;
                uint64_t res;
                int ret = read(ev.data.fd, &res, sizeof(uint64_t));
                if(type == TimerType::OneTimer){
                    auto save = epoll.DelEvent(timerfd);
                    close(timerfd);
                }
                else if(type == TimerType::IntervalTimer){
                    
                }
                callback();
            };

            struct epoll_event ev;
            ev.data.fd = timerfd;
            ev.events = EPOLLIN;

            epoll.AddEvent(event, ev);
        }
        void
        clear(){
            auto save = epoll.DelEvent(timerfd);
            close(timerfd);
        }



    };
}