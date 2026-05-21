cmake_minimum_required(VERSION 3.16)
project(oj-judge-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

set(GTEST_INCLUDE_DIR /usr/include)
set(GTEST_LIBRARY /usr/lib/x86_64-linux-gnu/libgtest.a)
set(GTEST_MAIN_LIBRARY /usr/lib/x86_64-linux-gnu/libgtest_main.a)

add_executable(oj-judge-test
    test_runner.cpp
    test_sandbox.cpp
    test_judge_server.cpp
    test_main.cpp

    ../../judge/src/runner.cpp
    ../../judge/src/sandbox.cpp
    ../../judge/src/judge_server.cpp
)

target_include_directories(oj-judge-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../judge/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../judge/src
    ${GTEST_INCLUDE_DIR}
)

target_link_libraries(oj-judge-test PRIVATE
    ${GTEST_LIBRARY}
    ${GTEST_MAIN_LIBRARY}
    Threads::Threads
)

target_compile_options(oj-judge-test PRIVATE -Wall -Wextra -Wpedantic -pthread)
