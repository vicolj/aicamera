# AI 模型与数据管线

> v0.2 · 对齐海马爸比 AI 能力 · 部署：君正 MAGIK (T31/T41) / Core ML (iPhone 原型)

## 1. 模型清单（对齐竞品功能）

| 模型 | 功能 | 对标海马爸比 | 部署 |
|------|------|--------------|------|
| baby_detector | 婴儿/人形检测 | 人脸+人形精确检测 | NPU |
| face_cover | 遮脸/蒙头 | AI 遮脸提醒 | NPU |
| **prone_pose** | 趴睡/侧角>70° | 危险睡眠行为 | NPU |
| **breath_sleep** | 浅睡/深睡/呼吸感知 | 呼吸感知算法 | NPU+时序 |
| cry_classifier | 哭声识别 ≥95% | 哭声检测 | CPU/DSP |
| zone_guard | 虚拟围栏 | AI 虚拟围栏 | CPU |
| motion_track | 人形追踪框 | Pro 云台追踪 | NPU |
| highlight_picker | 精彩瞬间选帧 | 时光相册 | CPU |

---

## 2. 核心模型说明

### 2.1 face_cover（遮脸）

- 连续 **3 帧**（~600ms）触发，对齐竞品响应
- 红外夜视样本 ≥ 40% 训练集
- 指标：Recall ≥ **95%**

### 2.2 prone_pose（趴睡 — v0.2 新增 P0）

```
输入：baby_detector ROI
输出：
  - prone: 趴睡概率
  - roll_angle: 躯干侧角（度）
规则：
  IF prone > 0.8 OR roll_angle > 70° 持续 3s → EVENT_PRONE
```

- 对标：海马爸比「侧身角度大于 70 度自动推送预警」
- 数据：趴睡/侧睡/仰睡各 2000+ 张

### 2.3 breath_sleep（呼吸感知 — v0.2 新增 P0）

**非毫米波、非医疗级**，与海马爸比同款视觉路线：

```
输入：胸口 ROI 视频序列 (15s @ 5fps)
特征：光流微动 + 周期性 FFT (0.2–0.5Hz)
输出：
  - breath_rate: 估算呼吸频率
  - sleep_stage: awake / light / deep
```

| 阶段 | 判定 |
|------|------|
| 浅睡 | 微动低 + 呼吸规律 |
| 深睡 | 微动极低 + 呼吸稳定 |
| 醒 | 大幅运动或睁眼（可选 eye 子模型） |

- 生成 **睡眠报告**：总时长、浅/深睡占比、醒次
- 指标：与人工记录偏差 < 15min/天

### 2.4 cry_classifier + 智能安抚

```
IF cry_confidence > 0.75 持续 2s:
  1. EVENT_CRY → 推送
  2. 触发 soothing-svc 播放预设曲目（本地 MP3）
  3. 家长 App 可远程 stop
```

- 准确率目标 ≥ **95%**（对齐竞品宣传口径）
- 不对哭声原因做 NLP 分类

### 2.5 motion_track（Pro 云台）

```
检测框中心 (cx, cy) → PID → 步进电机 pan/tilt
目标：婴儿始终在画面中心 ±15%
```

---

## 3. 三看护模式 · 模型组合

| 模式 | 启用模型 | 关闭 |
|------|----------|------|
| **睡眠看护** | face_cover, prone_pose, breath_sleep, cry | zone_guard |
| **客厅看护** | baby_detector, zone_guard, motion | breath_sleep |
| **分床看护** | breath_sleep, cry, baby_detector | zone_guard |

---

## 4. 推理调度（T41）

```
Video 5fps:  detect → face_cover / prone (交替)
Video 1fps:  breath_sleep (15s 窗口滑动)
Audio 1fps:  cry_classifier
Pro 5fps:    motion_track → PTZ PID
```

**降级顺序**：highlight → breath → prone → 降帧率

---

## 5. 训练与部署路径

| 阶段 | 平台 | 格式 |
|------|------|------|
| 训练 | PyTorch | .pt |
| iPhone 验证 | Core ML | .mlmodel |
| 量产 | 君正 MAGIK | .bin |

**iPhone 12 与 T41 共用同一套训练权重结构**，仅转换工具不同 — 加速「尽量像海马爸比」的功能对齐。

---

## 6. 隐私

- 训练数据脱敏；**设备端推理，永不上传原始 AV**
- 用户反馈误报：默认仅上传 **事件类型+置信度统计**（可选 opt-in），不上传视频

---

## 7. 基准测试（对齐竞品场景）

| 场景 | 标准 |
|------|------|
| 蒙头遮脸 | 3s 内告警 |
| 趴睡/侧角>70° | 3s 内告警 |
| 正常仰睡 | 零告警 |
| 电视声 | 零哭声明警 |
| 真实哭声 | 2s 内告警 + 安抚播放 |
| 越界 | 2s 内告警 |
| 睡眠报告 | 与人工偏差 < 15min |
| 940nm 夜视 | 指标下降 < 5% |
| **抓包** | 零视频 bytes 到公网 IP |
