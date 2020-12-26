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

int main(int, char**) {

    std::cout << "Hello, world!\n";

    auto thread_pool = std::vector<std::thread>{};

    auto manager = simplemsgq::FileManagerBuilder::build("/home/ysh8361/msgq/custom");
    auto producer = simplemsgq::ProducerWorker{manager};
    auto consumer = simplemsgq::ConsumerWorker{manager};


    uint16_t i_port = 4444;
    auto i_io = boost::asio::io_service{1};
    auto i_work = boost::asio::io_service::work{i_io};

    auto i_listener = simplemsgq::ServerAcceptor<simplemsgq::TcpSessionCustom>(i_io, i_port);
    i_listener.run(&producer);

    thread_pool.emplace_back([&]{
        i_io.run();
    });



    uint16_t s_port = 5555;
    auto s_io = boost::asio::io_service{1};
    auto s_work = boost::asio::io_service::work{s_io};

    auto s_listener = simplemsgq::ServerAcceptor<simplemsgq::TcpSessionCustom>(s_io, s_port);
    s_listener.run(&consumer);

    thread_pool.emplace_back([&]{
        s_io.run();
    });


    thread_pool.emplace_back([]{
        auto offset = 0;
        auto count = 1;
        auto io_service = boost::asio::io_service{1};
        auto client = simplemsgq::ClientConsumer{io_service, "192.168.0.36", "5555"};
        client.init();
        client.do_select(offset++, count);
        while(1){ 
            std::cout << "[" << now_str2() << "][CONSUMER] While..\n";
            auto count = client.do_poll(1000);
            std::cout << "[" << now_str2() << "][CONSUMER] do poll\n";
            if(count > 0){
                auto header = client.get_header();
                auto body = client.get_body();
                auto bodylen = header->get_body_len();

                if(header->count == 0){
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                else {
                    std::cout << "[" << now_str2() << "][CONSUMER] offset:" << header->offset << ", Len:" << bodylen << ", body:" << body << std::endl;
                }
                client.do_select(offset++, count);
            }
            
        }
    });


   while(1){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;

}
