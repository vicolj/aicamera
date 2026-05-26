#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
INST="${ROOT}/recordings-test/week6-install"
PREFIX="/usr/local"
BIN="${INST}${PREFIX}/bin"
CONFIG="${INST}/etc/edger-rec/config.json"
RUNTIME_CONFIG="${ROOT}/recordings-test/week6-runtime.json"

if ! command -v ffmpeg >/dev/null 2>&1; then
  echo "ffmpeg not found, skip integration test"
  exit 0
fi

"${ROOT}/scripts/test-all.sh"

rm -rf "${INST}" "${ROOT}/recordings-test/week6-rec"
mkdir -p "${ROOT}/recordings-test/week6-rec"

echo "==> staged install"
"${ROOT}/scripts/install-x86.sh" --destdir "${INST}" --prefix "${PREFIX}"

for bin in edger-record-svc edger-ai-svc edger-rec-demo edger-web-server; do
  if [ ! -x "${BIN}/${bin}" ]; then
    echo "missing binary: ${bin}"
    exit 1
  fi
done

UNIT_DIR="${INST}/etc/systemd/system"
for unit in edger-record.service edger-ai.service edger-web.service; do
  if [ ! -f "${UNIT_DIR}/${unit}" ]; then
    echo "missing unit: ${unit}"
    exit 1
  fi
  grep -q 'Restart=on-failure' "${UNIT_DIR}/${unit}"
done

if [ ! -f "${UNIT_DIR}/edger-rec.target" ]; then
  echo "missing unit: edger-rec.target"
  exit 1
fi

grep -q 'edger-record.service' "${UNIT_DIR}/edger-rec.target"
grep -q 'edger-ai.service' "${UNIT_DIR}/edger-rec.target"
grep -q 'edger-web.service' "${UNIT_DIR}/edger-rec.target"

if [ ! -f "${INST}/etc/logrotate.d/edger-rec" ]; then
  echo "missing logrotate config"
  exit 1
fi

CHANNEL_COUNT="$(grep -c '"rtsp_url"' "${CONFIG}")"
if [ "${CHANNEL_COUNT}" -lt 4 ]; then
  echo "expected 4 channels in default config, got ${CHANNEL_COUNT}"
  exit 1
fi

echo "==> simulate restart recovery (4-channel record)"
SOURCE="${ROOT}/recordings-test/week6-rec/source.mp4"
ffmpeg -y -loglevel error -f lavfi -i testsrc=duration=20:size=640x480:rate=10 \
  -c:v libx264 -pix_fmt yuv420p "${SOURCE}"

python3 - <<PY
import json
from pathlib import Path

root = Path("${ROOT}")
out = Path("${ROOT}/recordings-test/week6-rec")
source = Path("${SOURCE}").resolve().as_posix()

data = json.loads((root / "config/test-4ch.json").read_text(encoding="utf-8"))
data["app"]["record_root"] = out.as_posix()
for ch in data["channels"]:
    ch["rtsp_url"] = source
Path("${RUNTIME_CONFIG}").write_text(json.dumps(data, indent=2), encoding="utf-8")
PY

(cd "${ROOT}" && "${BIN}/edger-rec-demo" --config "${RUNTIME_CONFIG}" record-all --duration 5)

MP4_COUNT="$(find "${ROOT}/recordings-test/week6-rec" -name '*.mp4' ! -name 'source.mp4' | wc -l | tr -d ' ')"
if [ "${MP4_COUNT}" -lt 4 ]; then
  echo "expected >=4 mp4 after recovery run, got ${MP4_COUNT}"
  exit 1
fi

echo "Week6 integration test passed."
