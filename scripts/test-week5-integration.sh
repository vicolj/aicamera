#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build/x86"
WEB_BIN="${BUILD_DIR}/bin/edger-web-server"
OUT="${ROOT}/recordings-test/web-integration"
CONFIG="${OUT}/runtime-config.json"
PORT=18091
COOKIE="${OUT}/cookies.txt"

if ! command -v ffmpeg >/dev/null 2>&1; then
  echo "ffmpeg not found, skip integration test"
  exit 0
fi

"${ROOT}/scripts/build-web-x86.sh"
"${ROOT}/scripts/test-all.sh"

rm -rf "${OUT}"
mkdir -p "${OUT}/ch0_cam-1/20250526"

ffmpeg -y -loglevel error -f lavfi -i testsrc=duration=2:size=320x240:rate=5 \
  -c:v libx264 -pix_fmt yuv420p "${OUT}/ch0_cam-1/20250526/sample.mp4"

python3 - <<PY
import json
import time
from pathlib import Path

out = Path("${OUT}")
clip = (out / "ch0_cam-1/20250526/sample.mp4").resolve()
now = int(time.time())

index = {
  "version": 1,
  "entries": [{
    "channel_id": 0,
    "channel_name": "cam-1",
    "date": "20250526",
    "path": clip.as_posix(),
    "created_at_unix": now,
    "size_bytes": clip.stat().st_size
  }]
}
(out / "index/recordings.json").parent.mkdir(parents=True, exist_ok=True)
(out / "index/recordings.json").write_text(json.dumps(index, indent=2), encoding="utf-8")

config = {
  "app": {
    "record_root": out.as_posix(),
    "retention_days": 7,
    "max_channels": 4,
    "segment_sec": 300,
    "max_storage_gb": 0
  },
  "channels": [{"id": 0, "name": "cam-1", "rtsp_url": "rtsp://127.0.0.1/live", "enabled": True}],
  "alert": {"webhook_url": "", "cooldown_sec": 10},
  "ai": {"intrusion": {"enabled": False, "channel_id": 0, "polygon": []}},
  "web": {
    "enabled": True,
    "listen_host": "127.0.0.1",
    "port": ${PORT},
    "auth_token": "week5-test-token"
  }
}
(out / "runtime-config.json").write_text(json.dumps(config, indent=2), encoding="utf-8")
PY

"${WEB_BIN}" --config "${CONFIG}" --port "${PORT}" &
SERVER_PID=$!

cleanup() {
  kill "${SERVER_PID}" >/dev/null 2>&1 || true
}
trap cleanup EXIT

sleep 1

CODE="$(curl -s -o /dev/null -w '%{http_code}' "http://127.0.0.1:${PORT}/api/recordings")"
if [ "${CODE}" != "401" ]; then
  echo "expected 401 without auth, got ${CODE}"
  exit 1
fi

curl -s -c "${COOKIE}" -X POST "http://127.0.0.1:${PORT}/api/login" \
  -H 'Content-Type: application/json' \
  -d '{"token":"week5-test-token"}' | grep -q '"ok":true'

LIST="$(curl -s -b "${COOKIE}" "http://127.0.0.1:${PORT}/api/recordings?channel_id=0")"
echo "${LIST}" > "${OUT}/list.json"
echo "${LIST}" | grep -q '"count":1'
echo "${LIST}" | grep -q 'sample.mp4'

REL="$(python3 - "${OUT}/list.json" <<'PY'
import json, sys
print(json.load(open(sys.argv[1], encoding="utf-8"))["entries"][0]["media_rel"])
PY
)"
ENC_REL="$(python3 - <<PY
import urllib.parse
print(urllib.parse.quote("""${REL}""", safe=""))
PY
)"

MEDIA_CODE="$(curl -s -b "${COOKIE}" -o /dev/null -w '%{http_code}' \
  "http://127.0.0.1:${PORT}/api/media?rel=${ENC_REL}")"
if [ "${MEDIA_CODE}" != "200" ]; then
  echo "expected media 200, got ${MEDIA_CODE}"
  exit 1
fi

INDEX_CODE="$(curl -s -o /dev/null -w '%{http_code}' "http://127.0.0.1:${PORT}/")"
if [ "${INDEX_CODE}" != "200" ]; then
  echo "expected index.html 200, got ${INDEX_CODE}"
  exit 1
fi

echo "Week5 integration test passed."
