#include <iostream>


#include "simple-message-queue/simplemsgq.hpp"

int main(int argc, char * argv[]){
    std::cout << "Hello world" << std::endl;
    auto epoll = EventCLoop::Epoll{};


    auto fm = simplemsgq::FileManagerBuilder::build("/home/ysh8361/msgq/custom");
    auto server_consumer = simplemsgq::ServerConsumer{epoll, 5555, "192.168.0.36", fm};
    auto server_producer = simplemsgq::ServerProducer{epoll, 4444, "192.168.0.36", fm};
    server_consumer.run();
    server_producer.run();

    while(1){
        epoll.Run();
    }

    return 0;
}