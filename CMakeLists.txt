cmake_minimum_required(VERSION 3.0.0)
project(simple-message-queue VERSION 0.1.0)

set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

link_libraries(boost_thread boost_filesystem pthread)


# add_executable(simple-message-queue-chat chat_msgq.cpp)
# add_executable(simple-message-queue-custom custom_msgq.cpp)
# add_executable(consumer-test3 client_consumer_test3.cpp)
# add_executable(consumer-test client_consumer_test.cpp)
# add_executable(producer-test client_producer_test.cpp)
# add_executable(client client.cpp)
# add_executable(all-test msgq_all.cpp)
# add_executable(poll_test poll_test.cpp)
# add_executable(client_consumer_test2 client_consumer_test2.cpp)
# add_executable(boost_server boost_server.cpp)
# add_executable(boost_client boost_client.cpp)
add_executable(test-msgq-1 msgq-1.cpp)
add_executable(test-msgq-2 msgq-2.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
