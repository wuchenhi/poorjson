cmake_minimum_required (VERSION 3.10)
project (poorjson)


# C++11 编译
set(CMAKE_CXX_STANDARD 11)


add_library(poorjson SHARED poorjson.cpp poorjson.h )
