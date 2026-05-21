cmake_minimum_required(VERSION 3.16)
project(oj-backend-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

set(MYSQL_INCLUDE_DIR /usr/include/mysql)
set(MYSQL_LIBRARY mysqlclient)

set(GTEST_INCLUDE_DIR /usr/include)
set(GTEST_LIBRARY /usr/lib/x86_64-linux-gnu/libgtest.a)
set(GTEST_MAIN_LIBRARY /usr/lib/x86_64-linux-gnu/libgtest_main.a)

add_executable(oj-backend-test
    test_logger.cpp
    test_db.cpp
    test_server.cpp
    test_config.cpp
    test_main.cpp

    ../../backend/src/utils/logger.cpp
    ../../backend/src/db/connection.cpp
    ../../backend/src/server.cpp
)

target_include_directories(oj-backend-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../backend/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../backend/src
    ${GTEST_INCLUDE_DIR}
    ${MYSQL_INCLUDE_DIR}
)

target_link_libraries(oj-backend-test PRIVATE
    ${GTEST_LIBRARY}
    ${GTEST_MAIN_LIBRARY}
    ${MYSQL_LIBRARY}
    Threads::Threads
)

target_compile_options(oj-backend-test PRIVATE -Wall -Wextra -Wpedantic -pthread)
