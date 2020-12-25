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
        TcpSessionCustom(boost::asio::ip::tcp::socket socket)
            : socket{std::move(socket)}
            , buffer{}{

        }

        void 
        run(IFWorker * worker) {
            do_read_frame(worker);
        }
    private:
        void 
        do_read_frame(IFWorker * worker) {
            std::cout << "do_read_frame start exactly FRAME_SIZE : " << FRAME_SIZE << std::endl;
            auto self(this->shared_from_this());

            // boost::asio::async_read(socket, boost::asio::buffer(buffer), 
            //     [](const boost::system::error_code & error, const size_t len){

            // });

            boost::asio::async_read( socket,  boost::asio::buffer(buffer), boost::asio::transfer_exactly(FRAME_SIZE),
                [self, this, worker](const boost::system::error_code & error, const std::size_t & len){
                    if(error){ // TODO connection clear
                        return;
                    }

                    auto frame = SIMPLEMSGQ_FRAME{buffer};

                    if(frame.check()){
                        frame.ntoh();
                        auto packet_len = frame.packet_len;
                        std::cout << "check succ : " << std::dec << packet_len << std::endl;
                        do_read_body(worker, packet_len);
                    }
                    else{
                        std::cout << "check fail\n";
                        do_read_frame(worker);
                    }

                });
        }

        void
        do_read_body(IFWorker * worker, unsigned int bodylen) {
            auto self(shared_from_this());
            boost::asio::async_read( socket,  boost::asio::buffer(boost::asio::buffer(buffer) + FRAME_SIZE), boost::asio::transfer_exactly(bodylen - FRAME_SIZE),
                [self, this, worker](const boost::system::error_code & error, std::size_t len){
                    if(error){
                        // TODO connection clear
                        return;
                    }

                    SIMPLEMSGQ_HEADER * header = (SIMPLEMSGQ_HEADER *)buffer;
                    header->ntoh();
                    std::cout << "count : " << header->count << std::endl;
                    std::cout << "offset : " << header->offset << std::endl;
                    std::cout << "sequence : " << header->sequence << std::endl;
                    std::cout << "type : " << header->type << std::endl;
                    std::cout << "packet_len : " << header->frame.packet_len << std::endl;
                    

                    worker->do_read(socket, *header, buffer + sizeof(SIMPLEMSGQ_HEADER));

                    
                    do_read_frame(worker);
                });
        }
    };
} // namespace simplemsgq
