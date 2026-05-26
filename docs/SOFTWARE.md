# 软件架构设计

> 对应 PRD v0.1 · 原则：**边缘优先、事件驱动、隐私默认本地**

## 1. 系统分层

```
┌─────────────────────────────────────────────────────────────┐
│                      移动端 App (Flutter)                     │
│   实时预览 │ 告警 │ 时间线 │ 配网 │ 安全区 │ 家庭成员          │
└───────────────────────────┬─────────────────────────────────┘
                            │ HTTPS / WebRTC / MQTT
┌───────────────────────────▼─────────────────────────────────┐
│                      云服务 (可选)                            │
│   认证 │ 设备注册 │ 推送(FCM/APNs) │ 信令 │ 事件元数据         │
│   对象存储(OSS) ← 仅事件片段，非全量流                          │
└───────────────────────────┬─────────────────────────────────┘
                            │ MQTT over TLS
┌───────────────────────────▼─────────────────────────────────┐
│                    设备端 (Linux / Buildroot)                 │
│ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐            │
│ │  media-svc  │ │   ai-svc    │ │  cloud-svc  │            │
│ │ RTSP/WebRTC │ │ 推理+事件   │ │ MQTT/OTA    │            │
│ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘            │
│        └───────────────┼───────────────┘                    │
│                   device-core (配置/状态机/看门狗)             │
│ ┌─────────────────────────────────────────────────────────┐ │
│ │ HAL: camera │ mic │ speaker │ gpio │ wifi │ sd │ ble   │ │
│ └─────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## 2. 技术栈选型

| 层级 | 技术 | 理由 |
|------|------|------|
| 设备 OS | Buildroot + Linux 5.10 | RV1106 官方支持，IPC 成熟 |
| 设备语言 | C++17（核心）+ Python（AI 原型） | 性能 + 迭代速度 |
| 视频 | Rockchip MPP + RTSP/WebRTC | 硬件编码低延迟 |
| AI 推理 | RKNN Runtime | RV1106 NPU 原生 |
| 音频 | ALSA + WebRTC AEC | 标准栈 |
| 消息 | Mosquitto client / MQTT | IoT 轻量 |
| App | **Flutter 3** | iOS/Android 一套代码 |
| 后端 | Go + Gin | 高并发信令/API |
| DB | PostgreSQL + Redis | 关系数据 + 会话 |
| 存储 | 阿里云 OSS / MinIO | 事件片段 |
| 推送 | APNs + 厂商通道 | 国内 Android 到达率 |

## 3. 设备端服务设计

### 3.1 media-svc（媒体服务）

**职责**：采集、编码、分发、本地录像

```
Camera ISP → VI → VENC(H.265) ──┬──► RTSP (LAN)
                                ├──► WebRTC (P2P/中继)
                                └──► SD 循环录像 (mp4 segment)
```

| 参数 | 值 |
|------|-----|
| 主码流 | 2560×1440 @ 15fps, 2Mbps |
| 子码流 | 1280×720 @ 15fps, 512Kbps |
| 录像 | 5min 分片，滚动删除 |
| 夜视 | 照度 < 5lux → IR-CUT night + IR LED |

### 3.2 ai-svc（AI 服务）

**职责**：多模型推理、事件融合、告警决策

```
Video frame (720p) ──► 检测流水线 ──► 事件引擎 ──► MQTT publish
Audio stream     ──► 哭声分类
```

**推理流水线（每 200ms 一帧，可配置）**

1. **人体/婴儿检测** — YOLOv5n / PP-YOLOE-s（RKNN）
2. **姿态/遮脸** — 关键点模型，判断口鼻遮挡
3. **运动分析** — 帧差 + 检测框位移
4. **哭声分类** — YAMNet / 自研 CNN on audio chunk (1s)

**事件融合规则**

```
IF 遮脸置信度 > 0.8 持续 3s → EVENT_FACE_COVERED (紧急)
IF 哭声置信度 > 0.75 持续 2s → EVENT_CRYING (重要)
IF 检测框离开安全多边形 → EVENT_ZONE_EXIT (重要)
IF 5min 无运动 AND 非睡眠时段 → EVENT_NO_MOTION (提示)
```

### 3.3 cloud-svc（连接服务）

| 功能 | 说明 |
|------|------|
| 配网 | BLE 传 WiFi 凭证 → 连接 MQTT broker |
| 心跳 | 30s ping，离线标记 |
| OTA | A/B 分区，签名校验 |
| 时间同步 | NTP |
| 远程配置 | 告警阈值、安全区坐标下发 |

### 3.4 device-core

- 统一配置（JSON / protobuf）
- 服务守护（systemd / supervisord）
- 看门狗喂狗
- 日志 ring buffer（不含视频内容）

## 4. 移动端 App

### 4.1 页面结构

```
TabBar
├── 首页（实时预览 + 快捷状态）
├── 时间线（事件卡片 + 片段回放）
├── 洞察（睡眠统计，V1.1）
└── 设置
    ├── 设备管理
    ├── 告警策略
    ├── 安全区域编辑
    ├── 家庭成员
    └── 隐私与存储
