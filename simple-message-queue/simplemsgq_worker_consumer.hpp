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
                response.hton();

                boost::system::error_code error;
                auto len = boost::asio::write(socket, boost::asio::buffer(&response, sizeof(SIMPLEMSGQ_HEADER)), error);
                if(error){ return; }
                if(send_size <= 0 || filefd == -1) { return ; }
                auto ref_sendposition = send_position;

                boost::system::error_code nonblock_ec;

                socket.non_blocking(false);
                socket.native_non_blocking(false, nonblock_ec);
                
                auto result = sendfile(socket.native_handle(), filefd, &ref_sendposition, send_size);
                std::cout << "sendfile result:" << result << std::endl;
                auto sendfileec = boost::system::error_code(result < 0 ? errno : 0, boost::asio::error::get_system_category());
                std::cout << "strerror: " << strerror(errno) << std::endl;
                if(sendfileec == boost::asio::error::interrupted){
                    std::cout << "interrupted\n";
                }
                if(sendfileec == boost::asio::error::would_block){
                    std::cout << "would_block\n";
                }
                if(sendfileec == boost::asio::error::try_again){
                    std::cout << "try again\n";
                }
                if(sendfileec || result == 0){
                    std::cout << "error occur..\n";
                }

                // std::cout << "[" << now_str() << "] SendFile len : " << result << std::endl;
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