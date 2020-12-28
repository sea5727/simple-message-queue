#pragma once

#include <iostream>
#include <boost/bind/bind.hpp>
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
        char send_buffer[8096];
        unsigned int offset = 0;
        unsigned int count = 1;

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

            boost::asio::async_connect(socket, endpoint_iterator, 
                [this](const boost::system::error_code & error, boost::asio::ip::tcp::endpoint endpoint){
                    async_read_frame();
                    send_header(offset++, count);
                });

        }


        void
        async_read_frame(){
            boost::asio::async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(FRAME_SIZE),
                [this](const boost::system::error_code & error, size_t len){
                    if(error){
                        throw std::logic_error("frame async_read fail " + error.message());
                    }
                    SIMPLEMSGQ_FRAME * frame = (SIMPLEMSGQ_FRAME *)buffer;
                    if(!frame->check()){
                        throw std::logic_error("frame async_read invalid frame ");
                    }
                    auto packet_len = ntohl(frame->packet_len);
                    async_read_body(packet_len);
                });
        }
        void
        async_read_body(size_t packet_len){
            boost::asio::async_read(socket, boost::asio::buffer(buffer) + FRAME_SIZE, boost::asio::transfer_exactly(packet_len - FRAME_SIZE), 
                [this](const boost::system::error_code & error, size_t len){
                    if(error){
                        throw std::logic_error("body read fail " + error.message());
                    }
                    std::cout << "[" << now_str() << "] read body len : " << len << std::endl;
                    send_header(offset++, count);
                    async_read_frame();
                });
        }


        unsigned int
        send_header(
            unsigned int offset,
            unsigned int count){

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
            
            memcpy(send_buffer, &request, sizeof(SIMPLEMSGQ_HEADER));

            boost::asio::async_write(socket, boost::asio::buffer(&request, sizeof(SIMPLEMSGQ_HEADER)), 
                [=](const boost::system::error_code & error, size_t len){
                    if(error){
                        throw std::logic_error("write fail " + error.message());
                    }
                    std::cout << "async_write success:" << len << ", offset : " << offset <<", count : " << count << std::endl;
                });
        }

        

    };
}