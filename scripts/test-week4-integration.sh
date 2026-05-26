#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build/x86"
BIN="${BUILD_DIR}/bin/edger-rec-demo"
OUT="${ROOT}/recordings-test/qt-integration"
CONFIG="${OUT}/runtime-config.json"

"${ROOT}/scripts/test-all.sh"

rm -rf "${OUT}"
mkdir -p "${OUT}"

python3 - <<PY
import json
from pathlib import Path

root = Path("${ROOT}")
out = Path("${OUT}")
example = json.loads((root / "config/example.json").read_text(encoding="utf-8"))
example["app"]["record_root"] = out.as_posix()
example["app"]["retention_days"] = 21
example["app"]["max_storage_gb"] = 256
example["ai"]["intrusion"]["polygon"] = [
    [0.25, 0.25], [0.75, 0.25], [0.75, 0.75], [0.25, 0.75]
]
(out / "runtime-config.json").write_text(json.dumps(example, indent=2), encoding="utf-8")
PY

(cd "${ROOT}" && "${BIN}" --config "${CONFIG}" summary > "${OUT}/summary.txt")

grep -q "retention_days: 21" "${OUT}/summary.txt"
grep -q "max_storage_gb: 256" "${OUT}/summary.txt"
grep -q "polygon: 0.25,0.25" "${OUT}/summary.txt"

RELOAD_OUT="$(cd "${ROOT}" && "${BIN}" --config "${CONFIG}" config-reload)"
echo "${RELOAD_OUT}" | grep -q "reload_generation="
GENERATION="$(echo "${RELOAD_OUT}" | awk -F= '{print $2}')"
test "${GENERATION}" -gt 0

echo "Week4 integration test passed."
