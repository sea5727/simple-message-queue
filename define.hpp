#pragma once

namespace simplemsgq
{
    class Define{
    public:
        static constexpr unsigned int SEGMENT_PACKET_COUNT = 128;
        // static constexpr EndiaHelper endianHelper;
    };


    class SIMPLEMSGQ_FRAME{
    public:
        char frame[4]; //0xfb, 0xfc, 0xfd
        unsigned int packet_len;
        inline
        void 
        init(){
            frame[0] = 0xfb;
            frame[1] = 0xfc;
            frame[2] = 0xfd;
            frame[3] = 0xfe;
            packet_len = 0;
        }
        inline
        bool 
        check(){
            if( (frame[0] & 0xff) == 0xfb && 
                (frame[1] & 0xff) == 0xfc &&
                (frame[2] & 0xff) == 0xfd && 
                (frame[3] & 0xff) == 0xfe){
                return true;
            }
            std::cout << "frame chaeck fail!!!" << std::endl;
            for(int i = 0 ; i < 4 ; i++){
                std::cout << "frame[" << i << "] : " << (int)frame[i] << std::endl;
            }
            return false;
        }
        inline
        void 
        ntoh(){
            packet_len = ::ntohl(packet_len);
        }
        inline
        void 
        hton(){
            packet_len = ::htonl(packet_len);
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

        inline
        void 
        init(){
            frame.init();
            sequence = 0;
            type = 0;
            name = 0;
            code = 0;
            offset = 0;
            count = 0;
        }
        inline
        bool 
        check(){
            return frame.check();
        }
        inline
        void 
        ntoh(){
            frame.ntoh();
            sequence = ::ntohl(sequence);
            type = ::ntohl(type);
            name = ::ntohl(name);
            code = ::ntohl(code);
            offset = ::ntohl(offset);
            count = ::ntohl(count);
        }
        inline
        void 
        hton(){
            frame.hton();
            sequence = ::htonl(sequence);
            type = ::htonl(type);
            name = ::htonl(name);
            code = ::htonl(code);
            offset = ::htonl(offset);
            count = ::htonl(count);
        }
        inline
        unsigned int 
        get_body_len(){
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