# 测试计划：TODO #5 评测服务

## 待测模块
- `judge/src/judge_server.h/.cpp`
- `judge/src/runner.h/.cpp`
- `judge/src/sandbox.h/.cpp`
- 与后端的 HTTP REST 通信

---

## 1. Runner 单元测试（核心评测逻辑）

### 1.1 正确性判断
| # | 场景 | 预期 |
|---|------|------|
| 1 | 提交代码输出完全匹配预期 | status = accepted |
| 2 | 提交代码输出与预期不同 | status = wrong_answer |
| 3 | 编译错误（语法错误） | status = compile_error，含错误信息 |
| 4 | 多个测试用例，全部通过 | accepted |
| 5 | 多个测试用例，部分通过 | wrong_answer，score 为通过比例 |

### 1.2 超时控制
| # | 场景 | 预期 |
|---|------|------|
| 6 | 代码含死循环（while(true)） | status = time_limit，进程被 kill |
| 7 | 代码刚好在时间限制边缘 | 准确计时，不误杀 |
| 8 | time_limit = 0 | 立即超时 |

### 1.3 内存限制
| # | 场景 | 预期 |
|---|------|------|
| 9 | 代码申请大量内存（> limit） | status = memory_limit |
| 10 | 代码使用边界内存 | 正常运行，不误杀 |

### 1.4 运行时错误
| # | 场景 | 预期 |
|---|------|------|
| 11 | 除零操作 | status = runtime_error |
| 12 | 段错误（空指针解引用） | status = runtime_error |
| 13 | 返回值非零 | status = runtime_error |

### 1.5 安全隔离
| # | 场景 | 预期 |
|---|------|------|
| 14 | 代码尝试打开文件 /etc/passwd | 权限被拒绝或空结果 |
| 15 | 代码尝试执行系统命令 system() | 被禁止或无权限 |
| 16 | 代码 fork 子进程 | 被禁止或限制 |
| 17 | 代码写入大量垃圾到 stdout | stdout 截断，不 OOM |

## 2. Judge Server HTTP 测试

| # | 场景 | 预期 |
|---|------|------|
| 1 | POST /judge 合法请求 | 200，返回评判结果 |
| 2 | POST /judge 缺少代码字段 | 400 |
| 3 | POST /judge 缺少测试用例 | 400 |
| 4 | POST /judge 语言不支持 | 400 |

## 3. 后端 ↔ 评测服务集成测试

| # | 场景 | 预期 |
|---|------|------|
| 1 | 后端提交后调用评测服务 | 评测服务收到正确参数 |
| 2 | 评测服务宕机时后端处理 | 后端返回 503 或重试 |
| 3 | 评测结果写回 DB | submissions 表状态和结果正确更新 |
