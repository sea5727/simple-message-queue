#pragma once

#include "simplemsgq_asio_interface.hpp"
#include "simplemsgq_file_manager.hpp"

#include <boost/asio.hpp>
#include <boost/asio/socket_base.hpp>

namespace simplemsgq
{
    class FileInserter : public SimplemsgqWorker{
    private:
        std::shared_ptr<FileManager> fm;
    public:
        FileInserter() = default;
        FileInserter(std::shared_ptr<FileManager> fm)
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
            SIMPLEMSGQ_HEADER & header,
            char * body) override{
            std::cout << "FileInserter do_read callback\n";
            if(fm){
                auto bodylen = header.get_body_len();
                (*fm).insert_data(body, bodylen);
                // TODO SEND RESPONSE
            }
            //  (*m_manager).insert_data(buffer, len);
        }

        // void
        // do_read(
        //     boost::asio::ip::tcp::socket & socket, 
        //     const char *buffer, 
        //     const unsigned int len) override{
        //     std::cout << "FileInserter do_read callback\n";
        //     if(fm){
        //         (*fm).insert_data(buffer, len);
        //         // TODO SEND RESPONSE
        //     }
        //     //  (*m_manager).insert_data(buffer, len);
        // }
    };
}