cmake_minimum_required(VERSION 3.0.0)
project(simple-message-queue VERSION 0.1.0)

set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

include_directories(.)
link_directories(.)
link_libraries(pthread boost_filesystem)

add_executable(server server.cpp)
add_executable(client_consumer client_consumer.cpp)
add_executable(client_producer client_producer.cpp)



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
