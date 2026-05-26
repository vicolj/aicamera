# Web 回放服务（Week 5）

局域网 Web 回放录像片段。

## 能力

- `GET /api/recordings` — 列出索引中的录像（支持 `channel_id` / `date` / `from` / `to`）
- `GET /api/media?rel=...` — MP4 流媒体（支持 Range）
- `POST /api/login` — Token 登录，设置 HttpOnly Cookie
- `/` — 静态回放页面

## 构建

```bash
./scripts/build-web-x86.sh
./build/x86/bin/edger-web-server --config config/example.json
```

浏览器访问 `http://<设备IP>:8080/`，使用 `config.web.auth_token` 登录。

## 配置

```json
"web": {
  "enabled": true,
  "listen_host": "0.0.0.0",
  "port": 8080,
  "auth_token": "edger-demo"
}
```

`auth_token` 为空时不启用鉴权（仅开发环境）。
