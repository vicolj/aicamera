# EdgeRec Alert

边缘录播 + AI 告警一体机软件，面向安防集成商 / OEM 小厂。

**能力**：多路 RTSP/ONVIF 接入 · 本地循环录播 · 区域入侵 AI 告警 · Qt 本地配参 · Web 回放

**目标平台**：瑞芯微 RV1126（量产）· x86 Linux（开发与高端版）

## 文档

| 文档 | 说明 |
|------|------|
| [docs/PRD.md](docs/PRD.md) | 产品需求与版本边界 |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | 软件架构 |
| [docs/ROADMAP.md](docs/ROADMAP.md) | 8 周副业开发计划 |

## 目录

```
firmware/          # 设备端 C++ 服务
  services/        # media / record / ai / alert
  apps/            # qt-config / web-server
  board/           # rv1126 / x86 板级配置
scripts/           # 构建与开发脚本
docs/
```

## 快速开始（x86 开发）

```bash
# Linux / WSL
sudo apt-get install -y build-essential cmake pkg-config \
  libavformat-dev libavcodec-dev libavutil-dev

./scripts/build-x86.sh

# 查看配置
./build/x86/bin/edger-rec-demo --config config/example.json summary

# 探测 RTSP（需可用流，例如 mediamtx）
./build/x86/bin/edger-rec-demo --config config/example.json probe

# 录播 60 秒
./build/x86/bin/edger-rec-demo --config config/example.json record --duration 60

# AI 入侵检测 + Webhook（需可用视频源）
./build/x86/bin/edger-rec-demo --config config/example.json ai-watch --duration 10

# Qt 配置工具（需先 ./scripts/build-qt-x86.sh）
./build/x86/bin/edger-qt-config --config config/example.json

# Web 回放（需先 ./scripts/build-web-x86.sh）
./build/x86/bin/edger-web-server --config config/example.json
```

RV1126 交叉编译见 [firmware/board/rv1126/README.md](firmware/board/rv1126/README.md)。

## Docker 演示

```bash
docker compose up -d --build
# http://localhost:8080  Token: edger-demo
```

详见 [docker/README.md](docker/README.md)。

## 测试

```bash
./scripts/build-x86.sh               # 构建 + 单元测试（Week1–4）
./scripts/test-week2-integration.sh  # 4 路录播集成测试
./scripts/test-week3-integration.sh  # AI 入侵 + Webhook 集成测试
./scripts/test-week4-integration.sh  # 配置读写 + reload 集成测试
./scripts/test-week5-integration.sh  # Web 回放 API 集成测试
./scripts/test-week6-integration.sh  # 安装包 + 4 路恢复集成测试
./scripts/build-qt-x86.sh            # 可选：构建 Qt 配置工具
./scripts/build-web-x86.sh           # 可选：构建 Web 回放服务
sudo ./scripts/install-x86.sh      # x86 一键安装 + systemd
```

## 授权（规划）

| 版本 | 说明 |
|------|------|
| 社区版 | 1 路录播，评估用 |
| Pro | 4 路录播 + 2 路 AI，¥1,980/年/台 |
| 试点包 | 首客户远程部署，¥9,800 |

## License

Proprietary — 社区版组件文件头标注许可范围。
