cmake_minimum_required(VERSION 3.16)
project(oj-backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC OFF)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

set(MYSQL_INCLUDE_DIR /usr/include/mysql)
set(MYSQL_LIBRARY mysqlclient)

add_executable(oj-backend
    src/main.cpp
    src/server.cpp
    src/router.cpp
    src/utils/logger.cpp
    src/db/connection.cpp
    src/models/user.cpp
    src/models/problem.cpp
    src/middleware/session.cpp
    src/controllers/auth_controller.cpp
    src/controllers/problem_controller.cpp
    src/models/submission.cpp
    src/controllers/submission_controller.cpp
    src/judge_client.cpp
)

target_include_directories(oj-backend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${MYSQL_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(oj-backend PRIVATE
    ${MYSQL_LIBRARY}
    Threads::Threads
    OpenSSL::Crypto
)

target_compile_options(oj-backend PRIVATE -Wall -Wextra -Wpedantic)
