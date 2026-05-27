#!/usr/bin/env bash
set -euo pipefail

EDGER_HOME="${EDGER_HOME:-/opt/edger-rec}"
EDGER_DATA="${EDGER_DATA:-/data}"
EDGER_CONFIG="${EDGER_CONFIG:-${EDGER_DATA}/config.json}"
MEDIA="${EDGER_DATA}/media/test.mp4"
RECORD_ROOT="${EDGER_DATA}/recordings"
WEBHOOK_LOG="${EDGER_DATA}/webhook.log"
MODE="${1:-demo}"

mkdir -p "${EDGER_DATA}/media" "${RECORD_ROOT}" "${RECORD_ROOT}/index"

write_config() {
  cat >"${EDGER_CONFIG}" <<EOF
{
  "app": {
    "record_root": "${RECORD_ROOT}",
    "retention_days": 7,
    "max_channels": 4,
    "segment_sec": 60,
    "max_storage_gb": 2
  },
  "channels": [
    {
      "id": 0,
      "name": "demo-cam",
      "rtsp_url": "${MEDIA}",
      "enabled": true
    }
  ],
  "alert": {
    "webhook_url": "http://127.0.0.1:18080/hook",
    "cooldown_sec": 10
  },
  "ai": {
    "intrusion": {
      "enabled": true,
      "channel_id": 0,
      "polygon": [[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0]]
    }
  },
  "web": {
    "enabled": true,
    "listen_host": "0.0.0.0",
    "port": 8080,
    "auth_token": "edger-demo"
  }
}
EOF
}

ensure_media() {
  if [ -f "${MEDIA}" ]; then
    return
  fi
  echo "[demo] generating test video ${MEDIA}"
  ffmpeg -y -loglevel error -f lavfi -i testsrc=size=640x480:rate=10 \
    -t 120 -c:v libx264 -pix_fmt yuv420p "${MEDIA}"
}

start_webhook() {
  echo "[demo] webhook http://127.0.0.1:18080/hook -> ${WEBHOOK_LOG}"
  python3 "${EDGER_HOME}/scripts/webhook_server.py" 18080 "${WEBHOOK_LOG}" &
  WEBHOOK_PID=$!
}

record_loop() {
  while true; do
    echo "[demo] recording 60s..."
    "${EDGER_HOME}/bin/edger-rec-demo" --config "${EDGER_CONFIG}" record --duration 60 \
      || echo "[demo] record round finished with error"
    sleep 5
  done
}

start_demo() {
  ensure_media
  write_config

  echo "=========================================="
  echo " EdgeRec Alert Docker Demo"
  echo " Web UI:    http://localhost:8080/"
  echo " Token:     edger-demo"
  echo " Webhook:   http://localhost:18080/hook"
  echo " Data dir:  ${EDGER_DATA}"
  echo "=========================================="

  start_webhook
  trap 'kill ${WEBHOOK_PID:-} 2>/dev/null || true; kill ${RECORD_PID:-} ${AI_PID:-} 2>/dev/null || true' EXIT

  echo "[demo] bootstrap record 30s"
  "${EDGER_HOME}/bin/edger-rec-demo" --config "${EDGER_CONFIG}" record --duration 30 || true

  record_loop &
  RECORD_PID=$!

  "${EDGER_HOME}/bin/edger-ai-svc" --config "${EDGER_CONFIG}" &
  AI_PID=$!

  exec "${EDGER_HOME}/bin/edger-web-server" --config "${EDGER_CONFIG}"
}

case "${MODE}" in
  demo)
    start_demo
    ;;
  shell|bash|sh)
    exec /bin/bash
    ;;
  *)
    exec "${MODE}" "$@"
    ;;
esac
