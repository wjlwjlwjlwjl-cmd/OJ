# OJ 项目规格说明书 (SPEC)

## 1. 项目概述
仿 LeetCode 的在线评测系统，供个人/小团队内部刷题使用。

## 2. 技术栈
| 层级 | 技术 | 备注 |
|------|------|------|
| 后端 | C++ + cpp-httplib | 硬性要求 |
| 前端 | 原生 HTML + CSS + JS | 硬性要求 |
| 数据库 | MySQL | 已选定 |
| 评测服务 | 独立 HTTP 服务 | 与后端 REST 通信 |
| 认证 | Session-Cookie | 用户名+密码 |
| 部署 | 单机部署 | |

## 3. 功能需求

### 3.1 用户系统
- 注册 / 登录（用户名 + 密码）
- Session-Cookie 鉴权
- 无需邮箱验证、找回密码等复杂功能

### 3.2 题目管理
- 仅管理员可录入/编辑/删除题目（管理员后台）
- 题目字段：标题、描述、输入输出示例、测试用例（BLOB 存 DB）、难度标签、时间/内存限制

### 3.3 前端页面
| 页面 | 功能 |
|------|------|
| 首页 / 题目列表 | 展示所有题目，支持搜索/筛选 |
| 题目详情页 | 题目描述 + 代码编辑器 + 提交按钮 |
| 提交记录页 | 查看历史提交及评测结果 |
| 登录 / 注册页 | 用户认证 |
| 管理员后台 | 题目 CRUD |

### 3.4 评测系统
- 初始仅支持 C++ 提交
- 仅判断正确性（比对输出）
- 独立评测服务，通过 HTTP REST 接收评测任务
- **异步轮询**：提交后立即返回 submission_id，前端轮询结果
- 测试用例存储于数据库 BLOB 字段

### 3.5 异常处理
- 超时限制：timeout kill 子进程
- 内存限制：rlimit 限制
- 评测失败返回相应错误信息

## 4. 架构示意

```
Browser (HTML/CSS/JS)
    ↕ HTTP (Session-Cookie)
C++ Backend (cpp-httplib)
    ↕ HTTP REST
Judge Service (独立进程)
    ↓
执行用户代码 (fork + execve)
    ↓
比对输出 → 写入结果到 DB
```

**请求流程**：
1. 用户在题目页提交代码 → 前端 POST `/api/submit`
2. C++ 后端记录提交到 DB，返回 `submission_id`
3. 后端调用评测服务 HTTP POST `/judge` 传递代码+用例
4. 前端轮询 `GET /api/submission/{id}` 获取结果
5. 评测服务执行代码，比对输出，写回结果

## 5. 验收标准
- [x] 用户注册/登录/登出正常
- [x] 管理员可创建/编辑/删除题目
- [x] 用户可浏览题目列表和题目详情
- [x] 用户可提交 C++ 代码
- [x] 提交后正确/错误/编译错误/超时等状态正确返回
- [x] 提交记录页可查看历史提交
- [x] 基本防护：超时和内存溢出被正确拦截

## 6. TODO 清单
- [x] 1. 搭建 C++ 后端框架（cpp-httplib + MySQL 连接）
- [x] 2. 实现用户注册/登录 + Session 中间件
- [x] 3. 实现题目 CRUD API 和管理员后台页面
- [x] 4. 实现题目列表页、题目详情页（含代码编辑器）
- [x] 5. 实现评测服务（独立进程，fork + execve 编译运行 + 输出比对）+ 单元测试（28 tests）
- [x] 6. 实现提交 API + 异步轮询机制 + 单元测试（21 tests）
- [x] 7. 实现提交记录页
- [ ] 8. 集成测试和基本安全防护（timeout + rlimit）
- [x] 9. 单机部署脚本

## 7. 风险与权衡
| 权衡 | 选择 | 理由 |
|------|------|------|
| 同步 vs 异步 | 异步轮询 | 避免 HTTP 长阻塞 |
| DB 存用例 | BLOB | 单机小规模，简单够用 |
| 评测安全 | 基本防护 | 内部使用，暂时不上容器化 |
| 单机部署 | 单体+独立评测进程 | 简化运维，后续可容器化 |