```

### 4.2 关键流程

**配网（BLE + WiFi）**

```
App ──BLE──► 设备：SSID + password
设备 ──WiFi──► 路由器
设备 ──MQTT──► 云：register(device_id, token)
App ◄──API── 云：绑定成功
App ◄─WebRTC─► 设备：开始预览
```

**实时预览**

- 局域网：优先 P2P WebRTC（STUN）
- 公网：TURN 中继（仅在 P2P 失败时）
- 子码流默认，双击切主码流

## 5. 云端架构

```
                    ┌──────────┐
                    │   CDN    │ (App 静态资源)
                    └──────────┘
┌────────┐    ┌─────▼──────────────────────────┐
│  App   │───►│  API Gateway (HTTPS)           │
└────────┘    │  auth │ devices │ events │ clips│
              └─────┬───────────────┬──────────┘
                    │               │
              ┌─────▼─────┐   ┌─────▼─────┐
              │ PostgreSQL│   │   Redis   │
              └───────────┘   └───────────┘
                    │
              ┌─────▼─────┐   ┌───────────┐
              │ MQTT Broker│◄──│  设备端   │
              │ (EMQX)    │   └───────────┘
              └───────────┘
                    │
              ┌─────▼─────┐
              │ OSS       │ 事件 mp4 / 缩略图
              └───────────┘
```

**API 示例**

| Method | Path | 说明 |
|--------|------|------|
| POST | /v1/auth/login | 用户登录 |
| POST | /v1/devices/bind | 扫码绑定 |
| GET | /v1/devices/{id}/events | 事件列表 |
| GET | /v1/webrtc/offer | WebRTC 信令 |
| PUT | /v1/devices/{id}/config | 远程配置 |

## 6. 数据模型（核心）

```protobuf
message DeviceEvent {
  string event_id = 1;
  string device_id = 2;
  EventType type = 3;       // CRYING, FACE_COVERED, MOTION, ...
  int64 timestamp_ms = 4;
  float confidence = 5;
  string clip_url = 6;      // 可选，10s 片段
  string thumbnail_url = 7;
}

message DeviceConfig {
  repeated Point safe_zone = 1;
  AlertPolicy alert_policy = 2;
  bool night_ir_enabled = 3;
  int32 cry_sensitivity = 4;  // 1-5
}
```

## 7. 安全与隐私

| 措施 | 实现 |
|------|------|
| 传输 | TLS 1.3（API/MQTT），DTLS（WebRTC） |
| 存储 | SD 录像 AES 可选加密；云端 SSE |
| 认证 | JWT + 设备证书（MQTT mTLS） |
| 最小上传 | 仅告警事件片段，默认 10s |
| 本地开关 | 「隐私模式」关闭云端，仅 LAN |
| 合规 | 隐私政策 + 数据删除 API |

## 8. 仓库结构（建议）

```
aicamera/
├── firmware/           # 设备端 C++ / Buildroot
│   ├── services/
│   │   ├── media-svc/
│   │   ├── ai-svc/
│   │   └── cloud-svc/
│   ├── hal/
│   └── board/          # RV1106 板级配置
├── ai/                 # 模型训练与 RKNN 转换
│   ├── models/
│   ├── datasets/
│   └── tools/
├── app/                # Flutter 移动端
├── backend/            # Go API + MQTT
├── proto/              # 公共 protobuf
├── docs/               # 设计文档
└── prototype/          # 树莓派快速验证脚本
```

## 9. 原型轨快速启动（树莓派）

```bash
# 后续实现
prototype/
├── docker-compose.yml  # MQTT + MinIO + API
├── pi-setup.sh         # 依赖安装
├── stream.py           # libcamera + RTSP
└── ai_demo.py          # 哭声/遮脸 demo
```

## 10. 性能预算（RV1106）

| 模块 | CPU | NPU | 内存 |
|------|-----|-----|------|
| H.265 双码流 | 15% | - | 80MB |
| AI 4 模型 | 10% | 60% | 120MB |
| WebRTC | 20% | - | 40MB |
| 系统 + 其他 | 15% | - | 16MB |
| **合计** | ~60% | ~60% | ~256MB |
