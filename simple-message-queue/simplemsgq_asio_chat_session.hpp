#pragma once

#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

#include "simplemsgq_worker_interface.hpp"

namespace simplemsgq
{
    class TcpSessionChat : public std::enable_shared_from_this<TcpSessionChat>{
    private:
        boost::asio::ip::tcp::socket socket;
        std::string buffer;
    public:
        TcpSessionChat(boost::asio::ip::tcp::socket socket)
            : socket{std::move(socket)}
            , buffer{}{

        }

        void 
        run(IFWorker * worker) {
            do_read(worker);
        }
    private:
        void 
        do_read(IFWorker * worker) {
            auto self(this->shared_from_this());
            boost::asio::async_read_until( socket, boost::asio::dynamic_buffer(buffer), "\n", 
                [this, self, worker](const boost::system::error_code & error_code , size_t len){
                    if(error_code){
                        return ;
                    }
                    auto data = buffer.c_str();
                    auto size = buffer.length();
                    
                    // worker->do_read(socket, data, size); // TODO do_read를 다른 interface로 교체 필요

                    buffer.clear();
                    do_read(worker);
                });
        }
    };
} // namespace simplemsgq
