
#include <iostream>
#include <boost/asio.hpp>

#include "simple-message-queue/simplemsgq.hpp"
#include "client_consumer_test.hpp"
class myclass {
public:
    myclass(){
        std::cout << "default constructor\n";
    }
    myclass(const myclass & copy){
        std::cout << "copy constructor\n";
    }
    myclass(myclass && move){
        std::cout << "move constructor\n";
    }
    ~myclass(){
        std::cout << "myclass delete\n";
    }
    void print(){
        std::cout << "Hello world\n";
    }
};

int main(int, char**) {


    auto io = boost::asio::io_service{1};

    auto test = test::TestConsumer{};
    auto client = simplemsgq::Client{io, "192.168.0.35", "5555", 0, 1};
    client.run(&test);

    io.run();

   while(1){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;

}