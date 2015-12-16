cmake_minimum_required(VERSION 3.3)
project(xio_raft)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")
set(PATH_TO_ACCELIO "$ENV{$HOME}/accelio/")

link_directories(/usr/local/lib)
include_directories(${PATH_TO_ACCELIO}/include)

set(SOURCE_FILES main.cpp
        raft_state_interface.h
        follower.cpp follower.h
        raft_log.cpp raft_log.h
        candidate.cpp candidate.h
        leader.cpp leader.h
        rpc_requests.h
        xio_callbacks.h
        defines.h)

add_executable(xio_raft ${SOURCE_FILES})

target_link_libraries (xio_raft xio pthread)