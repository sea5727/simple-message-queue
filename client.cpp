#include <iostream>

#include "simplemsgq.hpp"

int main(int argc, char * argv[]){

    auto epoll = EventCLoop::Epoll{};

    auto consumer = simplemsgq::ClientConsumer{epoll, 5555, "192.168.0.36"};


    int offset = 0;
    int count = 1;
    
    consumer.connect();
    consumer.async_read();
    consumer.send_consume(offset, count);
    while(1){
        epoll.Run();

    }

    return 0;
}