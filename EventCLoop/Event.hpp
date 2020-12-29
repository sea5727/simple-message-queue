#pragma once

#include <functional>
#include <sys/epoll.h>

namespace EventCLoop
{
    class Event{
    public:
        int fd;
        std::function<void(const struct epoll_event & ev)> pop;
        Event()
            : fd{-1}
            , pop{nullptr} {}
        ~Event(){
            std::cout << "~Event Delete\n";
        }

        bool
        isCleared(){
            if(fd == -1) 
                return true;
            return false;
        }
        void
        clear(){
            fd = -1;
            pop = nullptr;
        }

    };
}