# EdgeRec Alert — 8 周副业开发计划

> 假设每周 10–15 小时（工作日编码 + 周末联调）

## 总目标

**Pro v1 可演示**：RV1126/x86 上 4 路录播 + 区域入侵 + Qt 配参 + Web 回放 + 5 分钟 demo 视频。

---

## Week 1 — 媒体链路

- [ ] x86：`media-svc` RTSP 拉流（FFmpeg）
- [ ] 单路硬录/软录 MP4 分段
- [ ] `edger-core` 配置读写 JSON
- [ ] CMake 工程可编译

**产出**：`./media-svc --config config.json` 录 1 路 1 小时

---

## Week 2 — 多路录播

- [ ] 4 路并发拉流
- [ ] `record-svc` 独立进程 + 循环删除
- [ ] 录像索引（通道/日期/文件名）
- [ ] systemd unit 模板

**产出**：4 路 24h  soak test（x86）

---

## Week 3 — AI 告警

- [ ] `ai-svc` 区域入侵 v0（x86 ONNX 或 OpenCV MOG 占位）
- [ ] ROI 多边形配置
- [ ] `alert-svc` HTTP Webhook
- [ ] 事件去重（同通道 10s 内合并）

**产出**：入侵触发 Webhook 可在 curl 看到

---

## Week 4 — Qt 配置

- [ ] 通道列表（URL、启用）
- [ ] 录像保留天数 / 容量上限
- [ ] 告警区域画框（QGraphicsView）
- [ ] 写回 config.json 并通知 edger-core reload

**产出**：Qt 改参后 AI 框生效

---

## Week 5 — Web 回放

- [ ] `web-server` 列出录像片段
- [ ] 按时间范围查询 API
- [ ] 浏览器播放 MP4 或 HLS
- [ ] 简单登录（token 或 basic auth）

**产出**：局域网浏览器可回放

---

## Week 6 — 打包与稳定性

- [ ] 统一 `edger-rec.target` 启停
- [ ] 日志轮转
- [ ] 异常自动重启（systemd Restart=）
- [ ] x86 一键安装脚本

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
