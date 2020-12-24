#pragma once

#include <memory.h>

namespace simplemsgq
{
    using connect_handler = std::function<void(const boost::system::error_code & , const boost::asio::ip::tcp::endpoint & )>;
    using read_handler = std::function<void(const boost::system::error_code & , const size_t &)>;
    using write_handler = std::function<void(const boost::system::error_code & , const size_t &)>;
    
    class Define{
    public:
        static constexpr unsigned int SEGMENT_PACKET_COUNT = 8;
    };


    class SIMPLEMSGQ_FRAME{
    public:
        SIMPLEMSGQ_FRAME(){}
        SIMPLEMSGQ_FRAME(char * buffer){
            frame[0] = buffer[0];
            frame[1] = buffer[1];
            frame[2] = buffer[2];
            frame[3] = buffer[3];
            memcpy(&packet_len, buffer + 4, sizeof(packet_len));
        }
        char frame[4]; //0xfb, 0xfc, 0xfd
        unsigned int packet_len;
        void init(){
            frame[0] = 0xfb;
            frame[1] = 0xfc;
            frame[2] = 0xfd;
            frame[4] = 0xfe;
            packet_len = 0;
        }
        bool check(){
            if( (frame[0] & 0xff) == 0xfb && 
                (frame[1] & 0xff) == 0xfc &&
                (frame[2] & 0xff) == 0xfd){
                return true;
            }
            return false;
        }
        void ntoh(){
            packet_len = ntohl(packet_len);
        }
        void hton(){
            packet_len = htonl(packet_len);
        }        
        
    };

    // #define MAX_MSG_SIZE                            (8*1024)
    class SIMPLEMSGQ_HEADER{
    public:
        SIMPLEMSGQ_FRAME frame;
        unsigned int sequence;
        unsigned int type;
        unsigned int name;
        int code;
        int offset;
        unsigned int count;
        void init(){
            frame.init();
            sequence = 0;
            type = 0;
            name = 0;
            code = 0;
            offset = 0;
            count = 0;
        }
        bool check(){
            return frame.check();
        }
        void ntoh(){
            frame.ntoh();
            sequence = ntohl(sequence);
            type = ntohl(type);
            name = ntohl(name);
            code = ntohl(code);
            offset = ntohl(offset);
            count = ntohl(count);
        }
        void hton(){
            frame.hton();
            sequence = htonl(sequence);
            type = htonl(type);
            name = htonl(name);
            code = htonl(code);
            offset = htonl(offset);
            count = htonl(count);
        }
        unsigned int get_body_len(){
            return frame.packet_len - sizeof(SIMPLEMSGQ_HEADER);
        }
    };
 
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
    const static unsigned int FRAME_SIZE = sizeof(SIMPLEMSGQ_FRAME);
    
}