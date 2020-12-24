#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "simplemsgq.hpp"

namespace simplemsgq
{

    class SimqClient{
    private:

        boost::asio::io_service & io_service;
        boost::asio::ip::tcp::socket socket;
        std::string host;
        std::string port;
        char buffer[8096];
    public:
        SimqClient() = default;
        SimqClient(boost::asio::io_service & io_service, const std::string & host, const std::string & port)
            : socket{io_service} 
            , io_service{io_service}
            , host{host}
            , port{port} { }
        
        void
        run(){
            // async_connect();
        }
        void
        handle_error(const boost::system::error_code & error){
            auto timer = std::make_shared<boost::asio::deadline_timer>(io_service, boost::posix_time::millisec(1000));
            (*timer).async_wait([=](const boost::system::error_code & error){
                timer;
                // async_connect();
            });
        }

        void
        do_read(){
            boost::asio::async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_at_least(1),
                [=](const boost::system::error_code & error, size_t len){
                if(error){
                    std::cout << "do_read error : " << error.message() << std::endl; 
                    handle_error(error);
                    return;
                }
                do_read();
            });
        }
        void
        async_connect(
            connect_handler handler){
            auto resolver = boost::asio::ip::tcp::resolver{io_service};
            auto query = boost::asio::ip::tcp::resolver::query{host, port};
            auto endpoint_iterator = resolver.resolve(query);

            boost::asio::async_connect(socket, endpoint_iterator, handler);
        }

        void 
        async_select(
            const unsigned int index,
            const unsigned int timeout,
            read_handler handler){


            auto timer = std::make_shared<boost::asio::deadline_timer>(io_service, boost::posix_time::millisec(timeout));
            (*timer).async_wait([](const boost::system::error_code & error){
                if(error){
                    return;
                }
                std::cout << "timeout occur!\n";
            });
            boost::asio::async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(sizeof(SIMPLEMSGQ_HEADER)), 
                [=](const boost::system::error_code & error, size_t len){
                    (*timer).cancel();
                    if(error){
                        std::cout << "async_read error " << error.message() << std::endl;
                        return;
                    }
                    std::cout << "async_read succ : " << len << std::endl;
            });

            // boost::asio::write(socket, boost::asio::buffer(&query, sizeof(query)));
            


        }
    };
}