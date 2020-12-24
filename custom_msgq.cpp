#include <iostream>
#include <boost/asio.hpp>
#include "simple-message-queue/simplemsgq.hpp"

int main(int, char**) {

    std::cout << "Hello, world!\n";

    auto thread_pool = std::vector<std::thread>{};

    auto manager = simplemsgq::FileManagerBuilder::build("/home/ysh8361/msgq/custom");
    auto inserter = simplemsgq::FileInserter{manager};
    auto selecter = simplemsgq::FileSelecter{manager};


    uint16_t i_port = 4444;
    auto i_io = boost::asio::io_service{1};
    auto i_work = boost::asio::io_service::work{i_io};

    auto i_listener = simplemsgq::TcpAcceptor<simplemsgq::TcpSessionCustom>(i_io, i_port);
    i_listener.run(&inserter);

    thread_pool.emplace_back([&]{
        i_io.run();
    });



    uint16_t s_port = 5555;
    auto s_io = boost::asio::io_service{1};
    auto s_work = boost::asio::io_service::work{s_io};

    auto s_listener = simplemsgq::TcpAcceptor<simplemsgq::TcpSessionCustom>(s_io, s_port);
    s_listener.run(&selecter);

    thread_pool.emplace_back([&]{
        s_io.run();
    });

   while(1){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;

}
