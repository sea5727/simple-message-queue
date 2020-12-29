#pragma once



#include "define.hpp"
#include "file_manager_builder.hpp"

namespace simplemsgq
{
    
    class FileManager{
    private:
        std::string                 basepath;
        std::vector<indexmmap_info> mmaps;
    public:
        FileManager() = default;
        FileManager(
            const std::string & basepath, 
            std::vector<indexmmap_info> & mmaps) 
                : basepath(basepath)
                , mmaps(mmaps) { }


        void 
        insert_data(
            const char * buffer, 
            unsigned int len) {

            if(mmaps.size() == 0){ // need new index file
                
                auto filename = FileManagerBuilder::make_indexfilename(basepath, 0);
                auto mmap = FileManagerBuilder::create_mmap(filename, 0);

                mmaps.emplace_back(mmap);
            }

            auto fileindex = std::get<0>(mmaps.back()); // indexfile name
            auto filesize = std::get<1>(mmaps.back());
            auto mmap = std::get<2>(mmaps.back());

            auto next_index = mmap->header.next_index; // index in indexfile 0 ~ SEGMENT_PACKET_COUNT

            if(next_index >= Define::SEGMENT_PACKET_COUNT){ // need next index file
                std::cout << "need need index file\n";
                auto new_index = fileindex + next_index;

                auto new_file = FileManagerBuilder::make_indexfilename(basepath, new_index);
                auto new_mmap = FileManagerBuilder::create_mmap(new_file , new_index);

                fileindex = std::get<0>(new_mmap);
                mmap = std::get<2>(new_mmap);
                mmaps.emplace_back(new_mmap);
            }
            write_msgq(fileindex, buffer, len);
            update_mmap(mmap, len);

        }


        std::tuple< int         /*filefd*/, 
                    long int    /*send_position*/, 
                    int         /*sendsize*/>
        select_data(
            unsigned int offset, 
            unsigned int count ){

            auto request_index = offset;
            
            if(mmaps.empty()){
                // throw std::runtime_error("mmap is empty TODO send fail"); // TODO sendfail
                return std::make_tuple(-1, -1, 0);
            }
            auto last = mmaps.back();
            auto f = mmaps.front();
            
            auto last_fileindex = std::get<0>(last);
            auto list_filesize = std::get<1>(last);
            auto last_indexdata = std::get<2>(last);

            auto next_index = last_indexdata->header.next_index;

            if(last_fileindex + next_index -1 < request_index){
                return std::make_tuple(-1, -1, 0);
                // throw std::runtime_error("index over fail"); // TODO sendfail
            }
            std::cout << "last_fileindex: "  << last_fileindex << std::endl;
            std::cout << "next_index: "  << next_index << std::endl;
            std::cout << "request_index: "  << request_index << std::endl;

            auto lower = std::lower_bound(mmaps.rbegin(), mmaps.rend(), std::make_tuple(request_index, 0, nullptr),
                [](const indexmmap_info & a, const indexmmap_info & b){
                    return std::get<0>(a) > std::get<0>(b);
                });

            if(lower == mmaps.rend()){
                return std::make_tuple(-1, -1, 0);
                // throw std::runtime_error("index not found"); // TODO sendfail
            }
            auto file_index = std::get<0>(*lower);
            auto file_size = std::get<1>(*lower);
            auto find_data = std::get<2>(*lower);
            auto searched = request_index - file_index;
            auto filename = FileManagerBuilder::make_datafilename(basepath, file_index);
            auto send_size = find_data->bodys[searched].size;
            auto send_position = find_data->bodys[searched].position;
            auto filefd = open(filename.c_str(), O_RDONLY, 0644 ); 

            if(filefd == -1){
                // throw std::runtime_error("invalid directory path" + basepath);
                return std::make_tuple(-1, -1, 0);
            }
                

            return std::make_tuple(filefd, send_position, send_size);
            
        }
        void write_msgq(
            const unsigned int index, 
            const char * buffer, 
            const unsigned int len){
            
            auto file = FileManagerBuilder::make_datafilename(basepath, index);

            int fd = open(file.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
            if(fd == -1) {
                throw std::runtime_error("open file fail");
            }

            auto ret = write(fd, buffer, len);
            if(ret != len) {
                throw std::runtime_error("write file fail");
            }

            close(fd);
        }

        void 
        update_mmap( 
            INDEX_FILE * mmap, 
            unsigned int write_len) {

            auto index = mmap->header.next_index;
            auto position = mmap->header.next_data_position;

            mmap->bodys[index].index = index;
            mmap->bodys[index].position = position;
            mmap->bodys[index].size = write_len;

            mmap->header.next_index += 1;
            mmap->header.next_data_position += write_len;
        }
    }; 




}