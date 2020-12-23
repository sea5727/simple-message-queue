#pragma once

#include "simplemsgq_asio_interface.hpp"
#include "simplemsgq_file_manager.hpp"

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
            std::cout << "FileInserter do_accept callback\n";
        }

        void
        do_read(
            boost::asio::ip::tcp::socket & socket, 
            const char *buffer, 
            const unsigned int len) override{
            std::cout << "FileInserter do_read callback\n";
            if(fm){
                (*fm).insert_data(buffer, len);
                // TODO SEND RESPONSE
            }
            //  (*m_manager).insert_data(buffer, len);
        }
    };
}