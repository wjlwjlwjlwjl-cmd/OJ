# 测试计划：TODO #6 提交 API + 异步轮询

## 待测模块
- `backend/src/controllers/submission_controller.h/.cpp`
- `backend/src/models/submission.h/.cpp`
- 异步轮询机制

---

## 1. Submission Model 单元测试

| # | 场景 | 预期 |
|---|------|------|
| 1 | 创建提交记录 | DB 写入，status = pending |
| 2 | 更新提交状态和结果 | DB 正确更新 |
| 3 | 按 user_id 查询提交历史 | 返回该用户所有提交（分页） |
| 4 | 按 problem_id 查询提交历史 | 返回该题目的所有提交 |
| 5 | 查询不存在的 submission_id | 返回 404 |

## 2. Submission Controller 测试

### 2.1 提交代码
| # | 场景 | 预期 |
|---|------|------|
| 1 | POST /api/submissions 合法请求 | 202，返回 submission_id |
| 2 | POST /api/submissions 未登录 | 401 |
| 3 | POST /api/submissions 题目 ID 不存在 | 404 |
| 4 | POST /api/submissions 代码为空 | 400 |

### 2.2 异步轮询
| # | 场景 | 预期 |
|---|------|------|
| 5 | GET /api/submissions/:id 评测中 | 200，status = pending/judging |
| 6 | GET /api/submissions/:id 已完成 | 200，status = final，含结果 |
| 7 | 轮询间隔合理（如每秒一次） | 服务器不过载 |

### 2.3 提交列表
| # | 场景 | 预期 |
|---|------|------|
| 8 | GET /api/submissions?problem_id=1 | 返回该题目的提交列表 |
| 9 | GET /api/submissions | 返回当前用户的提交列表 |
| 10 | 列表分页 | 返回 limit 条，含 total |
