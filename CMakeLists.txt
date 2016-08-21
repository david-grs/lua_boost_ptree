cmake_minimum_required(VERSION 2.8)

project(lua_boost_ptree)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14 -g -Wall -Wextra")

add_executable(lua_boost_ptree main.cpp)
target_link_libraries(lua_boost_ptree lua5.2)

