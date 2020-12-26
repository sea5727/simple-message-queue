
#include <iostream>
#include <thread>
#include <chrono>
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


class mytestclass{
public: 
    // boost::asio::io_service io;
    boost::asio::ip::tcp::socket socket;
    int index = 0;
    char buffer[8096];
    mytestclass(boost::asio::io_service & io)
        : socket{io}{ }
    
    void
    connect(boost::asio::io_service & io_service, std::string host, std::string port){
        auto resolver = boost::asio::ip::tcp::resolver{io_service};
        auto query = boost::asio::ip::tcp::resolver::query{host, port};
        auto endpoint_iterator = resolver.resolve(query);
        auto conn = boost::asio::connect(socket, endpoint_iterator);
    }

    void
    async_read_frame(){
        boost::asio::async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(8),
            [this](const boost::system::error_code & error, const size_t len){
                if(error){
                    throw std::logic_error("frame async_read fail " + error.message());
                }
                std::cout << "[" << now_str2() << "][CONSUMER2] read frame len: " << len << std::endl;
                auto frame = simplemsgq::SIMPLEMSGQ_FRAME{buffer};
                if(frame.check()){
                    frame.ntoh();
                    auto packet_len = frame.packet_len;

                    boost::asio::async_read(socket, boost::asio::buffer(buffer) + 8, boost::asio::transfer_exactly(packet_len - 8),
                        [this, packet_len](const boost::system::error_code & ec, size_t len){
                            std::cout << "[" << now_str2() << "][CONSUMER2] read body len: " << len << std::endl;
                            if(ec){ // TODO Error Control
                                throw std::logic_error("read fail TODO Error control : " + ec.message());
                            }

                            simplemsgq::SIMPLEMSGQ_HEADER * header = (simplemsgq::SIMPLEMSGQ_HEADER *)buffer;
                            header->ntoh();

                            auto body = header + sizeof(simplemsgq::SIMPLEMSGQ_HEADER);
                            auto bodylen = header->get_body_len();

                            if(header->count == 0){
                                std::cout << "[" << now_str2() << "][CONSUMER2] count : 0 " << std::endl;
                            }
                            else {
                                std::cout << "[" << now_str2() << "][CONSUMER2] offset:" << header->offset << ", Len:" << bodylen << ", body:" << body << std::endl;
                            }

                            // send_header(header->offset + 1, header->count);

                            async_read_frame();

                        });
                }
                else{
                    // do_read();
                }
            });
    }
    void
    read(){
        boost::asio::async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(8), 

            [this](const boost::system::error_code & error, const size_t len){
                if(error){
                    throw std::logic_error("frame async_read fail " + error.message());
                }
                std::cout << "[" << now_str2() << "][CONSUMER2] read frame len: " << len << std::endl;
                auto frame = simplemsgq::SIMPLEMSGQ_FRAME{buffer};
                if(frame.check()){
                    frame.ntoh();
                    auto packet_len = frame.packet_len;

                    boost::asio::async_read(socket, boost::asio::buffer(buffer) + 8, boost::asio::transfer_exactly(packet_len - 8),
                        [this, packet_len](const boost::system::error_code & ec, size_t len){
                            std::cout << "[" << now_str2() << "][CONSUMER2] read body len: " << len << std::endl;
                            if(ec){ // TODO Error Control
                                throw std::logic_error("read fail TODO Error control : " + ec.message());
                            }

                            simplemsgq::SIMPLEMSGQ_HEADER * header = (simplemsgq::SIMPLEMSGQ_HEADER *)buffer;
                            header->ntoh();

                            auto body = header + sizeof(simplemsgq::SIMPLEMSGQ_HEADER);
                            auto bodylen = header->get_body_len();

                            if(header->count == 0){
                                std::cout << "[" << now_str2() << "][CONSUMER2] count : 0 " << std::endl;
                            }
                            else {
                                std::cout << "[" << now_str2() << "][CONSUMER2] offset:" << header->offset << ", Len:" << bodylen << ", body:" << body << std::endl;
                            }

                            // send_header(header->offset + 1, header->count);

                            async_read_frame();

                        });
                }
                else{
                    // do_read();
                }
            });
    }
    void
    send_header(unsigned int offset, unsigned int count){
        std::cout << "[" << now_str2() << "][CONSUMER2] send_header " << index<< " start.. \n" ;

        simplemsgq::SIMPLEMSGQ_HEADER request;
        request.init();
        request.frame.packet_len = sizeof(simplemsgq::SIMPLEMSGQ_HEADER);
        request.sequence = 0;
        request.type = 0;
        request.name = 0;
        request.code = 0;
        request.offset = offset;
        request.count = count;
        request.hton();


        boost::asio::async_write(socket, boost::asio::buffer(&request, sizeof(request)),
            [this](const boost::system::error_code & error, const size_t len){
                std::cout << "[" << now_str2() << "][CONSUMER2] write " << index++ << " succ.. \n" ;
                if(error){
                    throw std::logic_error("async_write fail.. " + error.message());
                }
                std::cout << "[" << now_str2() << "][CONSUMER2] write " << index++ << " succ.. \n" ;
            });

    }
};
int main(int, char**) {
    auto offset = 0;
    auto count = 1;
    auto io_service = boost::asio::io_service{1};
    auto work = boost::asio::io_service::work{io_service};
    auto socket = boost::asio::ip::tcp::socket{io_service};
    // auto client = simplemsgq::ClientConsumer{io_service, "192.168.0.36", "5555"};

    std::string host = "192.168.0.36";
    std::string port = "5555";

    auto test = mytestclass{io_service};
    test.connect(io_service, host, port);
    std::cout << "[" << now_str2() << "][CONSUMER2] connect succ\n" ;
    test.async_read_frame();

    

    while(1){
        test.send_header(0, 1);
    }
    //        threadpool.emplace_back([&]{
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //     while(1){
    //         test.send_header(0, 1);
    //     }
        
    // });

    // std::cout << "[" << now_str2() << "][CONSUMER2] send header\n" ;
    // test.send_header(offset, count);
    
    
    return 0;
}

