
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


    auto io_service = boost::asio::io_service{1};

    auto test = test::TestConsumer{};
    auto client = simplemsgq::Client{io_service, "192.168.0.35", "5555", 0, 1};
    client.init();
    client.do_poll();
    std::cout << "do_poll end\n";

    std::this_thread::sleep_for(std::chrono::seconds(3));
    io_service.run_for(std::chrono::milliseconds(1000));
    // client.run(&test);

    std::cout << "test io.run\n";

    // io.run();

    
    std::cout << "test io.run end\n";


   while(1){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;

}
