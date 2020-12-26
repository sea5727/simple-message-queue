
#include <iostream>
#include <boost/asio.hpp>

#include "simple-message-queue/simplemsgq.hpp"

std::string now_str2()
{
    // Get current time from the clock, using microseconds resolution
    const boost::posix_time::ptime now = 
        boost::posix_time::microsec_clock::local_time();

    // Get the time offset in current day
    const boost::posix_time::time_duration td = now.time_of_day();

    const long hours        = td.hours();
    const long minutes      = td.minutes();
    const long seconds      = td.seconds();
    const long milliseconds = td.total_milliseconds() -
                              ((hours * 3600 + minutes * 60 + seconds) * 1000);

    char buf[40];
    sprintf(buf, "%02ld:%02ld:%02ld.%03ld", 
        hours, minutes, seconds, milliseconds);

    return buf;
}

class TestClient{
private:
    boost::asio::io_service & io_service;
    boost::asio::ip::tcp::socket socket;
    std::string host;
    std::string port;
    char buffer[8096];

public:
    TestClient() = default;
    TestClient(boost::asio::io_service & io_service, const std::string & host, const std::string & port)
        : socket{io_service} 
        , io_service{io_service}
        , host{host}
        , port{port} { }
    


    void
    init(){
        auto resolver = boost::asio::ip::tcp::resolver{io_service};
        auto query = boost::asio::ip::tcp::resolver::query{host, port};
        auto endpoint_iterator = resolver.resolve(query);
        auto conn = boost::asio::connect(socket, endpoint_iterator);
        do_read();
    }
    void
    do_read(){
        boost::asio::async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(8), 
            [=](const boost::system::error_code & error, const size_t len){
            if(error){ // TODO Error Control
                throw std::logic_error("async_read fail TODO Error control : " + error.message());
            }
            do_read();
        });
    }
    void
    do_write(char * data, size_t len){
        boost::asio::async_write(socket, boost::asio::buffer(data, len), 
            [=](const boost::system::error_code & error, const size_t len){
            if(error){ // TODO Error Control
                throw std::logic_error("async_read fail TODO Error control : " + error.message());
            }
            do_read();
        });
    }
};



int main(int, char**) {

    auto io_service = boost::asio::io_service{1};

    auto test = TestClient{io_service, "192.168.0.36", "5555"};
    test.init();
    // char buffer[1024] = "12345678";
    char buffer[1024] = "12345678";

    char data[sizeof(simplemsgq::SIMPLEMSGQ_HEADER)] ;
    simplemsgq::SIMPLEMSGQ_HEADER header;
    header.init();
    header.frame.packet_len = 40;

    header.hton();

    memcpy(data, &header, sizeof(simplemsgq::SIMPLEMSGQ_HEADER));

    for(int i = 0 ; i < sizeof(simplemsgq::SIMPLEMSGQ_HEADER) ; ++i){
        std::cout << "[" << i << "] : " << (int)data[i] << std::endl;
    }


    test.do_write((char *)&header, sizeof(header));

    io_service.run();

    while(1){

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

