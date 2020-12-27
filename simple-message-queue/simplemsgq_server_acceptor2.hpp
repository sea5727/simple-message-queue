#pragma once

#include "simplemsgq_worker_interface.hpp"

#include "simplemsgq_server_session2.hpp"

namespace simplemsgq
{
    class ServerAcceptor2
    {
    private:
        boost::asio::ip::tcp::acceptor acceptor;
        boost::asio::ip::tcp::socket socket;
    public:

        ServerAcceptor2(boost::asio::io_service & io_service, uint16_t port)
            : acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
            , socket(io_service) { }

        void
        run(){
            do_accept();
        }
    private:
        void
        do_accept(){
            
            acceptor.async_accept(socket, [this](const boost::system::error_code & error) {
                if(error){
                    auto message = error.message();
                    return;
                }

                std::make_shared<ServerSession2>(std::move(socket))->run();
                do_accept();
            });
        }


    };
}