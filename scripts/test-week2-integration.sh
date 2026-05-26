#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build/x86"
BIN="${BUILD_DIR}/bin/edger-rec-demo"
OUT="${ROOT}/recordings-test/integration"
SOURCE="${OUT}/source.mp4"
CONFIG="${OUT}/runtime-config.json"

if ! command -v ffmpeg >/dev/null 2>&1; then
  echo "ffmpeg not found, skip integration test"
  exit 0
fi

"${ROOT}/scripts/test-all.sh"

rm -rf "${OUT}"
mkdir -p "${OUT}"

ffmpeg -y -loglevel error -f lavfi -i testsrc=duration=30:size=640x480:rate=10 \
  -c:v libx264 -pix_fmt yuv420p "${SOURCE}"

python3 - <<PY
import json
from pathlib import Path

root = Path("${ROOT}")
out = Path("${OUT}")
source = Path("${SOURCE}").resolve().as_posix()

data = json.loads((root / "config/test-4ch-integration.json").read_text(encoding="utf-8"))
data["app"]["record_root"] = out.as_posix()
for ch in data["channels"]:
    ch["rtsp_url"] = source
(out / "runtime-config.json").write_text(json.dumps(data, indent=2), encoding="utf-8")
PY

(cd "${ROOT}" && "${BIN}" --config "${CONFIG}" record-all --duration 5)

MP4_COUNT="$(find "${OUT}" -name '*.mp4' ! -name 'source.mp4' | wc -l | tr -d ' ')"
INDEX_FILE="${OUT}/index/recordings.json"
INDEX_COUNT=0
if [ -f "${INDEX_FILE}" ]; then
  INDEX_COUNT="$(grep -c '"path"' "${INDEX_FILE}" || true)"
fi

echo "integration mp4=${MP4_COUNT} index=${INDEX_COUNT}"

if [ "${MP4_COUNT}" -lt 4 ]; then
  echo "expected at least 4 mp4 files"
  exit 1
fi

if [ "${INDEX_COUNT}" -lt 4 ]; then
  echo "expected at least 4 index entries"
  exit 1
fi

echo "Week2 integration test passed."
