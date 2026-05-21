# 测试计划：TODO #1 C++ 后端框架

## 测试范围

`backend/src/` 中已完成的基础设施模块：
- Logger（日志）
- DbConnection（数据库连接）
- Server（HTTP 服务封装）
- Router（路由注册）
- Config（配置加载）
- SignalHandler（信号处理）

---

## 1. Logger 单元测试

### 1.1 日志级别过滤
| # | 场景 | 预期 |
|---|------|------|
| 1 | 设置级别为 INFO，调用 debug() | 不输出任何内容 |
| 2 | 设置级别为 DEBUG，调用 debug() | 正常输出 |
| 3 | 设置级别为 WARN，调用 info() | 不输出任何内容 |
| 4 | error() 输出到 stderr，其余到 stdout | 验证输出流 |

### 1.2 线程安全
| # | 场景 | 预期 |
|---|------|------|
| 5 | 10 个线程并发写日志 | 行不被打乱，无崩溃 |

### 1.3 格式校验
| # | 场景 | 预期 |
|---|------|------|
| 6 | 日志格式为 `[LEVEL] YYYY-MM-DD HH:MM:SS 消息` | 正则匹配 |

---

## 2. DbConnection 单元测试

### 2.1 连接管理
| # | 场景 | 预期 |
|---|------|------|
| 1 | 使用正确的 config 调用 init() | 返回 true，isConnected() 为 true |
| 2 | 使用错误密码调用 init() | 返回 false，isConnected() 为 false |
| 3 | 连接后调用 close() | isConnected() 为 false |
| 4 | 重复调用 init() 两次 | 先 close 再重新连接，无内存泄漏 |

### 2.2 查询执行
| # | 场景 | 预期 |
|---|------|------|
| 5 | execute("SELECT 1") | 返回 true |
| 6 | execute("SELECT * FROM nonexistent_table") | 返回 false |
| 7 | query("SELECT 1 as a") | 返回 MYSQL_RES，读取到行 a=1 |
| 8 | query("SELECT * FROM nonexistent_table") | 返回 nullptr |

### 2.3 SQL 注入防护
| # | 场景 | 预期 |
|---|------|------|
| 9 | escape("O'Brien") | 返回 "O\\'Brien" |
| 10 | escape("foo; DROP TABLE users") | 正确转义，不导致注入 |

---

## 3. Server 单元测试

### 3.1 启动与停止
| # | 场景 | 预期 |
|---|------|------|
| 1 | listen("127.0.0.1", 8080) | 返回 true，端口被监听 |
| 2 | 启动后调用 stop() | 端口释放，无 hang |
| 3 | 监听已被占用的端口 | 返回 false |

### 3.2 CORS 头
| # | 场景 | 预期 |
|---|------|------|
| 4 | 发送 OPTIONS 预检请求 | 返回 200，含 Access-Control-* 头 |
| 5 | 发送 GET 请求 | 响应含 Access-Control-Allow-Origin: * |

### 3.3 异常处理
| # | 场景 | 预期 |
|---|------|------|
| 6 | 路由处理器抛出异常 | 返回 500，JSON 错误体 |

---

## 4. Router 单元测试

### 4.1 路由注册
| # | 场景 | 预期 |
|---|------|------|
| 1 | GET /api/health | 返回 {"status":"ok"} |
| 2 | GET /api/nonexistent | 返回 404 |

### 4.2 日志输出
| # | 场景 | 预期 |
|---|------|------|
| 3 | setupRoutes() 调用后 | 日志打印所有预注册路由 |

---

## 5. Config 加载测试

| # | 场景 | 预期 |
|---|------|------|
| 1 | 从正确路径加载 config.json | 解析成功，字段齐全 |
| 2 | 设置 CONFIG_PATH 环境变量 | 优先使用该路径 |
| 3 | 所有候选路径都不存在 | 程序退出并报错 |
| 4 | JSON 格式错误 | 抛出 parse 异常 |

---

## 6. 集成测试（端到端）

| # | 场景 | 预期 |
|---|------|------|
| 1 | 启动服务 → GET /api/health | 返回 200，body 含 ok |
| 2 | 启动后 kill -TERM | 日志打印 shutdown，DB 正常关闭 |
| 3 | 启动后 kill -INT (Ctrl+C) | 同上 |

---

## 测试工具建议
- C++ 测试框架：Google Test (gtest) 或 Catch2
- HTTP 客户端测试：使用 curl 或 httplib 的 Client 类
- DB 测试：独立的测试数据库 `oj_test`，每次运行前重建
