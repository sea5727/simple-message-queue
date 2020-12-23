#pragma once

#include <sys/sendfile.h>

#include "simplemsgq_asio_interface.hpp"
#include "simplemsgq_file_manager.hpp"


namespace simplemsgq{
    class FileSelecter : public SimplemsgqWorker{
    private:
        std::shared_ptr<FileManager> fm;
    public:
        FileSelecter() = default;
        FileSelecter(std::shared_ptr<FileManager> fm)
            : fm{fm} { }

        void
        do_accept(boost::asio::ip::tcp::socket & socket) override{
            // TODO socket option settiing...
            std::cout << "FileSelecter do_accept callback\n";
        }

        void
        do_read(
            boost::asio::ip::tcp::socket & socket, 
            const char *buffer, 
            const unsigned int len) override {

            std::cout << "FileSelecter do_read callback\n";
            if(fm){
                auto sendinfo = (*fm).select_data(buffer, len);
                auto filefd = std::get<0>(sendinfo);
                auto send_position = std::get<1>(sendinfo);
                auto send_size = std::get<2>(sendinfo);
                auto fd = socket.native_handle();
                // TODO SEND RESPONSE
                auto result = sendfile(fd, filefd, &send_position, send_size);
                printf("sendfile.. filefd:%d, send_position:%ld, send_size:%d, fd:%d.. result:%d\n", filefd, send_position, send_size, fd, result);
            }
        }


    };

} //namespace simplemsgq