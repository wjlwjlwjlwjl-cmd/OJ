cmake_minimum_required(VERSION 3.16)
project(oj-auth-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

set(MYSQL_INCLUDE_DIR /usr/include/mysql)
set(MYSQL_LIBRARY mysqlclient)

set(GTEST_INCLUDE_DIR /usr/include)
set(GTEST_LIBRARY /usr/lib/x86_64-linux-gnu/libgtest.a)
set(GTEST_MAIN_LIBRARY /usr/lib/x86_64-linux-gnu/libgtest_main.a)

add_executable(oj-auth-test
    test_user_model.cpp
    test_session.cpp
    test_auth_controller.cpp
    test_main.cpp

    ../../backend/src/models/user.cpp
    ../../backend/src/middleware/session.cpp
    ../../backend/src/controllers/auth_controller.cpp
    ../../backend/src/utils/logger.cpp
    ../../backend/src/db/connection.cpp
    ../../backend/src/server.cpp
    ../../backend/src/router.cpp
)

target_include_directories(oj-auth-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../backend/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../backend/src
    ${GTEST_INCLUDE_DIR}
    ${MYSQL_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(oj-auth-test PRIVATE
    ${GTEST_LIBRARY}
    ${GTEST_MAIN_LIBRARY}
    ${MYSQL_LIBRARY}
    Threads::Threads
    OpenSSL::Crypto
)

target_compile_options(oj-auth-test PRIVATE -Wall -Wextra -Wpedantic -pthread)
