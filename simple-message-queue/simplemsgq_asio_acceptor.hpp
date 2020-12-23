#pragma once

#include "simplemsgq_asio_interface.hpp"


namespace simplemsgq
{
    template<typename TyServer>
    class TcpAcceptor
    {
    private:
        boost::asio::ip::tcp::acceptor acceptor;
        boost::asio::ip::tcp::socket socket;
    public:

        TcpAcceptor(boost::asio::io_service & io_service, uint16_t port)
            : acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
            , socket(io_service) { }

        void
        run(SimplemsgqWorker * worker){
            do_accept(worker);
        }
    private:
        void
        do_accept(SimplemsgqWorker * worker){
            
            acceptor.async_accept(socket, [worker, this](const boost::system::error_code & error) {
                if(error){
                    auto message = error.message();
                    return;
                }
                std::cout << "accept success\n";
                worker->do_accept(socket);
                std::make_shared<TyServer>(std::move(socket))->run(worker);
                do_accept(worker);
            });
        }


    };
}