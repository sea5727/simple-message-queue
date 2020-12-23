#pragma once

#include <memory.h>

namespace simplemsgq
{
    
    
    class Define{
    public:
        static constexpr unsigned int SEGMENT_PACKET_COUNT = 8;
    };

    // #define MAX_MSG_SIZE                            (8*1024)

    class INDEX_HEADER{
    public:
        char frame[4]; //0xfb, 0xfc, 0xfd, 0xfe
        unsigned int next_index;
        unsigned int next_data_position;
    };

    class INDEX_FORMAT{
    public:
        unsigned int index;
        unsigned int position;
        unsigned int size;
    };

    class INDEX_FILE{
    public:
        INDEX_FILE(){
            header.frame[0] = 0xfb;
            header.frame[1] = 0xfc;
            header.frame[2] = 0xfd;
            header.frame[3] = 0xfe;
            header.next_index = 0;
            header.next_data_position = 0;
            memset(bodys, 0x00, sizeof(INDEX_FORMAT) * Define::SEGMENT_PACKET_COUNT);
        }
    
        INDEX_HEADER header;
        INDEX_FORMAT bodys[Define::SEGMENT_PACKET_COUNT];
    };

    using indexmmap_info = std::tuple<unsigned int /*index*/, unsigned int /*size*/, INDEX_FILE * /*index_file_mmap*/>;
    
}