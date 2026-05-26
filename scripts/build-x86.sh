#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build/x86"

mkdir -p "${BUILD_DIR}"
cmake -S "${ROOT}/firmware" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DEDGER_BOARD=x86

cmake --build "${BUILD_DIR}" -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

cd "${BUILD_DIR}"
ctest --output-on-failure

echo "Build OK: ${BUILD_DIR}/bin/edger-rec-demo"
