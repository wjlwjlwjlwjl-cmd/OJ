# 测试计划：TODO #3 题目 CRUD API + 管理员后台

## 待测模块
- `backend/src/controllers/problem_controller.h/.cpp`
- `backend/src/models/problem.h/.cpp`
- 前端 `admin.html` + `admin.js`

---

## 1. Problem Model 单元测试

| # | 场景 | 预期 |
|---|------|------|
| 1 | 创建题目（标题、描述、难度、限制） | 写入 DB，返回 ID |
| 2 | 查询题目 by ID | 返回完整题目信息 |
| 3 | 查询不存在的题目 ID | 返回 404 |
| 4 | 更新题目所有字段 | DB 中数据被更新 |
| 5 | 删除题目 | DB 中记录消失 |
| 6 | 题目列表按难度筛选 | 只返回对应难度的题目 |
| 7 | 题目列表分页 | 返回 limit 条，含 total 计数 |

## 2. Problem Controller 测试

### 2.1 管理员权限
| # | 场景 | 预期 |
|---|------|------|
| 1 | 管理员 POST /api/problems | 201，创建成功 |
| 2 | 普通用户 POST /api/problems | 403 |
| 3 | 未登录 POST /api/problems | 401 |

### 2.2 公开接口
| # | 场景 | 预期 |
|---|------|------|
| 4 | GET /api/problems | 200，返回题目列表（不含测试用例） |
| 5 | GET /api/problems?difficulty=easy | 200，只返回 easy 题目 |
| 6 | GET /api/problems/:id | 200，返回题目详情（不含测试用例） |
| 7 | GET /api/problems/:id 不存在 | 404 |

## 3. 管理员前端页面测试

| # | 场景 | 预期 |
|---|------|------|
| 1 | 管理员登录后访问 admin.html | 显示题目管理界面 |
| 2 | 点击"新建题目" | 弹出表单 |
| 3 | 填写表单并提交 | 题目出现在列表中 |
| 4 | 点击"编辑" | 表单预填已有数据 |
| 5 | 点击"删除" | 确认后题目从列表移除 |
