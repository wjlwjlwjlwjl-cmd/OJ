# 测试计划：TODO #2 用户注册/登录 + Session 中间件

## 待测模块
- `backend/src/controllers/auth_controller.h/.cpp`
- `backend/src/middleware/session.h/.cpp`
- `backend/src/models/user.h/.cpp`

---

## 1. User Model 单元测试

| # | 场景 | 预期 |
|---|------|------|
| 1 | 创建用户，用户名 3-64 字符 | 成功 |
| 2 | 创建用户，用户名中包含特殊字符 | 失败或拒绝 |
| 3 | 密码 bcrypt/sha256 哈希存储 | 原始密码不可逆推 |
| 4 | 查询不存在的用户 | 返回 nullptr/空 |
| 5 | 创建重复用户名 | 返回错误（UNIQUE 约束） |

## 2. Auth Controller 测试

### 2.1 注册
| # | 场景 | 预期 |
|---|------|------|
| 1 | POST /api/auth/register 合法参数 | 201，返回用户信息（不含密码） |
| 2 | POST /api/auth/register 缺少字段 | 400，错误提示 |
| 3 | POST /api/auth/register 用户名已存在 | 409，错误提示 |

### 2.2 登录
| # | 场景 | 预期 |
|---|------|------|
| 4 | POST /api/auth/login 正确凭据 | 200，设置 Cookie，返回用户信息 |
| 5 | POST /api/auth/login 错误密码 | 401，错误提示 |
| 6 | POST /api/auth/login 不存在用户 | 401，错误提示 |

### 2.3 登出
| # | 场景 | 预期 |
|---|------|------|
| 7 | POST /api/auth/logout 携带有效 Cookie | 200，清除 Session |

### 2.4 当前用户
| # | 场景 | 预期 |
|---|------|------|
| 8 | GET /api/auth/me 携带有效 Cookie | 200，返回当前用户信息 |
| 9 | GET /api/auth/me 无 Cookie | 401 |

## 3. Session 中间件测试

| # | 场景 | 预期 |
|---|------|------|
| 1 | 登录后返回 Set-Cookie 头 | Cookie 含 session_id |
| 2 | 携带有效 session_id 请求受保护路由 | 中间件通过，req 附加 user 信息 |
| 3 | 携带过期 session_id | 返回 401 |
| 4 | 伪造/篡改 session_id | 返回 401 |
| 5 | 登出后 session_id 失效 | 再次使用返回 401 |

## 4. 密码安全

| # | 场景 | 预期 |
|---|------|------|
| 1 | 密码最小长度限制（如 6 位） | 过短时拒绝注册 |
| 2 | 使用 bcrypt 或 SHA-256+盐 存储 | DB 中密码字段不可读 |
