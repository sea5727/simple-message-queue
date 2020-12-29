#pragma once


#include "simplemsgq.hpp"


namespace simplemsgq
{
    class FileManager;
    class FileManagerBuilder{
    friend class FileManager;
    public:
        static
        std::shared_ptr<FileManager>
        build(const std::string & basepath){

            std::vector<indexmmap_info> mmaps;

            auto is_directory = boost::filesystem::is_directory(basepath);
            if(!is_directory){
                throw std::runtime_error("invalid directory path" + basepath);
            }
            auto files = search_index_file(basepath);
            std::sort(files.begin(), files.end()); // for validation
            for(auto & fileinfo : files){
                auto index = std::get<0>(fileinfo);
                auto filename = std::get<1>(fileinfo);

                mmaps.emplace_back(open_mmap(index, filename));
            }

            return std::make_shared<FileManager>(basepath, mmaps);
        }
    private:
        static 
        std::vector<std::tuple<unsigned int /*index*/, std::string /*filename*/>>
        search_index_file(const std::string & basepath){

            std::vector<std::tuple<unsigned int /*index*/, std::string /*filename*/>> indexfiles;

            auto directories = boost::filesystem::directory_iterator{basepath};

            for(const auto & entry : directories){
                auto path = boost::filesystem::path{entry};

                auto stem = path.stem().string();
                auto filename = path.filename().string();
                auto extension = path.extension().string();

                if(!extension.empty() && extension.compare(".index") == 0 &&
                    !stem.empty() && stem.find_first_not_of("0123456789") == std::string::npos){ // index file name include only number

                    auto index = std::stoi(stem);
                    auto expected_name = make_indexfilename(basepath, index);
                    auto real_name = basepath + "/" + filename;

                    if(expected_name.compare(real_name) == 0){
                        indexfiles.emplace_back(index, expected_name);

                    }
                }
            }
            return indexfiles;
        }

        static
        indexmmap_info
        create_mmap(
            const std::string & filename,
            const unsigned int index) {

            int fd = open(filename.c_str(), O_CREAT | O_RDWR , 0644 );
            if(fd == -1){
                throw std::runtime_error("cannot open file " + filename);
            }

            INDEX_FILE buffer;
            auto size = sizeof(buffer);

            int write_len = write(fd, &buffer, size); // write index
            if(write_len != size){
                throw std::runtime_error("cannot write file " + filename);
            }
            
            void *map = (void *)mmap(0, size, PROT_READ | PROT_WRITE , MAP_SHARED , fd, 0);
            if (map == MAP_FAILED) {
                close(fd);
                throw std::runtime_error("cannot open mmap");
            }

            // printf( "[%s:%d] create mmap..map:%p, size:%u, file_name:%s\n", 
            //     __func__, 
            //     __LINE__,
            //     map,
            //     size,
            //     filename.c_str());
            
            INDEX_FILE * info = (INDEX_FILE *)map;
            close(fd);

            return std::make_tuple(index, size, info);
        }

        static
        indexmmap_info
        open_mmap(
            const unsigned int index, 
            const std::string & filename){

            struct stat fileInfo = {0};
            
            int fd = open(filename.c_str(), O_RDWR);
            if(fd == -1){
                throw std::runtime_error("cannot open file " + filename);
            }
            
            if (fstat(fd, &fileInfo) == -1) {
                close(fd);
                throw std::runtime_error("cannot open file fstat");
            }
            if (fileInfo.st_size == 0) {
                close(fd);
                remove(filename.c_str()); // if file size is 0, remove file
                throw std::runtime_error("invalid file.. please restart program");
            }

            void *map = (void *)mmap(0, fileInfo.st_size, PROT_READ | PROT_WRITE , MAP_SHARED , fd, 0);
            if (map == MAP_FAILED) {
                close(fd);
                throw std::runtime_error("cannot open mmap");
            }
            
            INDEX_FILE * info = (INDEX_FILE *)map;
            
            close(fd);

            printf( "[%s:%d] create mmap..map:%p, size:%u, file_name:%s, next_index:%u, next_data_position:%u\n", 
                __func__, 
                __LINE__,
                map,
                fileInfo.st_size,
                filename.c_str(),
                info->header.next_index,
                info->header.next_data_position);

            
            return std::make_tuple(index, fileInfo.st_size, info);
        }

        static
        std::string
        make_indexfilename(
            const std::string & basepath, 
            const unsigned int index){

            std::stringstream ss;
            ss  << basepath
                << "/"
                << std::setfill('0') 
                << std::setw(16) 
                << index
                << ".index";
            return ss.str();
        }

        static
        std::string
        make_datafilename(
            const std::string & basepath, 
            const unsigned int index){

            std::stringstream ss;
            ss  << basepath
                << "/"
                << std::setfill('0') 
                << std::setw(16) 
                << index
                << ".data";
            return ss.str();
        }

    };
}