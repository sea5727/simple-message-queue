#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "simplemsgq_define.hpp"
#include "simplemsgq_client_consumer.hpp"

namespace simplemsgq
{
    class ClientProducer{
    private:
        boost::asio::io_service & io_service;
        boost::asio::ip::tcp::socket socket;
        std::string host;
        std::string port;
        char buffer[8096];

    public:
        ClientProducer() = default;
        ClientProducer(boost::asio::io_service & io_service, const std::string & host, const std::string & port)
            : socket{io_service} 
            , io_service{io_service}
            , host{host}
            , port{port} { }
        
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
        send(const char * data, 
            unsigned int sendlen){
            std::cout << "send body : " << data << ", len: " << sendlen << std::endl;
            send_header(sendlen);
            
            boost::system::error_code ec;

            auto len = boost::asio::write(
                                    socket, 
                                    boost::asio::buffer(data, sendlen), 
                                    boost::asio::transfer_exactly(sendlen), 
                                    ec);
            
            if(ec){
                return; // TODO Error control
            }
            std::cout << "send  len : " << len << std::endl;
        }

        void
        send_header(unsigned int valuelen){
            SIMPLEMSGQ_HEADER header;
            header.init();
            header.frame.packet_len = valuelen + sizeof(SIMPLEMSGQ_HEADER);
            header.code = 0;
            header.count = 0;
            header.name = 0 ;
            header.offset = 0;
            header.sequence = 123;
            header.type = 0;

            header.hton();

            boost::system::error_code ec;
            std::cout << "send_header write..\n";
            auto len = boost::asio::write(
                    socket, 
                    boost::asio::buffer(&header, sizeof(SIMPLEMSGQ_HEADER)), 
                    boost::asio::transfer_exactly(sizeof(SIMPLEMSGQ_HEADER)), 
                    ec);
            std::cout << "send_header write.. end.. len: " << len << std::endl;
            if(ec){
                std::cout << "write error : " << ec.message() << std::endl;
                return; // TODO Error control
            }
            std::cout << "send_header len : " << len << std::endl;
        }
        void
        do_read(){
            boost::asio::async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(FRAME_SIZE), 
                [=](const boost::system::error_code & error, const size_t len){
                if(error){ // TODO Error Control
                    return; 
                }

                auto frame = SIMPLEMSGQ_FRAME{}; // copy

                if(frame.check()){
                    frame.ntoh();
                    auto packet_len = frame.packet_len;
                    std::cout << "check succ : " << std::dec << packet_len << std::endl;
                    boost::system::error_code ec;
                    auto len = boost::asio::read(
                                            socket,  
                                            boost::asio::buffer(boost::asio::buffer(buffer) + FRAME_SIZE), 
                                            boost::asio::transfer_exactly(packet_len - FRAME_SIZE), 
                                            ec);

                    if(ec){
                        return; // TODO Error Control
                    }

                    SIMPLEMSGQ_HEADER * header = (SIMPLEMSGQ_HEADER *)buffer;
                    SIMPLEMSGQ_PACKET * packet = (SIMPLEMSGQ_PACKET *)buffer;
                    header->ntoh();
                    std::cout << "sequence : " << header->sequence << std::endl;
                    std::cout << "type : " << header->type << std::endl;
                    std::cout << "name : " << header->name << std::endl;
                    std::cout << "code : " << header->code << std::endl;
                    std::cout << "offset : " << header->offset << std::endl;
                    std::cout << "count : " << header->count << std::endl;
                    do_read();
                }
                else{
                    std::cout << "check fail\n";
                    do_read();
                    // do_read_frame(worker);
                }
            });
        }

        unsigned int 
        do_poll(unsigned int timeout_milli){
            auto count = io_service.run_one_for(std::chrono::milliseconds(timeout_milli));
            return count;
        }
    };
}
