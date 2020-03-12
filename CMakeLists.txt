cmake_minimum_required(VERSION 3.12.4)
project(MemLeak)

set(CMAKE_CXX_STANDARD 14)

if(UNIX)
    SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread")
endif()

add_executable(MemLeak main.cpp src/MemRecord.h)
if(WIN32)

elseif(UNIX)
    TARGET_LINK_LIBRARIES(MemLeak dl)
endif()