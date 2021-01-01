#include <iostream>
#include <thread>
#include <chrono>

#include "EventCLoop/EventCLoop.hpp"
#include "simple-message-queue/simplemsgq.hpp"

int main(int argc, char * argv[]){

    std::vector<std::thread> pool;

    auto epoll = EventCLoop::Epoll{};

    auto producer = simplemsgq::ClientProducer{epoll, 4444, "192.168.0.35"};

    producer.set_connect_callback(
        [&producer, &epoll](EventCLoop::Error & error, int fd){
            std::cout << "set_connect_callback!!" << std::endl;
        });
    
    producer.set_read_callback(
        [&epoll, &producer](int fd, char * buffer, size_t len){
            simplemsgq::SIMPLEMSGQ_HEADER * header = (simplemsgq::SIMPLEMSGQ_HEADER *) buffer;
            std::cout << "read ! offset : " << header->offset << ", " << header->count << std::endl;
            int offset = header->offset;
        });

    producer.run();

    pool.emplace_back([&producer]{
        std::string buffer;
        while(1){
            std::cin >> buffer;
            std::cout << "[INPUT]" << buffer << std::endl;
            
            producer.send_produce((void *)buffer.c_str(), buffer.length());
        }

    });
    
    while(1){
        epoll.Run();
    }

    return 0;
}