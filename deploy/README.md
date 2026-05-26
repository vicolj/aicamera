# EdgeRec Alert 部署

## x86 一键安装

```bash
# 构建并安装到 /usr/local（需 root 写 /etc、/var）
sudo ./scripts/install-x86.sh

# 编辑通道 RTSP 地址
sudo nano /etc/edger-rec/config.json

# 启停整套服务
sudo systemctl daemon-reload
sudo systemctl enable --now edger-rec.target
sudo systemctl status edger-rec.target

# 查看录播日志
journalctl -u edger-record -f
```

## 服务说明

| 单元 | 说明 |
|------|------|
| `edger-rec.target` | 统一启停 record + ai + web |
| `edger-record.service` | 多路录播，`Restart=on-failure` |
| `edger-ai.service` | 区域入侵 AI（配置禁用时空闲驻留） |
| `edger-web.service` | Web 回放（配置禁用时空闲驻留） |

## 日志

- 运行日志：`journalctl -u edger-record|edger-ai|edger-web`
- 文件日志目录：`/var/log/edger-rec/`（logrotate 每日轮转，保留 7 天）

## 目录

```
/etc/edger-rec/config.json
/var/lib/edger-rec/recordings/
/var/log/edger-rec/
/usr/local/bin/edger-*-svc
```

## 打包安装（DESTDIR）

```bash
./scripts/install-x86.sh --destdir /tmp/edger-pkg --prefix /usr/local
```
