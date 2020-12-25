#pragma once

#include "simplemsgq_define.hpp"

namespace simplemsgq
{
    class ClientConsumer{
    public:
        virtual void do_select(SIMPLEMSGQ_PACKET & packet) = 0;
        virtual void do_connect() = 0;
        
    };
}