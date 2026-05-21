#!/bin/bash
# dependence_install.sh
# OJ平台项目依赖安装脚本
# 适用于 Ubuntu 26.04 LTS

# 更新包列表
apt-get update

# 安装C++编译工具链
apt-get install -y build-essential cmake pkg-config

# 安装MySQL服务器和客户端
apt-get install -y mysql-server mysql-client libmysqlclient-dev

# 安装JSON库
apt-get install -y nlohmann-json3-dev

# 安装安全相关库
apt-get install -y libseccomp-dev

# 安装其他语言支持
apt-get install -y openjdk-11-jdk python3 python3-pip

# 创建项目依赖目录并下载cpp-httplib
mkdir -p ./third_party
if [ ! -f ./third_party/httplib.h ]; then
    cd third_party
    wget https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
    cd ..
fi

# 启动并启用MySQL服务
systemctl start mysql
systemctl enable mysql

echo "OJ平台依赖安装完成！"