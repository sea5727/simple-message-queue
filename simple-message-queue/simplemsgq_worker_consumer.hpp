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
            socket.non_blocking(true);
            std::cout << "[" << now_str() << "] Do Accept\n";
        }
        void
        do_read(
            boost::asio::ip::tcp::socket & socket, 
            SIMPLEMSGQ_HEADER * header,
            char * body ) override {

            std::cout << "[" << now_str() << "] Do Read Offset : " << header->offset<< ", count : " << header->count << std::endl;
            
            if(fm){
                // auto started = std::chrono::high_resolution_clock::now();
                auto sendinfo = (*fm).select_data(header->offset, header->count);
                auto filefd = std::get<0>(sendinfo);
                auto send_position = std::get<1>(sendinfo);
                auto send_size = std::get<2>(sendinfo);
                // send_size = 0;
                // auto done = std::chrono::high_resolution_clock::now();
                // std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(done-started).count() << "ms\n";

                SIMPLEMSGQ_HEADER response;
                response.init();
                response.frame.packet_len = sizeof(SIMPLEMSGQ_HEADER) + send_size;
                response.sequence = header->sequence;
                response.type = header->type;
                response.name = header->name;
                response.code = filefd == -1 ? -1 : 0;
                response.offset = header->offset;
                response.count = send_size == 0 ? 0 : 1;
                std::cout << "[" << now_str() << "] send try pakcet_len : " << response.frame.packet_len << ", offset : " << response.offset << ", count : " << response.count << std::endl;
                response.hton();

                

                boost::system::error_code error;
                // auto len = boost::asio::write(socket, boost::asio::buffer(&response, sizeof(SIMPLEMSGQ_HEADER)), error);
                auto len = ::write(socket.native_handle(), &response,  sizeof(SIMPLEMSGQ_HEADER));
                
                if(error){ return; }
                if(send_size <= 0 || filefd == -1) { 
                    std::cout << "[" << now_str() << "] send result len:" << len << std::endl;
                    return ; 
                }
                
                auto result = ::sendfile(socket.native_handle(), filefd, &send_position, send_size);
                close(filefd);

                std::cout << "[" << now_str() << "] send + sendfile result len:" << len + result << std::endl;

                // std::cout << "[" << now_str() << "] SendFile X Write len : " << result << std::endl;
            }
            else {
                std::cout << "ConsumerWorker do_read callback fm false\n";
            }
        }


        void
        do_read(
            boost::asio::ip::tcp::socket & socket, 
            SIMPLEMSGQ_HEADER & header,
            char * body ) override {
            std::cout << "[" << now_str() << "] Do Read Offset : " << header.offset<< ", count : " << header.count << std::endl;
            
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
                response.offset = header.offset;
                response.count = send_size == 0 ? 0 : 1;

                response.hton();
                std::cout << "[" << now_str() << "]async_write try\n";
                boost::system::error_code error;
                auto len = boost::asio::write(socket, boost::asio::buffer(&response, sizeof(SIMPLEMSGQ_HEADER)), error);
                std::cout << "[" << now_str() << "] Write len :" << len << ", fd:" << fd << ", error:" << error.message() << std::endl;

                char buffer2[1024] = "Aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

                // auto sendfilelen = boost::asio::write(socket, boost::asio::buffer(buffer2, send_size), error);
                
                return;
                if(error){ return; }
                
                if(send_size <= 0 || filefd == -1) { return ; }
                // auto fd = socket.native_handle();
                auto ref_sendposition = send_position;
                auto result = sendfile(fd, filefd, &ref_sendposition, send_size);
                std::cout << "[" << now_str() << "] SendFile len : " << result << std::endl;
            }
            else {
                std::cout << "ConsumerWorker do_read callback fm false\n";
            }
        }




    };

} //namespace simplemsgq