## 8. 数据库管理

### 8.1 连接数据库

使用 `oj` 用户登录 MySQL（密码为 `oj_password`）：

```bash
mysql -u oj -p -D oj
# 回车后输入密码: oj_password
```

或直接在命令中指定密码：

```bash
mysql -u oj -poj_password oj
```

### 8.2 配置文件对应关系

`config/config.json` 中的数据库配置：

```json
{
  "db": {
    "host": "127.0.0.1",
    "port": 3306,
    "user": "oj",
    "password": "oj_password",
    "database": "oj"
  }
}
```

### 8.3 常用查询

```sql
-- 查看所有用户
SELECT id, username, role FROM users;

-- 查看所有题目
SELECT id, title, difficulty FROM problems;

-- 查看提交记录
SELECT id, user_id, problem_id, status, score FROM submissions;

-- 查看某用户的所有提交
SELECT * FROM submissions WHERE user_id = 1;
```

## 9. 项目目录结构

```
oj/
├── backend/
│   ├── src/
│   │   ├── controllers/
│   │   │   ├── auth_controller.cpp
│   │   │   ├── auth_controller.h
│   │   │   ├── problem_controller.cpp
│   │   │   ├── problem_controller.h
│   │   │   ├── submission_controller.cpp
│   │   │   └── submission_controller.h
│   │   ├── middleware/
│   │   │   ├── session.cpp
│   │   │   └── session.h
│   │   ├── models/
│   │   │   ├── user.cpp
│   │   │   ├── user.h
│   │   │   ├── problem.cpp
│   │   │   ├── problem.h
│   │   │   ├── submission.cpp
│   │   │   └── submission.h
│   │   ├── db/
│   │   │   ├── connection.cpp
│   │   │   └── connection.h
│   │   ├── utils/
│   │   │   ├── logger.cpp
│   │   │   └── logger.h
│   │   ├── server.cpp
│   │   ├── server.h
│   │   ├── router.cpp
│   │   ├── router.h
│   │   └── main.cpp
│   ├── include/
│   │   └── httplib.h
│   ├── CMakeLists.txt
│   └── Makefile
├── judge/
│   ├── src/
│   │   ├── judge_server.cpp
│   │   ├── judge_server.h
│   │   ├── runner.cpp
│   │   ├── runner.h
│   │   ├── sandbox.cpp
│   │   ├── sandbox.h
│   │   └── main.cpp
│   ├── CMakeLists.txt
│   └── Makefile
├── frontend/
│   ├── index.html
│   ├── pages/
│   │   ├── login.html
│   │   ├── register.html
│   │   ├── problems.html
│   │   ├── problem-detail.html
│   │   ├── submissions.html
│   │   └── admin.html
│   ├── css/
│   │   ├── style.css
│   │   └── admin.css
│   ├── js/
│   │   ├── api.js
│   │   ├── auth.js
│   │   ├── problems.js
│   │   ├── editor.js
│   │   ├── submission.js
│   │   └── admin.js
│   └── lib/
│       ├── ace/
│       │   ├── ace.js
│       │   ├── mode-c_cpp.js
│       │   ├── theme-monokai.js
│       │   └── ...
│       └── ...
├── sql/
│   ├── init.sql
│   └── seed.sql
├── scripts/
│   ├── start.sh
│   └── init_db.sh
├── config/
│   ├── config.json
│   └── nginx.conf
├── test/
│   ├── todo-01-backend-framework/  (30 tests)
│   ├── todo-02-auth/               (42 tests)
│   ├── todo-05-judge/              (28 tests)
│   ├── todo-06-submission-api/     (21 tests)
│   │   ├── README.md
│   │   ├── CMakeLists.txt
│   │   ├── test_main.cpp
│   │   ├── test_runner.cpp
│   │   ├── test_sandbox.cpp
│   │   └── test_judge_server.cpp
│   ├── ...
│   └── build/
├── sql/
│   ├── init.sql
│   └── seed.sql
├── scripts/
│   ├── start.sh
│   └── init_db.sh
└── README.md
```
