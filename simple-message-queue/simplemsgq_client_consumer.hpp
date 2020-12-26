#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "simplemsgq_define.hpp"
#include "simplemsgq_client_consumer.hpp"

namespace simplemsgq
{
    class ClientConsumer{
    private:
        boost::asio::io_service & io_service;
        boost::asio::ip::tcp::socket socket;
        std::string host;
        std::string port;
        char buffer[8096];

    public:
        ClientConsumer() = default;
        ClientConsumer(boost::asio::io_service & io_service, const std::string & host, const std::string & port)
            : socket{io_service} 
            , io_service{io_service}
            , host{host}
            , port{port} { }
        
        SIMPLEMSGQ_HEADER *
        get_header(){
            SIMPLEMSGQ_HEADER * header = (SIMPLEMSGQ_HEADER *)buffer;
            return header;
        }
        char *
        get_body(){
            return buffer + sizeof(SIMPLEMSGQ_HEADER);
        }

        void
        init(){
            auto resolver = boost::asio::ip::tcp::resolver{io_service};
            auto query = boost::asio::ip::tcp::resolver::query{host, port};
            auto endpoint_iterator = resolver.resolve(query);
            auto conn = boost::asio::connect(socket, endpoint_iterator);
            // socket.non_blocking(true);
            do_read();
        }
        void
        do_read(){
            boost::asio::async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(FRAME_SIZE), 
                [=](const boost::system::error_code & error, const size_t len){
                if(error){ // TODO Error Control
                    throw std::logic_error("async_read fail TODO Error control : " + error.message());
                }

                auto frame = SIMPLEMSGQ_FRAME{buffer}; // copy

                if(frame.check()){
                    frame.ntoh();
                    auto packet_len = frame.packet_len;
                    boost::system::error_code ec;
                    auto len = boost::asio::read(
                                            socket,  
                                            boost::asio::buffer(boost::asio::buffer(buffer) + FRAME_SIZE), 
                                            boost::asio::transfer_exactly(packet_len - FRAME_SIZE), 
                                            ec);

                    if(ec){ // TODO Error Control
                        throw std::logic_error("read fail TODO Error control : " + ec.message());
                    }

                    SIMPLEMSGQ_HEADER * header = (SIMPLEMSGQ_HEADER *)buffer;
                    header->ntoh();

                    do_read();
                }
                else{
                    do_read();
                }
            });
        }

        void
        do_select(
            unsigned int offset,
            unsigned int count){            
            send_header(offset, count);
        }
        unsigned int 
        do_poll(unsigned int timeout_milli){
            auto started = std::chrono::high_resolution_clock::now();
            auto count = io_service.run_one();
            auto done = std::chrono::high_resolution_clock::now();
            std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(done-started).count() << "ms\n";
            // auto count = io_service.run_one_for(
            //         std::chrono::milliseconds(timeout_milli));
            return count;
        }
        void
        send_header(
            unsigned int offset,
            unsigned int count){

            char testbuffer[sizeof(SIMPLEMSGQ_HEADER)];
            SIMPLEMSGQ_HEADER request;
            request.init();
            request.frame.packet_len = sizeof(SIMPLEMSGQ_HEADER);
            request.sequence = 0;
            request.type = 0;
            request.name = 0;
            request.code = 0;
            request.offset = offset;
            request.count = count;
            request.hton();

            memcpy(testbuffer, &request, sizeof(SIMPLEMSGQ_HEADER));

            // for(int i = 0 ; i < sizeof(SIMPLEMSGQ_HEADER) ; ++i){
            //     std::cout << "[" << i << "] : " << (int)testbuffer[i] << std::endl;
            // }
            auto write_len = boost::asio::write(socket, 
                                                boost::asio::buffer(testbuffer));
            std::cout << "[" << now_str() << "]send packet_len : " << write_len << std::endl;
        }

    };
}