#pragma once

#include "simplemsgq_worker_interface.hpp"


namespace simplemsgq
{
    template<typename TyServer>
    class ServerAcceptor
    {
    private:
        boost::asio::ip::tcp::acceptor acceptor;
        boost::asio::ip::tcp::socket socket;
    public:

        ServerAcceptor(boost::asio::io_service & io_service, uint16_t port)
            : acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
            , socket(io_service) { }

        void
        run(IFWorker * worker){
            std::cout << "run start\n";
            do_accept(worker);
        }
    private:
        void
        do_accept(IFWorker * worker){
            acceptor.async_accept(socket, [this, worker](const boost::system::error_code & error) {
                if(error){
                    auto message = error.message();
                    return;
                }
                std::cout << "async_accept ! TyServer run\n";
                std::make_shared<TyServer>(std::move(socket))->run(worker);
                do_accept(worker);
            });
        }
    };
}