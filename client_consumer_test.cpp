
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
    return 0;
}

