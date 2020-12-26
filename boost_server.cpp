
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

class TestSession : public std::enable_shared_from_this<TestSession>{ 
public:
    boost::asio::ip::tcp::socket socket;
    char buffer[8096];
    TestSession(boost::asio::ip::tcp::socket socket) 
        : socket{std::move(socket)}{

    }

    void
    do_read(){
        auto self(this->shared_from_this());
        boost::asio::async_read( socket,  boost::asio::buffer(buffer), boost::asio::transfer_exactly(8),
            [self, this](const boost::system::error_code & error, const std::size_t & len){
                std::cout << "[" << now_str2() << "] read len: " << len << std::endl;
                for(int i = 0 ; i < len ; ++i){
                    std::cout << "[" << i << "] : " << (int)buffer[i] << std::endl;
                }
                if(error){ // TODO connection clear
                    throw std::logic_error("TODO Connection Error : " + error.message());
                }

                do_read();
            });
            
    }


};

class myClass{
public:
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;
    boost::asio::io_service::work work;

    myClass(boost::asio::io_service & io, uint16_t port)
        : acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
        , socket{io}
        , work{io} { }

    void
    do_accept(){
        acceptor.async_accept(socket, [this](const boost::system::error_code & error) {
            if(error){
                return;
            }
            auto session = std::make_shared<TestSession>(std::move(socket));
            session->do_read();
            do_accept();
        });
    }
    void
    do_write(){
        simplemsgq::SIMPLEMSGQ_HEADER header;
        header.init();
        // boost::asio::async_write(socket, )
    }
};

int main(int, char**) {

    auto io_service = boost::asio::io_service{1};

    auto test = myClass{io_service, 5555};
    test.do_accept();

    io_service.run();

    while(1){

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

