#pragma once

#include <memory.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <unistd.h>


namespace EventCLoop
{
    class TcpBuffer{
    private:
        constexpr static int DEFUALT_SIZE = 16 * 1024;
        int capacity = 0;
        int reader = 0;
        int writer = 0;
        ssize_t read_len = 0;
        char *pbuf = nullptr;
    public:
        TcpBuffer(int capacity = DEFUALT_SIZE)
            : capacity{capacity} { }

        ~TcpBuffer(){
            free_chunk();
        }

        inline char * get_buf() { return pbuf; }
        inline int get_capacity() { return capacity; }
        inline char * get_free_buf() { return pbuf + writer; }
        inline int get_free_size() {return (capacity - writer + 1); }
        inline void set_free_buf(int size) { writer += size; }

        int 
        alloc_chunk(int size){
            if(capacity < size){
                char * tmp = (char *)malloc(size);
                if(writer > reader + 1)
                    memcpy(tmp, pbuf + reader, writer - reader);
                free_chunk();
                capacity = size;
                pbuf = tmp;
            }
            else {
                if(pbuf == nullptr)
                    pbuf = (char *)malloc(capacity);
            }
            return 0;
        }
        void
        free_chunk(){
            if(pbuf) free(pbuf);
            pbuf = nullptr;
            reader = 0;
            writer = 0;

        }
        char *
        move_chunk(){
            memmove(pbuf, (pbuf + reader), writer - reader);
            writer = writer - reader;
            reader = 0;
            return nullptr;
        }
        int 
        read_chunk(int fd){
            if(pbuf == nullptr)
                alloc_chunk(capacity);
            
            int rbytes = 0;
            while(1){
                rbytes = recv(fd, pbuf + writer, capacity - writer, 0);
                if((rbytes <= 0) && errno == EINTR || 
                    (rbytes <= 0) &&  errno == EAGAIN)
                    continue;
                else if (rbytes < 0) {
                    return -1;
                }
                break;
            }

            if(rbytes == 0) {
                read_len = 0;
                return 0;
            }

            writer += rbytes;
            read_len = (writer - reader);
            return read_len;
        };
        int 
        dispatch_chunk(char * &p, std::function<int(char *, ssize_t)> pf_dispatch){
            if(pf_dispatch == nullptr){
                int length = writer - reader;
                p = pbuf + reader;
                reader = writer;
                return length;
            }

            int length = pf_dispatch(pbuf + reader, writer - reader);
            if(length > 0){
                p = pbuf + reader;
                reader += length;
            }
            else {
                if(writer >= capacity){
                    move_chunk();
                }
            }

            return length;
        }
    };
}