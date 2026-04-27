cmake_minimum_required(VERSION 4.1)
project(atta1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(atta1 main.cpp
        Logger.h
        Parser.h
        Session.h
        Session.cpp
        Server.cpp
        Server.h
        App.cpp
        App.h
        Protocol.h
        ConnectionManager.cpp
        ConnectionManager.h
        BroadCast.h
)

find_package(Boost REQUIRED)

target_include_directories(atta1 PRIVATE ${Boost_INCLUDE_DIRS})


if (TARGET Boost::headers)
    target_link_libraries(atta1 PRIVATE Boost::headers)
endif()
