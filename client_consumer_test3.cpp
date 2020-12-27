
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
    client.init_async();

    
    auto started = std::chrono::high_resolution_clock::now();

    while(1){
        client.do_select_async(offset++, 1);
        auto result = client.do_poll(1000);
        if(result > 0){
            simplemsgq::SIMPLEMSGQ_HEADER * header = client.get_header();
            header->ntoh();
            std::cout << "[" << now_str2() << "] recv offset:" << header->offset << ", count:" << header->count << std::endl;
            // auto body = client.get_body();
            // auto bodylen = header->get_body_len();

        }
        // std::this_thread::sleep_for(std::chrono::seconds(1));

        if(offset == 100000) break;
    }
    
    auto done = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(done-started).count() << "ms\n";
    return 0;
}

