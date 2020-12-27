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
        char send_buffer[8096];

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
        }
        void
        init_async(){
            auto resolver = boost::asio::ip::tcp::resolver{io_service};
            auto query = boost::asio::ip::tcp::resolver::query{host, port};
            auto endpoint_iterator = resolver.resolve(query);
            auto conn = boost::asio::connect(socket, endpoint_iterator);
            
            do_read_async();
        }
        // void
        // do_read(std::function<void(boost::system::error_code, size_t)> read_body_handler){
        //     boost::asio::async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(FRAME_SIZE), 
        //         [=](const boost::system::error_code & error, const size_t len){
        //         if(error){ // TODO Error Control
        //             throw std::logic_error("async_read fail TODO Error control : " + error.message());
        //         }

        //         auto frame = SIMPLEMSGQ_FRAME{}; // copy

        //         if(frame.check()){
        //             frame.ntoh();
        //             auto packet_len = frame.packet_len;
        //             boost::system::error_code ec;
        //             boost::asio::async_read( 
        //                 socket,
        //                 boost::asio::buffer(boost::asio::buffer(buffer) + FRAME_SIZE),
        //                 boost::asio::transfer_exactly(packet_len - FRAME_SIZE), 
        //                 read_body_handler);
        //         }
        //         else{
        //             do_read();
        //         }
        //     });
        // }

        void
        do_select_block(
            unsigned int offset,
            unsigned int count){     
            auto result = send_header(offset, count);
            std::cout << "[" << now_str() << "]send_header result : " << result << std::endl;
            if(result == 0) return;

            unsigned int packet_len = 0;
            {
                boost::system::error_code error;
                auto len = boost::asio::read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(FRAME_SIZE), error);
                if(error){
                    throw std::logic_error("frame read fail " + error.message());
                }
                SIMPLEMSGQ_FRAME * frame = (SIMPLEMSGQ_FRAME *) buffer;
                if(frame->check()){
                    packet_len = ntohl(frame->packet_len);
                }
                std::cout << "[" << now_str() << "]frame read packet_len :" << packet_len << std::endl;
            }

            {
                boost::system::error_code error;
                auto len = boost::asio::read(socket, boost::asio::buffer(buffer) + FRAME_SIZE, boost::asio::transfer_exactly(packet_len - FRAME_SIZE), error);
                if(error){
                    throw std::logic_error("body read fail " + error.message());
                }
                SIMPLEMSGQ_HEADER * header = (SIMPLEMSGQ_HEADER *) buffer;
                header->ntoh();
                header->offset;
                header->count;
                std::cout << "[" << now_str() << "]body read len :" << len << std::endl;
            }

        }

        void
        do_select_async(
            unsigned int offset,
            unsigned int count){    

            auto result = send_header(offset, count);
            std::cout << "[" << now_str() << "]send_header result : " << result << std::endl;
            
            if(result == 0) return;
            
        }
        void
        do_read_async(){
            boost::asio::async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(FRAME_SIZE),
                [this](boost::system::error_code error, size_t len){
                    if(error){
                        throw std::logic_error("frame async_read fail " + error.message());
                    }
                    SIMPLEMSGQ_FRAME * frame = (SIMPLEMSGQ_FRAME *)buffer;
                    if(!frame->check()){
                        throw std::logic_error("frame async_read invalid frame ");
                    }
                    auto packet_len = ntohl(frame->packet_len);
                    auto header_len = boost::asio::read(socket, boost::asio::buffer(buffer) + FRAME_SIZE, boost::asio::transfer_exactly(packet_len - FRAME_SIZE), error);
                    if(error){
                        throw std::logic_error("body read fail " + error.message());
                    }
                    std::cout << "[" << now_str() << "] read body len : " << header_len << std::endl;

                    do_read_async();

                });
        }

        unsigned int 
        do_poll(unsigned int timeout_milli){
            auto count = io_service.run_one_for( std::chrono::milliseconds(timeout_milli));
            return count;
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

            boost::system::error_code error;
            auto result = boost::asio::write(socket, boost::asio::buffer(&request, sizeof(SIMPLEMSGQ_HEADER)), error);
            if(error){
                throw std::logic_error("write fail " + error.message());
            }
            return result;
            
        }

        

    };
}