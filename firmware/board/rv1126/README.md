# RV1126 板级说明

## 目标硬件

- SoC: Rockchip **RV1126**
- 内存: ≥ 1GB
- 存储: eMMC 或 SD
- 典型形态: 4 路 NVR 盒子 / OEM 主板

## BSP 依赖

- Rockchip Linux SDK（Buildroot 或 Debian）
- MPP（Media Process Platform）
- RKNN Runtime（AI 区域入侵，Week 7）

## 交叉编译（示例）

```bash
export EDGER_RK_TOOLCHAIN=/path/to/aarch64-linux-gnu
export EDGER_RK_SYSROOT=/path/to/sysroot

cmake -S firmware -B build/rv1126 \
  -DEDGER_BOARD=rv1126 \
  -DCMAKE_TOOLCHAIN_FILE=firmware/board/rv1126/toolchain.cmake

cmake --build build/rv1126 -j$(nproc)
```

`toolchain.cmake` 在 Week 7 按实际 SDK 路径补全。

## 与 x86 差异

| 模块 | x86 | RV1126 |
|------|-----|--------|
| 解码/编码 | FFmpeg | MPP |
| AI | ONNX Runtime | RKNN |
| 性能 | 软编 | 硬编 4 路 1080p |
