#pragma once

#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

#include "simplemsgq_define.hpp"
#include "simplemsgq_worker_interface.hpp"

namespace simplemsgq
{
    class TcpSessionCustom : public std::enable_shared_from_this<TcpSessionCustom>{
    private:
        boost::asio::ip::tcp::socket socket;
        char buffer[8096];
    public:
        TcpSessionCustom(
            boost::asio::ip::tcp::socket socket)
            : socket{std::move(socket)}
            , buffer{} { }

        void 
        run(IFWorker * worker) {
            do_read_frame(worker);
        }
    private:
        void 
        do_read_frame(IFWorker * worker) {
            auto self(this->shared_from_this());
            boost::asio::async_read( socket,  boost::asio::buffer(buffer), boost::asio::transfer_exactly(FRAME_SIZE),
                [self, this, worker](const boost::system::error_code & error, const std::size_t & len){
                    if(error){ // TODO connection clear
                        throw std::logic_error("TODO Connection Error : " + error.message());
                    }
                    std::cout << "[" << now_str() << "] async_read len:" << len << std::endl;

                    // std::cout << "buffer[4] : " << (int)buffer[4] << std::endl;
                    // std::cout << "buffer[5] : " << (int)buffer[5] << std::endl;
                    // std::cout << "buffer[6] : " << (int)buffer[6] << std::endl;
                    // std::cout << "buffer[7] : " << (int)buffer[7] << std::endl;

                    auto frame = SIMPLEMSGQ_FRAME{buffer};

                    if(frame.check()){
                        std::cout << "before packet_len : " << frame.packet_len << std::endl;
                        frame.ntoh();
                        auto packet_len = frame.packet_len;
                        std::cout << "packet_len : " << packet_len << std::endl;
                        if(packet_len < sizeof(SIMPLEMSGQ_HEADER)){
                            throw std::logic_error("invalid packet len TODO fail control");
                        }

                        do_read_body(worker, packet_len);
                    }
                    else{
                        do_read_frame(worker);
                    }

                });
        }

        void
        do_read_body(
            IFWorker * worker, 
            unsigned int bodylen) {

                std::cout << "do_read_body bodylen: " << bodylen << std::endl;

            auto self(shared_from_this());
            boost::asio::async_read( socket,  boost::asio::buffer(boost::asio::buffer(buffer) + FRAME_SIZE), boost::asio::transfer_exactly(bodylen - FRAME_SIZE),
                [self, this, worker](const boost::system::error_code & error, std::size_t len){
                    std::cout << "[" << now_str() << "] async_read_body len: " << len << std::endl;
                    if(error){
                        throw std::logic_error("TODO Connection Error : " + error.message());
                    }
                    do_read_frame(worker);
                    SIMPLEMSGQ_HEADER * header = (SIMPLEMSGQ_HEADER *)buffer;
                    // header->frame.packet_len = ntohl(header->frame.packet_len);
                    // header->sequence = ntohl(header->sequence);
                    // header->type = ntohl(header->type);
                    // header->name = ntohl(header->name);
                    // header->code = ntohl(header->code);
                    // header->offset = ntohl(header->offset);
                    // header->count = ntohl(header->count);

                    header->ntoh();

                    // worker->do_read(socket, *header, buffer + sizeof(SIMPLEMSGQ_HEADER));
                    
                });
        }
    };
} // namespace simplemsgq
