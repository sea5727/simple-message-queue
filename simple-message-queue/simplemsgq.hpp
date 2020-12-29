#pragma once

#include <arpa/inet.h>
#include <memory.h>




#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <tuple>
#include <vector>
#include <boost/filesystem.hpp>
#include <exception>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/mman.h>


#include "define.hpp"
#include "file_manager_builder.hpp"
#include "file_manager.hpp"

#include "server_consumer.hpp"
#include "server_producer.hpp"
#include "client_consumer.hpp"
#include "client_producer.hpp"