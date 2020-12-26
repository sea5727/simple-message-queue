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

    auto thread_pool = std::vector<std::thread>{};

    std::cout << "Start\n";
    boost::asio::io_service io{1};
    boost::asio::io_service::work work{io};

    thread_pool.emplace_back([&]{
        // while(1){
        //     io.post([]{
        //         std::cout << "io.post\n";
        //     });
        //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
        // }

    });



   while(1){
        auto started = std::chrono::high_resolution_clock::now();
        io.#include <iostream>
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
