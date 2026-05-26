#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build/x86"

"${ROOT}/scripts/build-x86.sh"

cd "${BUILD_DIR}"
ctest --output-on-failure

echo "All unit tests passed."
