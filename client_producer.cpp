#include <iostream>

#include "simplemsgq.hpp"

int main(int argc, char * argv[]){

    auto epoll = EventCLoop::Epoll{};

    auto server_consumer = simplemsgq::
    // auto server_producer = simplemsgq::ServerProducer{epoll, 4444, "192.168.0.36"};
    // server_consumer.run();
    // server_producer.run();

    while(1){
        epoll.Run();
    }

    return 0;
}