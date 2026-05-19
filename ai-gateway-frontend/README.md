# AI 网关系统 - 前端

Vue3 + Element Plus 管理后台，配套 ai-gateway-backend 使用。

## 功能

- 用户注册 / 登录
- AI 对话控制台（支持 MiniMax / DeepSeek / OpenAI）
- 管理后台：用户管理、模型配置、套餐管理、用量统计、设备管理、审计日志、系统配置

## 运行

```bash
npm install
npm run dev
# 访问 http://localhost:3000
```

## 依赖

- Vue 3 (Composition API)
- Vite 5
- Element Plus
- Pinia (状态管理)
- Vue Router 4
- Axios
- ECharts

## 配置

Vite 代理已配置将 /api 请求转发到 http://localhost:8080。
修改 vite.config.js 中 server.proxy.target 可更改后端地址。

## 页面说明

| 路径 | 页面 | 权限 |
|------|------|------|
| /login | 登录 | 公开 |
| /register | 注册 | 公开 |
| /dashboard | 数据大盘 | 登录 |
| /chat | AI对话 | 登录 |
| /users | 用户管理 | 管理员 |
| /models | 模型配置 | 管理员 |
| /packages | 套餐管理 | 管理员 |
| /usage | 用量统计 | 登录 |
| /devices | 设备管理 | 登录 |
| /audit | 审计日志 | 管理员 |
| /config | 系统配置 | 管理员 |
| /profile | 个人设置 | 登录 |