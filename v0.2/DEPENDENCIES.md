# OJ平台项目依赖清单

## C++相关依赖

### 1. 编译器
- GCC/G++ (用于C/C++编译)
```bash
# Ubuntu/Debian
sudo apt-get update && sudo apt-get install build-essential

# CentOS/RHEL/Fedora
sudo yum groupinstall "Development Tools" 
# 或者 Fedora 上使用
sudo dnf groupinstall "Development Tools"
```

### 2. cpp-httplib (HTTP服务器库)
这是一个头文件库，可以直接下载到项目中
```bash
# 创建第三方库目录
mkdir -p third_party
cd third_party
wget https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
```

### 3. MySQL C++连接器
```bash
# Ubuntu/Debian
sudo apt-get install libmysqlclient-dev

# CentOS/RHEL
sudo yum install mysql-devel
# 或 Fedora
sudo dnf install mysql-devel
```

### 4. JSON库 (推荐nlohmann/json，用于API数据处理)
```bash
# Ubuntu/Debian
sudo apt-get install nlohmann-json3-dev

# 或手动安装
git clone https://github.com/nlohmann/json.git
cd json
mkdir build && cd build
cmake ..
sudo cmake --install .
```

### 5. 其他可能需要的C++库
```bash
# SQLite3 (如果需要作为辅助数据库)
sudo apt-get install sqlite3 libsqlite3-dev

# Boost库 (某些高级功能可能用到)
sudo apt-get install libboost-all-dev
```

## 数据库依赖

### 1. MySQL服务器
```bash
# Ubuntu/Debian
sudo apt-get install mysql-server mysql-client

# CentOS/RHEL/Fedora
sudo yum install mysql-server mysql-community-server
# 或 Fedora
sudo dnf install mysql-server mysql-community-server
```

## 系统工具依赖

### 1. 基本构建工具
```bash
# 已包含在build-essential中，但单独列出
sudo apt-get install cmake make gcc g++

# 安装pkg-config (用于库查找)
sudo apt-get install pkg-config
```

### 2. 安全相关工具 (用于沙箱实现)
```bash
# setrlimit相关系统调用已在libc中，无需额外安装
# 但需要确保系统支持容器化或命名空间功能
# 安装seccomp库用于系统调用过滤
sudo apt-get install libseccomp-dev
```

## 前端相关 (无需安装，使用原生技术)
- HTML/CSS/JavaScript 是浏览器内置支持，无需额外安装
- 但如果需要构建工具可以考虑：
```bash
# Node.js (如果后续需要构建工具)
curl -fsSL https://deb.nodesource.com/setup_lts.x | sudo -E bash -
sudo apt-get install -y nodejs
```

## 语言编译器 (用于评测功能)

### 1. Java
```bash
# Ubuntu/Debian
sudo apt-get install openjdk-11-jdk

# 设置JAVA_HOME
export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
```

### 2. Python3
```bash
# 通常系统已预装，如果没有：
sudo apt-get install python3 python3-pip
```

## 安装脚本示例

```bash
#!/bin/bash
# install_dependencies.sh

echo "正在安装OJ平台所需依赖..."

# 更新包列表
sudo apt-get update

# 安装C++编译工具链
echo "安装C++编译工具..."
sudo apt-get install -y build-essential cmake pkg-config

# 安装MySQL客户端和服务端
echo "安装MySQL..."
sudo apt-get install -y mysql-server mysql-client libmysqlclient-dev

# 安装JSON库
echo "安装JSON库..."
sudo apt-get install -y nlohmann-json3-dev

# 安装安全相关的库
echo "安装安全相关库..."
sudo apt-get install -y libseccomp-dev

# 安装其他语言支持
echo "安装评测语言支持..."
sudo apt-get install -y openjdk-11-jdk python3

echo "依赖安装完成！"
```

## 验证安装

安装完成后，可以通过以下命令验证：

```bash
# 验证C++编译器
g++ --version

# 验证MySQL
mysql --version

# 验证Java
java -version
javac -version

# 验证Python
python3 --version

# 检查必要的头文件
ls /usr/include/mysql/
ls /usr/include/nlohmann/
```