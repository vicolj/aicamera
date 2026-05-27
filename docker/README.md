# EdgeRec Alert Docker 演示

## 方式一：docker compose（推荐）

```bash
git clone https://github.com/vicolj/edger-rec.git
cd edger-rec

docker compose up -d --build
```

浏览器打开 **http://localhost:8080/**，Token：`edger-demo`

查看日志：

```bash
docker compose logs -f
```

停止：

```bash
docker compose down
```

## 方式二：导出镜像离线导入

在构建机：

```bash
./scripts/docker-save.sh
# 生成 edger-rec-demo.tar（约数百 MB）
```

在演示机：

```bash
docker load -i edger-rec-demo.tar
docker run -d --name edger-rec \
  -p 8080:8080 -p 18080:18080 \
  -v edger-data:/data \
  edger-rec:demo
```

## 端口

| 端口 | 说明 |
|------|------|
| 8080 | Web 回放 + 登录 |
| 18080 | Webhook 接收（演示用，日志在容器 `/data/webhook.log`） |

## 数据卷

`/data` 挂载点：

- `config.json` — 运行配置
- `media/test.mp4` — 自动生成的测试视频
- `recordings/` — 录像与索引
- `webhook.log` — 告警日志

## 进入容器调试

```bash
docker exec -it edger-rec-demo shell
```
