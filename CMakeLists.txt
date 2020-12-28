cmake_minimum_required(VERSION 3.0.0)
project(simple-message-queue VERSION 0.1.0)

set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

link_libraries(pthread)

add_executable(main main.cpp)



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
