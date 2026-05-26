# AI 模型与数据管线

> 婴儿看护专用 AI 设计 · 边缘部署目标：RV1106 NPU (INT8)

## 1. 模型清单

| 模型 | 输入 | 输出 | 框架 | 推理间隔 |
|------|------|------|------|----------|
| baby_detector | 640×640 RGB | bbox + class | YOLOv5n-RKNN | 200ms |
| face_cover | 224×224 ROI | covered/normal | MobileNetV3 | 200ms |
| pose_lite | 192×192 ROI | 17 keypoints | PP-PicoDet | 500ms |
| cry_classifier | 1s audio @16kHz | cry/noise/silence | CRNN | 1s |
| motion_scorer | 帧差 mask | motion score | 传统 CV | 100ms |

## 2. 各模型说明

### 2.1 baby_detector

- **目的**：定位婴儿/人体框，供后续 ROI 分析
- **训练数据**：COCO person + 自采婴儿床数据 5000+ 张
- **指标**：mAP@0.5 ≥ 0.85（婴儿床场景）
- **RKNN 转换**：rknn-toolkit2，INT8 量化（校准集 200 张）

### 2.2 face_cover（遮脸检测，最高优先级）

- **目的**：检测口鼻是否被织物遮挡
- **输入**：从 baby_detector ROI 裁剪并放大
- **训练数据**：
  - 正样本：婴儿正常露脸
  - 负样本：被子遮脸、侧睡压脸、枕头遮挡
  - 目标 3000+ 张，含不同光照/红外
- **指标**：Recall ≥ 95%，Precision ≥ 90%
- **误报控制**：连续 3 帧（600ms）才触发

### 2.3 cry_classifier（哭声检测）

- **目的**：区分婴儿哭声 vs 环境噪声 vs 静音
- **输入**：16kHz mono，1 秒滑动窗口，0.5 秒 hop
- **训练数据**：
  - 开源：ESC-50、AudioSet baby cry 子集
  - 自采：家庭录音（需授权）+ 数据增强
- **指标**：F1 ≥ 0.88；夜间误报 ≤ 3 次
- **注意**：不对「哭声原因」分类（饥饿/尿布）— 避免过度承诺

### 2.4 运动/睡眠启发式

非深度学习为主，结合检测框：

```
motion_score = α × bbox位移 + β × 帧差面积
sleep_state  = motion_score < 阈值 持续 5min →  asleep
wake         = motion_score > 阈值 持续 30s → awake
```

## 3. 推理调度

```
时间轴 ─────────────────────────────────────────►

Video:  |--detect--|--detect--|--detect--|--detect--|  (5fps 分析)
        |  face   |          |  face   |          |  (2.5fps ROI)
Audio:  |----cry----|----cry----|----cry----|       (1fps)

NPU 队列优先级: face_cover > baby_detector > pose_lite
```

**CPU 满载降级策略**

1. 关闭 pose_lite
2. 检测帧率 5fps → 2fps
3. 仅保留 cry + face_cover

## 4. 数据闭环

```
设备告警 ──► 用户反馈(误报/漏报) ──► 数据湖
                                        │
标注平台 ◄──────────────────────────────┘
    │
    ▼
模型重训 ──► RKNN 转换 ──► OTA 灰度发布
```

| 环节 | 工具 |
|------|------|
| 标注 | Label Studio |
| 训练 | PyTorch + Ultralytics (YOLO) |
| 转换 | rknn-toolkit2 |
| 版本 | MLflow / 简单 Git LFS |

## 5. 隐私与伦理

- 训练数据全部脱敏，不含可识别人脸身份
- 设备端推理，原始视频不上传
- 用户可选择「贡献误报样本」帮助改进（默认关闭）
- 不输出医疗诊断结论（如「呼吸暂停」需 V2 雷达 + 认证）

## 6. 开发阶段模型策略

| 阶段 | 策略 |
|------|------|
| 原型 (Pi) | PyTorch + ONNX Runtime，CPU 推理 |
| Alpha | RKNN INT8，单模型验证 |
| Beta | 全 pipeline + 事件融合 |
| 量产 | 量化校准 + 72h 压力测试 |

## 7. 基准测试场景

| 场景 | 通过标准 |
|------|----------|
| 正常睡觉 | 零遮脸告警 |
| 踢被子露脸 | 不告警 |
| 被子遮口鼻 | 3s 内告警 |
| 播放电视声 | 零哭声明警 |
| 真实哭声 | 2s 内告警 |
| 红外夜视 | 与白天指标偏差 < 5% |
