cmake_minimum_required(VERSION 3.16)
project(oj-judge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(oj-judge
    src/main.cpp
    src/judge_server.cpp
    src/runner.cpp
    src/sandbox.cpp
)

target_include_directories(oj-judge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(oj-judge PRIVATE -Wall -Wextra -Wpedantic)
