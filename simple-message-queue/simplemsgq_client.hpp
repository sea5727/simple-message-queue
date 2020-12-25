#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "simplemsgq_define.hpp"
#include "simplemsgq_client_consumer.hpp"

namespace simplemsgq
{
    class Client{
    private:
        boost::asio::io_service & io_service;
        boost::asio::ip::tcp::socket socket;
        std::string host;
        std::string port;
        unsigned int offset;
        unsigned int count;
        char buffer[8096];
        ClientConsumer * worker;

    public:
        Client() = default;
        Client(boost::asio::io_service & io_service, const std::string & host, const std::string & port, unsigned int offset, unsigned int count)
            : socket{io_service} 
            , io_service{io_service}
            , host{host}
            , port{port}
            , offset{offset}
            , count{count}{ }
        


        void
        init(){
            auto resolver = boost::asio::ip::tcp::resolver{io_service};
            auto query = boost::asio::ip::tcp::resolver::query{host, port};
            auto endpoint_iterator = resolver.resolve(query);
            auto conn = boost::asio::connect(socket, endpoint_iterator);
            // socket.non_blocking(true);
        }
        void
        do_poll(unsigned int timeout_milli){
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


            auto write_len = boost::asio::write(socket, boost::asio::buffer(&request, sizeof(request)));
            std::cout << "write len : " << write_len << std::endl;
            boost::asio::async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(FRAME_SIZE), 
                [](const boost::system::error_code & error, const size_t len){
                    std::cout << "async_read!!\n";
                });
            std::cout << "run_for start\n";
            io_service.run_for(std::chrono::milliseconds(timeout_milli));
            std::cout << "run_for end\n";

  

            

            // std::cout << "frame_len : " << frame_len << std::endl;

            // auto frame = SIMPLEMSGQ_FRAME{buffer}; // copy

            // if(!frame.check()){
            //     std::cout << "frame check fail\n";
            //     return;
            // }
            // frame.ntoh();
            // auto packet_len = frame.packet_len - FRAME_SIZE;
            // auto read_len = boost::asio::read(socket, (boost::asio::buffer(buffer) + FRAME_SIZE), boost::asio::transfer_exactly(packet_len));
            // std::cout << "read len : " <<  read_len << std::endl;

            // SIMPLEMSGQ_HEADER * header = (SIMPLEMSGQ_HEADER *)buffer;
            // SIMPLEMSGQ_PACKET * packet = (SIMPLEMSGQ_PACKET *)buffer;
            // header->ntoh();
            // std::cout << "sequence : " << header->sequence << std::endl;
            // std::cout << "type : " << header->type << std::endl;
            // std::cout << "name : " << header->name << std::endl;
            // std::cout << "code : " << header->code << std::endl;
            // std::cout << "offset : " << header->offset << std::endl;
            // std::cout << "count : " << header->count << std::endl;


                        
        }
        void
        run(ClientConsumer * worker){
            this->worker = worker;
            do_connect();
        }
        void
        handle_error(const boost::system::error_code & error){
            auto timer = std::make_shared<boost::asio::deadline_timer>(io_service, boost::posix_time::millisec(1000));
            (*timer).async_wait([=](const boost::system::error_code & error){
                timer;
                do_connect();
            });
        }

        void
        do_read_frame(){
            boost::asio::async_read(socket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(FRAME_SIZE), 
                [=](const boost::system::error_code & error, const size_t len){
                if(error){ handle_error(error); return; }

                auto frame = SIMPLEMSGQ_FRAME{buffer}; // copy

                if(frame.check()){
                    frame.ntoh();
                    auto packet_len = frame.packet_len;
                    std::cout << "check succ : " << std::dec << packet_len << std::endl;
                    do_read_body(packet_len);
                    // do_read_body(worker, packet_len);
                }
                else{
                    std::cout << "check fail\n";
                    do_read_frame();
                    // do_read_frame(worker);
                }
                        
                
            });
        }
        void
        do_read_body(size_t bodylen){
            boost::asio::async_read(socket, boost::asio::buffer(boost::asio::buffer(buffer) + FRAME_SIZE), boost::asio::transfer_exactly(bodylen - FRAME_SIZE), 
                [=](const boost::system::error_code & error, size_t len){
                std::cout << "do_read : " << error.message() << ", len : " << len << std::endl; 
                if(error){
                    handle_error(error);
                    return;
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

                worker->do_select(*packet);
                do_read_frame();
            });
        }
        void
        do_write(){
            SIMPLEMSGQ_HEADER header;
            header.init();
            header.frame.packet_len = sizeof(SIMPLEMSGQ_HEADER);
            header.sequence = 0;
            header.type = 0;
            header.name = 0;
            header.code = 0;
            header.offset = offset;
            header.count = count;
            std::cout << "do_write: packet_len:" <<  header.frame.packet_len << ", offset:" << header.offset << ", count : " << header.count << std::endl;
            header.hton();
            boost::asio::async_write(socket, boost::asio::buffer(&header, sizeof(header)) ,
                [this](const boost::system::error_code & error, const size_t len){
                if(error){
                    throw std::logic_error("boost async_write error:" + error.message());
                    // handle_error(error); // write error?
                    return;
                }
                std::cout << "async_write success.. len: " << len << std::endl;
            });

        }
        void
        do_connect(){
            auto resolver = boost::asio::ip::tcp::resolver{io_service};
            auto query = boost::asio::ip::tcp::resolver::query{host, port};
            auto endpoint_iterator = resolver.resolve(query);

            boost::asio::async_connect(socket, endpoint_iterator, 
                [=](const boost::system::error_code & error, const boost::asio::ip::tcp::endpoint & end){
                    std::cout << "connect error : " << error.message() << std::endl;
                    if(error){
                        handle_error(error);
                        return;
                    }

                    do_read_frame();
                    do_write();

            });
        }
    };
}