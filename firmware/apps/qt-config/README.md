# Qt 配置工具（Week 4）

本地图形化配置 EdgeRec Alert：

- 通道管理（URL、启用）
- 录像策略（保留天数、容量上限 GB、分片时长）
- 告警 Webhook / 冷却时间
- ROI 区域画框（QGraphicsView 拖动角点）

## 构建

```bash
# 需 Qt5 或 Qt6 Widgets
sudo apt-get install -y qtbase5-dev   # 或 qt6-base-dev

./scripts/build-qt-x86.sh
./build/x86/bin/edger-qt-config --config config/example.json
```

默认 `./scripts/build-x86.sh` 不构建 Qt（`EDGER_BUILD_QT=OFF`），避免 CI 无 Qt 依赖。

## 保存与重载

点击「保存并通知重载」会：

1. 写回 `config.json`
2. 生成 `<config>.reload` 标记文件（generation 时间戳）

运行中的服务可轮询该标记并重新 `Load()` 配置。
