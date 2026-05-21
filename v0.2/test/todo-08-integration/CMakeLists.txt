cmake_minimum_required(VERSION 3.16)
project(oj-integration-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

set(GTEST_INCLUDE_DIR /usr/include)
set(GTEST_LIBRARY /usr/lib/x86_64-linux-gnu/libgtest.a)
set(GTEST_MAIN_LIBRARY /usr/lib/x86_64-linux-gnu/libgtest_main.a)

add_executable(oj-integration-test
    test_main.cpp
    test_integration.cpp

    ../../judge/src/runner.cpp
    ../../judge/src/sandbox.cpp
    ../../judge/src/judge_server.cpp
)

target_include_directories(oj-integration-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../judge/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../judge/src
    ${GTEST_INCLUDE_DIR}
)

target_link_libraries(oj-integration-test PRIVATE
    ${GTEST_LIBRARY}
    ${GTEST_MAIN_LIBRARY}
    Threads::Threads
)

target_compile_options(oj-integration-test PRIVATE -Wall -Wextra -Wpedantic -pthread)
