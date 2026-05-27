# EdgeRec Alert — 5 分钟试用演示

> 最快方式：**Docker 一键演示**（见下方第 0 节）

## 0. Docker 一键演示（推荐）

```bash
git clone https://github.com/vicolj/edger-rec.git
cd edger-rec
docker compose up -d --build
```

浏览器打开 **http://localhost:8080/**，Token：`edger-demo`

离线导入：

```bash
./scripts/docker-save.sh          # 生成 edger-rec-demo.tar
docker load -i edger-rec-demo.tar
docker run -d -p 8080:8080 -p 18080:18080 -v edger-data:/data edger-rec:demo
```

详见 [docker/README.md](../docker/README.md)

---

> 本地编译环境：Ubuntu 22.04 / WSL2，已安装 `ffmpeg`

## 1. 获取代码

```bash
git clone https://github.com/vicolj/edger-rec.git
cd edger-rec
```

## 2. 安装依赖并构建

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config \
  libavformat-dev libavcodec-dev libavutil-dev libswscale-dev ffmpeg python3 curl

sed -i 's/\r$//' scripts/*.sh   # Windows 克隆时建议执行
chmod +x scripts/*.sh

./scripts/build-x86.sh          # 构建 + 19 项单元测试
./scripts/build-web-x86.sh      # Web 回放服务（可选）
```

## 3. 准备测试视频源（无摄像头时）

用本地 MP4 模拟 RTSP 输入：

```bash
mkdir -p recordings-demo
ffmpeg -y -f lavfi -i testsrc=size=640x480:rate=10 -t 60 \
  -c:v libx264 -pix_fmt yuv420p recordings-demo/test.mp4
```

编辑 `config/example.json`，把 `rtsp_url` 改成 MP4 绝对路径，例如：

```json
"rtsp_url": "/home/you/edger-rec/recordings-demo/test.mp4"
```

## 4. 演示命令

```bash
# 查看配置
./build/x86/bin/edger-rec-demo --config config/example.json summary

# 单路录播 30 秒
./build/x86/bin/edger-rec-demo --config config/example.json record --duration 30

# 4 路并发录播（需 test-4ch.json 且每路有可用源）
./build/x86/bin/edger-rec-demo --config config/test-4ch.json record-all --duration 30

# AI 入侵 + Webhook（另开终端跑 webhook）
python3 scripts/webhook_server.py 18080 /tmp/webhook.log &
./build/x86/bin/edger-rec-demo --config config/example.json ai-watch --duration 10
cat /tmp/webhook.log

# Web 回放
./build/x86/bin/edger-web-server --config config/example.json
# 浏览器打开 http://localhost:8080/  Token: edger-demo
```

## 5. 生产式安装（Linux 设备）

```bash
sudo ./scripts/install-x86.sh
sudo nano /etc/edger-rec/config.json    # 填写真实 RTSP
sudo systemctl enable --now edger-rec.target
journalctl -u edger-record -f
```

## 6. 一键回归测试

```bash
./scripts/test-week2-integration.sh
./scripts/test-week3-integration.sh
./scripts/test-week5-integration.sh
./scripts/test-week6-integration.sh
```
