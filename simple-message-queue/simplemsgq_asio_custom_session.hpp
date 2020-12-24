#pragma once

#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

#include "simplemsgq_asio_interface.hpp"

namespace simplemsgq
{
    class TCP_CUSTOM_HEADER{
    public:
        char frame[2];
        uint16_t length;
        uint32_t status_code;
        uint32_t msgname;
        uint32_t msgtype;
        uint64_t sequence_number;
    };

    constexpr static const unsigned int FRAME_SIZE = sizeof(uint32_t);
    constexpr static const unsigned int HEADER_SIZE = sizeof(TCP_CUSTOM_HEADER);

    class TCP_CUSTOM_BODY{
    public:
        uint32_t offset;
        uint32_t size;
    };

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
        run(SimplemsgqWorker * worker) {
            do_read_frame(worker);
        }
    private:
        void 
        do_read_frame(SimplemsgqWorker * worker) {
            auto self(this->shared_from_this());
            boost::asio::async_read( socket,  boost::asio::buffer(buffer), boost::asio::transfer_exactly(FRAME_SIZE),
                [self, this, worker](const boost::system::error_code & error, std::size_t len){
                    if(error){ // TODO connection clear
                        return;
                    }

                    TCP_CUSTOM_HEADER * header = (TCP_CUSTOM_HEADER *)buffer;
                    if((header->frame[0] & 0xff) == 0xfe && (header->frame[1] & 0xff) == 0xfe){
                        do_read_body(worker, header->length);
                    }
                    else{
                        do_read_frame(worker);
                    }

                });
        }

        void
        do_read_body(SimplemsgqWorker * worker, u_int16_t bodylen) {
            auto self(shared_from_this());
            boost::asio::async_read( socket,  boost::asio::buffer(buffer), boost::asio::transfer_exactly(bodylen),
                [self, this, worker](const boost::system::error_code & error, std::size_t len){
                    if(error){
                        // TODO connection clear
                        return;
                    }
                    //proc body
                    do_read_frame(worker);
                });
        }
    };
} // namespace simplemsgq
