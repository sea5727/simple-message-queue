#include <iostream>
#include <thread>
#include <chrono>

#include "EventCLoop/EventCLoop.hpp"
#include "simple-message-queue/simplemsgq.hpp"

int main(int argc, char * argv[]){

    auto epoll = EventCLoop::Epoll{};

    auto consumer = simplemsgq::ClientConsumer{epoll, 5555, "192.168.0.35"};




    consumer.set_connect_callback(
        [&consumer, &epoll](EventCLoop::Error & error){
            if(error){
                std::cout << "[CONNECT] error..." << error.what() << std::endl;
                auto timer = std::make_shared<EventCLoop::Timer>(epoll);
                timer->initOneTimer(1, 0);
                timer->async_wait(
                    [timer, &consumer](EventCLoop::Error & error) { 
                        consumer.run();
                    });
                return;
            }
            consumer.async_read();
            auto sendtimer = std::make_shared<EventCLoop::Timer>(epoll);
            sendtimer->initOneTimer(1, 0);
            sendtimer->async_wait(
                [sendtimer, &consumer, offset = 0](EventCLoop::Error & error) mutable { 
                    std::cout << "timer call!! offset : " << offset << std::endl;
                    consumer.send_consume(offset ++ , 1);
            });

        });
    
    consumer.set_read_callback(
        [&epoll, &consumer](int fd, char * buffer, size_t len){
            simplemsgq::SIMPLEMSGQ_HEADER * header = (simplemsgq::SIMPLEMSGQ_HEADER *) buffer;
            std::cout << "read ! offset : " << header->offset << ", " << header->count << std::endl;
            int offset = header->offset;

            auto sendtimer = std::make_shared<EventCLoop::Timer>(epoll);
            sendtimer->initOneTimer(1, 0);
            sendtimer->async_wait(
                [sendtimer, &consumer, offset](EventCLoop::Error & error) mutable { 
                    consumer.send_consume(offset + 1 , 1);
            });
        });
    consumer.run();

    
    while(1){
        epoll.Run();
    }

    return 0;
}