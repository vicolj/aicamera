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
```

RV1126 交叉编译见 [firmware/board/rv1126/README.md](firmware/board/rv1126/README.md)。

## 测试

```bash
./scripts/build-x86.sh          # 构建 + 单元测试
./scripts/test-week2-integration.sh  # 4 路录播集成测试
```

## 授权（规划）

| 版本 | 说明 |
|------|------|
| 社区版 | 1 路录播，评估用 |
| Pro | 4 路录播 + 2 路 AI，¥1,980/年/台 |
| 试点包 | 首客户远程部署，¥9,800 |

## License

Proprietary — 社区版组件文件头标注许可范围。
