#pragma once

#include <functional>
#include <sys/epoll.h>

namespace EventCLoop
{
    class Event{
    public:
        int fd;
        std::function<void(struct epoll_event ev)> pop;
        Event() = default;
        ~Event(){
            // std::cout << "~Event Delete\n";
        }

    };
}