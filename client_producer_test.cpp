
#include <iostream>
#include <boost/asio.hpp>

#include "simple-message-queue/simplemsgq.hpp"

std::string now_str()
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

    auto io_service = boost::asio::io_service{1};
    auto producer = simplemsgq::ClientProducer{io_service, "192.168.0.36", "4444"};

    producer.init();

    std::string ss;
    char buffer[1024] = "";
    int i = 1;
    
    std::stringstream index;
    while(1){
        // std::cin >> ss;
        sprintf(buffer, "%016d\n", i);
        producer.send(buffer, 16 + 1);
        producer.do_poll(100);
        // memset(buffer, 0x00, sizeof(buffer));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

