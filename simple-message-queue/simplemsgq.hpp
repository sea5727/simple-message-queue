#pragma once

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

#include <memory.h>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "simplemsgq_define.hpp"
#include "simplemsgq_client_consumer.hpp"
#include "simplemsgq_worker_interface.hpp"

#include "simplemsgq_file_manager.hpp"
#include "simplemsgq_worker_consumer.hpp"
#include "simplemsgq_worker_producer.hpp"
#include "simplemsgq_server_acceptor.hpp"
#include "simplemsgq_server_acceptor2.hpp"
#include "simplemsgq_server_session.hpp"
#include "simplemsgq_server_session2.hpp"

#include "simplemsgq_asio_chat_session.hpp"

#include "simplemsgq_client_consumer.hpp"
#include "simplemsgq_client_producer.hpp"

// #include "simplemsgq_simqclient.hpp"

namespace simplemsgq
{


}