#pragma once

#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

#include "simplemsgq_define.hpp"
#include "simplemsgq_worker_interface.hpp"

namespace simplemsgq
{
    class ServerSession : public std::enable_shared_from_this<ServerSession>{
    private:
        boost::asio::ip::tcp::socket socket;
        char buffer[8096];
    public:
        ServerSession(
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

                    SIMPLEMSGQ_FRAME * frame = (SIMPLEMSGQ_FRAME *)buffer;

                    if(frame->check()){
                        auto packet_len = ntohl(frame->packet_len);
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
                    
                    SIMPLEMSGQ_HEADER * header = (SIMPLEMSGQ_HEADER *)buffer;
                    header->ntoh();


                    // SIMPLEMSGQ_HEADER response;
                    // response.init();
                    // response.frame.packet_len = sizeof(SIMPLEMSGQ_HEADER) + 0;
                    // response.sequence = header->sequence;
                    // response.type = header->type;
                    // response.name = header->name;
                    // response.code = -1;
                    // response.offset = header->offset;
                    // response.count = 0;

                    // response.hton();
                
                    // boost::system::error_code ec;
                    // auto slen = boost::asio::write(socket, boost::asio::buffer(&response, sizeof(SIMPLEMSGQ_HEADER)), ec);

                    std::cout << "worker do read\n";
                    worker->do_read(socket, header, buffer + header->frame.packet_len);
                    do_read_frame(worker);
                    
                });
        }
    };
} // namespace simplemsgq
