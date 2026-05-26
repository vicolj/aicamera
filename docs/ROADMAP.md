# EdgeRec Alert — 8 周副业开发计划

> 假设每周 10–15 小时（工作日编码 + 周末联调）

## 总目标

**Pro v1 可演示**：RV1126/x86 上 4 路录播 + 区域入侵 + Qt 配参 + Web 回放 + 5 分钟 demo 视频。

---

## Week 1 — 媒体链路

- [x] x86：`media-svc` RTSP 探测（FFmpeg）
- [x] 单路 RTSP remux 录 MP4 分段
- [x] `edger-core` 配置读写 JSON
- [x] CMake 工程可编译

**测试**：`Week1Config.*`（随 `./scripts/build-x86.sh` 运行）

**产出**：`edger-rec-demo record` 单路录播

---

## Week 2 — 多路录播

- [x] 4 路并发拉流
- [x] `record-svc` 独立进程 + 循环删除
- [x] 录像索引（通道/日期/文件名）
- [x] systemd unit 模板

**测试**：`./scripts/test-all.sh` + `./scripts/test-week2-integration.sh`

## 测试

```bash
# 单元测试（Week1 + Week2，构建时自动跑）
./scripts/build-x86.sh

# 仅单元测试
./scripts/test-all.sh

# Week2 集成测试（4 路并发录播 + 索引）
./scripts/test-week2-integration.sh
```

**产出**：4 路短时并发录播 + 索引 + retention 单测通过

---

## Week 3 — AI 告警

- [x] `ai-svc` 区域入侵 v0（帧差 + ROI）
- [x] ROI 多边形配置
- [x] `alert-svc` HTTP Webhook
- [x] 事件去重（同通道 cooldown）

**测试**：`Week3*` 单测 + `./scripts/test-week3-integration.sh`（已通过）

**产出**：入侵触发 Webhook，curl/日志可见 JSON

---

## Week 4 — Qt 配置

- [x] 通道列表（URL、启用）
- [x] 录像保留天数 / 容量上限
- [x] 告警区域画框（QGraphicsView）
- [x] 写回 config.json 并通知 edger-core reload

**测试**：`Week4*` 单测 + `./scripts/test-week4-integration.sh`

**产出**：Qt 改参后 AI 框生效

---

## Week 5 — Web 回放

- [x] `web-server` 列出录像片段
- [x] 按时间范围查询 API
- [x] 浏览器播放 MP4 或 HLS
- [x] 简单登录（token 或 basic auth）

**测试**：`Week5*` 单测 + `./scripts/test-week5-integration.sh`

**产出**：局域网浏览器可回放

---

## Week 6 — 打包与稳定性

- [x] 统一 `edger-rec.target` 启停
- [x] 日志轮转
- [x] 异常自动重启（systemd Restart=）
- [x] x86 一键安装脚本

**测试**：`./scripts/test-week6-integration.sh`

**产出**：重启后 4 路自动恢复

---

## Week 7 — RV1126 移植

- [ ] 交叉编译 media/record
- [ ] MPP 硬编替代 FFmpeg 编码
- [ ] RKNN 入侵模型替换 x86 占位
- [ ] 板级 README + 镜像打包说明

**产出**：RV1126 演示机 2 路录播 + 1 路 AI

---

## Week 8 — 发布准备

- [ ] 5 分钟 demo 视频（接入→告警→回放）
- [ ] 社区版裁剪（1 路 + 水印说明）
- [ ] 知乎/CSDN 文章 1 篇
- [ ] 试点包 ¥9800 一页介绍（PDF/飞书）

**产出**：对外可推销的最小产品包

---

## 后续 backlog（v1 之后）

- ONVIF 发现
- 钉钉/企业微信告警模板
- GPIO 输出
- 第二 AI 算法（绊线）
- 授权文件 / 年费校验
