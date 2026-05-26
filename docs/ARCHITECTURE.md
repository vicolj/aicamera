# EdgeRec Alert — 软件架构

## 1. 进程模型

```
┌─────────────────────────────────────────────────────────┐
│                     EdgeRec Box (Linux)                  │
│                                                          │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────┐ │
│  │ media-svc│  │record-svc│  │  ai-svc  │  │alert-svc│ │
│  │ 拉流解码  │→│ MP4 写入  │  │ RKNN推理 │→│ webhook │ │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └───┬────┘ │
│       │             │             │             │      │
│       └─────────────┴──────┬──────┴─────────────┘      │
│                            │                            │
│                     edger-core (配置/授权/IPC)            │
│                            │                            │
│       ┌────────────────────┴────────────────────┐       │
│       │  qt-config (本地)  │  web-server (回放)   │       │
│       └─────────────────────────────────────────┘       │
└─────────────────────────────────────────────────────────┘
         ▲ RTSP                           ▲ HTTP
    IP Cameras                      浏览器 / 集成商平台
```

## 2. 服务职责

| 服务 | 职责 |
|------|------|
| **media-svc** | RTSP 拉流、硬解、帧分发（录播 + AI 各取所需分辨率） |
| **record-svc** | 编码写 MP4、分片、循环删除、索引 JSON |
| **ai-svc** | 区域入侵检测（RKNN/x86 ONNX） |
| **alert-svc** | 事件去重、Webhook、GPIO、可选钉钉格式 |
| **edger-core** | 统一配置、通道状态、授权校验、Unix socket IPC |
| **qt-config** | 图形配参（Qt5/Qt6） |
| **web-server** | 静态页 + 回放 API（cpp-httplib / 轻量方案） |

## 3. 技术栈

| 层 | RV1126 | x86 开发 |
|----|--------|----------|
| OS | Buildroot / 厂商 BSP | Ubuntu 22.04 |
| 媒体 | Rockchip MPP | FFmpeg |
| AI | RKNN | ONNX Runtime |
| UI | Qt 5.15+ | 同左 |
| 构建 | CMake + 交叉工具链 | CMake native |

## 4. 配置与存储

```
/etc/edger-rec/config.json      # 通道、录像、告警区
/var/lib/edger-rec/recordings/  # 录像根目录
/var/lib/edger-rec/index/       # 按通道/日期索引
/var/log/edger-rec/             # 运行日志
```

## 5. IPC

服务间采用 **Unix Domain Socket + JSON 行协议**（v1 简单可靠）：

```json
{"type":"frame","ch":0,"ts":1710000000}
{"type":"alert","ch":0,"algo":"intrusion","conf":0.92}
```

## 6. 仓库布局

```
firmware/
├── CMakeLists.txt
├── edger-core/
├── services/
│   ├── media-svc/
│   ├── record-svc/
│   ├── ai-svc/
│   └── alert-svc/
├── apps/
│   ├── qt-config/
│   └── web-server/
└── board/
    ├── rv1126/
    └── x86/
```
