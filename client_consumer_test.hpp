#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "simple-message-queue/simplemsgq.hpp"

namespace test
{
    class asdf{

    };
    class TestConsumer : public simplemsgq::ClientConsumer{
    public:
        TestConsumer() = default;
        void 
        do_select(simplemsgq::SIMPLEMSGQ_PACKET & packet){
            std::cout << "Client Program recv select \n";
        }
        void 
        do_connect(){
            
        }
    };
}