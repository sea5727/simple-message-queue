#pragma once

#include <boost/asio.hpp>
#include <sys/sendfile.h>

#include "simplemsgq_worker_interface.hpp"
#include "simplemsgq_file_manager.hpp"


namespace simplemsgq{
    class ConsumerWorker : public IFWorker{
    private:
        std::shared_ptr<FileManager> fm;
    public:
        ConsumerWorker() = default;
        ConsumerWorker(std::shared_ptr<FileManager> fm)
            : fm{fm} { }

        void
        do_accept(boost::asio::ip::tcp::socket & socket) override{
            // TODO socket option settiing...
            std::cout << "ConsumerWorker do_accept callback\n";
        }

        void
        do_read(
            boost::asio::ip::tcp::socket & socket, 
            SIMPLEMSGQ_HEADER & header,
            char * body ) override {

            
            if(fm){
                auto sendinfo = (*fm).select_data(header.offset, header.count);
                auto filefd = std::get<0>(sendinfo);
                auto send_position = std::get<1>(sendinfo);
                auto send_size = std::get<2>(sendinfo);
                auto fd = socket.native_handle();

                SIMPLEMSGQ_HEADER response;
                response.init();
                response.frame.packet_len = sizeof(SIMPLEMSGQ_HEADER) + send_size;
                response.sequence = header.sequence;
                response.type = header.type;
                response.name = header.type;
                response.code = filefd == -1 ? -1 : 0;
                response.offset = 0;
                response.count = 0;

                response.hton();

                std::cout << "send response.sequence : " << response.sequence << std::endl;
                std::cout << "send response.type : " << response.type << std::endl;
                std::cout << "send response.name : " << response.name << std::endl;
                std::cout << "send response.code : " << response.code << std::endl;
                std::cout << "send response.offset : " << response.offset << std::endl;
                std::cout << "send response.count : " << response.count << std::endl;
                // TODO SEND RESPONSE send header
                std::this_thread::sleep_for(std::chrono::seconds(2));
                boost::asio::async_write(socket, boost::asio::buffer(&response, sizeof(SIMPLEMSGQ_HEADER)), 
                    [=](const boost::system::error_code & error, const size_t len){
                    std::cout << "async_write  :"  << error.message()  << ", len :" << len << std::endl;
                    if(fd != -1 || error){ return; }
                    auto ref_sendposition = send_position;
                    auto result = sendfile(fd, filefd, &ref_sendposition, send_size);
                    printf("sendfile.. filefd:%d, send_position:%ld, send_size:%d, fd:%d.. result:%d\n", filefd, ref_sendposition, send_size, fd, result);
                });

            }
            else {
                std::cout << "ConsumerWorker do_read callback fm false\n";
            }
        }




    };

} //namespace simplemsgq