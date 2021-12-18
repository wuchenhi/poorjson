cmake_minimum_required(VERSION 3.10)


# C++11 编译
set(CMAKE_CXX_STANDARD 11)


set(TEST_SRC test.cpp)
ADD_EXECUTABLE(test ${TEST_SRC})


# 链接 MySharedLib 库
target_link_libraries(test poorjson)
