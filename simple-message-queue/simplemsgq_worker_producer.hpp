#pragma once

#include "simplemsgq_worker_interface.hpp"
#include "simplemsgq_file_manager.hpp"

#include <boost/asio.hpp>
#include <boost/asio/socket_base.hpp>

namespace simplemsgq
{
    class ProducerWorker : public IFWorker{
    private:
        std::shared_ptr<FileManager> fm;
    public:
        ProducerWorker() = default;
        ProducerWorker(std::shared_ptr<FileManager> fm)
            : fm{fm} { }
    
        void
        do_accept(boost::asio::ip::tcp::socket & socket) override{
            // TODO socket option settiing...
            socket.non_blocking(true);
            // socket.native_non_blocking(true);
        }
        void
        do_read(
            boost::asio::ip::tcp::socket & socket, 
            SIMPLEMSGQ_HEADER * header,
            char * body) override{
            if(fm){
                auto bodylen = header->get_body_len();
                std::cout << "do_read bodylen:" << bodylen << ", body:" << body << std::endl;
                (*fm).insert_data(body, bodylen);
                // TODO SEND RESPONSE
            }
        }
        void
        do_read(
            boost::asio::ip::tcp::socket & socket, 
            SIMPLEMSGQ_HEADER & header,
            char * body) override{
            if(fm){
                auto bodylen = header.get_body_len();
                std::cout << "do_read bodylen:" << bodylen << ", body:" << body << std::endl;
                (*fm).insert_data(body, bodylen);
                // TODO SEND RESPONSE
            }
            //  (*m_manager).insert_data(buffer, len);
        }

    };
}