#pragma once

#include <boost/asio/ip/tcp.hpp>

namespace simplemsgq{
    class SimplemsgqWorker{
    public:
        virtual void do_accept(boost::asio::ip::tcp::socket & socket) = 0;
        virtual void do_read(boost::asio::ip::tcp::socket & socket, const char * buffer, const unsigned int len) = 0;
    };
}