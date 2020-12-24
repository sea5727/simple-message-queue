cmake_minimum_required(VERSION 3.0.0)
project(simple-message-queue VERSION 0.1.0)

set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

link_libraries(boost_thread boost_filesystem pthread)


# add_executable(simple-message-queue-chat chat_msgq.cpp)
add_executable(simple-message-queue-custom custom_msgq.cpp)
add_executable(client-custom client_custom.cpp)
# add_executable(client client.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
