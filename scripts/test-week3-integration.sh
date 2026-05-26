#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build/x86"
BIN="${BUILD_DIR}/bin/edger-rec-demo"
OUT="${ROOT}/recordings-test/ai-integration"
SOURCE="${OUT}/source.mp4"
CONFIG="${OUT}/runtime-config.json"
WEBHOOK_LOG="${OUT}/webhook.log"
WEBHOOK_PORT=18080

if ! command -v ffmpeg >/dev/null 2>&1; then
  echo "ffmpeg not found, skip integration test"
  exit 0
fi

"${ROOT}/scripts/test-all.sh"

rm -rf "${OUT}"
mkdir -p "${OUT}"

python3 "${ROOT}/scripts/webhook_server.py" "${WEBHOOK_PORT}" "${WEBHOOK_LOG}" &
WEBHOOK_PID=$!

cleanup() {
  kill "${WEBHOOK_PID}" >/dev/null 2>&1 || true
}
trap cleanup EXIT

sleep 1

ffmpeg -y -loglevel error -f lavfi -i testsrc=size=640x480:rate=10 \
  -t 20 -c:v libx264 -pix_fmt yuv420p "${SOURCE}"

python3 - <<PY
import json
from pathlib import Path

out = Path("${OUT}")
source = Path("${SOURCE}").resolve().as_posix()

data = {
  "app": {
    "record_root": out.as_posix(),
    "retention_days": 7,
    "max_channels": 4,
    "segment_sec": 300
  },
  "channels": [
    {"id": 0, "name": "cam-1", "rtsp_url": source, "enabled": True}
  ],
  "alert": {
    "webhook_url": "http://127.0.0.1:${WEBHOOK_PORT}/hook",
    "cooldown_sec": 10
  },
  "ai": {
    "intrusion": {
      "enabled": True,
      "channel_id": 0,
      "polygon": [[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0]]
    }
  }
}
(out / "runtime-config.json").write_text(json.dumps(data, indent=2), encoding="utf-8")
PY

(cd "${ROOT}" && "${BIN}" --config "${CONFIG}" ai-watch --duration 8)

if [ ! -s "${WEBHOOK_LOG}" ]; then
  echo "expected webhook payload"
  exit 1
fi

grep -q '"intrusion"' "${WEBHOOK_LOG}"

echo "Week3 integration test passed."
