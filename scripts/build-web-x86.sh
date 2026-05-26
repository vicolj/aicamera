#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build/x86"

mkdir -p "${BUILD_DIR}"
cmake -S "${ROOT}/firmware" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DEDGER_BOARD=x86 \
  -DEDGER_BUILD_QT=OFF \
  -DEDGER_BUILD_WEB=ON

cmake --build "${BUILD_DIR}" -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo "Web build OK: ${BUILD_DIR}/bin/edger-web-server"
