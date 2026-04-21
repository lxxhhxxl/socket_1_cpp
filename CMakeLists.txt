cmake_minimum_required(VERSION 3.20)
project(tcp_demo VERSION 1.0.0 LANGUAGES CXX)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
add_compile_options(-Wall -Wextra -Wpedantic)

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 可执行文件
add_executable(tcp_demo
    src/main.cpp
    src/tcp_server.cpp
    src/tcp_client.cpp
)

# 链接线程库（服务器多线程需要）
find_package(Threads REQUIRED)
target_link_libraries(tcp_demo PRIVATE Threads::Threads)