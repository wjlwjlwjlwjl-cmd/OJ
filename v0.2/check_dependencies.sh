#!/bin/bash
# check_dependencies.sh
# 检查OJ平台项目依赖是否安装完整

# 定义颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

check_and_print() {
    local cmd=$1
    local name=$2

    if command -v $cmd &> /dev/null; then
        printf "${GREEN}%s 已安装${NC}\n" "$name"
    else
        printf "${RED}%s 未安装${NC}\n" "$name"
    fi
}

# 检查C++编译器
check_and_print "g++" "C++编译器(g++)"

# 检查构建工具
check_and_print "make" "make工具"
check_and_print "cmake" "cmake工具"
check_and_print "pkg-config" "pkg-config工具"

# 检查MySQL
check_and_print "mysql" "MySQL客户端"
check_and_print "mysqld" "MySQL服务器"

# 检查Java
check_and_print "java" "Java运行环境"
check_and_print "javac" "Java编译器"

# 检查Python
check_and_print "python3" "Python3"

# 检查头文件
if [ -f "/usr/include/nlohmann/json.hpp" ]; then
    printf "${GREEN}JSON库头文件 已安装${NC}\n"
else
    printf "${RED}JSON库头文件 未安装${NC}\n"
fi

if [ -f "/usr/include/mysql/mysql.h" ]; then
    printf "${GREEN}MySQL客户端库头文件 已安装${NC}\n"
else
    printf "${RED}MySQL客户端库头文件 未安装${NC}\n"
fi

if [ -f "/usr/include/seccomp.h" ]; then
    printf "${GREEN}Seccomp库头文件 已安装${NC}\n"
else
    printf "${RED}Seccomp库头文件 未安装${NC}\n"
fi

# 检查third_party目录下的httplib.h
if [ -f "./third_party/httplib.h" ]; then
    printf "${GREEN}cpp-httplib库 已下载${NC}\n"
else
    printf "${RED}cpp-httplib库 未下载${NC}\n"
fi

# 检查MySQL服务状态
if systemctl is-active --quiet mysql; then
    printf "${GREEN}MySQL服务 正在运行${NC}\n"
else
    printf "${RED}MySQL服务 未运行${NC}\n"
fi