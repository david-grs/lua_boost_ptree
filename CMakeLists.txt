cmake_minimum_required(VERSION 2.8)

project(lua_boost_ptree)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14 -g -Wall -Wextra")

#find_package(Boost REQUIRED COMPONENTS thread date_time system regex filesystem serialization iostreams program_options)
#include_directories(${Boost_INCLUDE_DIRS})

add_executable(lua_boost_ptree main.cpp)
#target_link_libraries(lua_boost_ptree ${Boost_LIBRARIES} pthread lua5.2)
target_link_libraries(lua_boost_ptree ${Boost_LIBRARIES} pthread lua5.2)

