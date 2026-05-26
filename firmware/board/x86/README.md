# x86 开发板说明

本地开发与 CI 使用 **Ubuntu 22.04+**。

## 依赖

```bash
sudo apt-get install -y \
  build-essential cmake pkg-config \
  libavformat-dev libavcodec-dev libavutil-dev libswscale-dev
```

## 构建

```bash
./scripts/build-x86.sh
./build/x86/bin/edger-rec-demo --help
```

## 示例配置

复制 `config/example.json` 到 `/etc/edger-rec/config.json`（或 `--config` 指定）。

## Week 1 联调

用公开 RTSP 测试流或本地 mediamtx：

```bash
# 示例：mediamtx 发布 test 流后
# rtsp://127.0.0.1:8554/live
```
