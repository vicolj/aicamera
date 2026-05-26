#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build/x86"
PREFIX="/usr/local"
DESTDIR=""
ENABLE_SYSTEMD=1
WITH_WEB=1

usage() {
  cat <<EOF
Usage: $0 [options]

  --prefix PATH       Install prefix (default: /usr/local)
  --destdir PATH      Staging root (for packaging)
  --no-systemd        Skip systemd/logrotate install
  --no-web            Skip edger-web-server
  -h, --help          Show help
EOF
}

while [ $# -gt 0 ]; do
  case "$1" in
    --prefix)
      PREFIX="$2"
      shift 2
      ;;
    --destdir)
      DESTDIR="$2"
      shift 2
      ;;
    --no-systemd)
      ENABLE_SYSTEMD=0
      shift
      ;;
    --no-web)
      WITH_WEB=0
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "unknown option: $1"
      usage
      exit 1
      ;;
  esac
done

STAGING="${DESTDIR}"
BIN_DIR="${STAGING}${PREFIX}/bin"
CONFIG_DIR="${STAGING}/etc/edger-rec"
DATA_DIR="${STAGING}/var/lib/edger-rec"
LOG_DIR="${STAGING}/var/log/edger-rec"
WEB_SHARE="${STAGING}${PREFIX}/share/edger-rec/web"

echo "==> build firmware"
"${ROOT}/scripts/build-x86.sh"

if [ "${WITH_WEB}" -eq 1 ]; then
  echo "==> build web server"
  "${ROOT}/scripts/build-web-x86.sh"
fi

echo "==> install binaries to ${BIN_DIR}"
mkdir -p "${BIN_DIR}"
install -m 755 "${BUILD_DIR}/bin/edger-record-svc" "${BIN_DIR}/"
install -m 755 "${BUILD_DIR}/bin/edger-ai-svc" "${BIN_DIR}/"
install -m 755 "${BUILD_DIR}/bin/edger-rec-demo" "${BIN_DIR}/"
if [ "${WITH_WEB}" -eq 1 ] && [ -f "${BUILD_DIR}/bin/edger-web-server" ]; then
  install -m 755 "${BUILD_DIR}/bin/edger-web-server" "${BIN_DIR}/"
  mkdir -p "${WEB_SHARE}"
  cp -r "${ROOT}/firmware/apps/web-server/static/." "${WEB_SHARE}/"
fi

echo "==> install config"
mkdir -p "${CONFIG_DIR}" "${DATA_DIR}/recordings" "${LOG_DIR}"
if [ ! -f "${CONFIG_DIR}/config.json" ]; then
  install -m 644 "${ROOT}/deploy/config/config.json" "${CONFIG_DIR}/config.json"
else
  echo "keep existing ${CONFIG_DIR}/config.json"
fi

if [ "${ENABLE_SYSTEMD}" -eq 1 ]; then
  UNIT_DIR="${STAGING}/etc/systemd/system"
  echo "==> install systemd units to ${UNIT_DIR}"
  mkdir -p "${UNIT_DIR}"
  for unit in edger-rec.target edger-record.service edger-ai.service; do
    sed "s|/usr/local|${PREFIX}|g" \
      "${ROOT}/deploy/systemd/${unit}" > "${UNIT_DIR}/${unit}"
  done
  if [ "${WITH_WEB}" -eq 1 ]; then
    sed "s|/usr/local|${PREFIX}|g" \
      "${ROOT}/deploy/systemd/edger-web.service" > "${UNIT_DIR}/edger-web.service"
  fi

  LOGROTATE_DIR="${STAGING}/etc/logrotate.d"
  mkdir -p "${LOGROTATE_DIR}"
  install -m 644 "${ROOT}/deploy/logrotate/edger-rec" "${LOGROTATE_DIR}/edger-rec"
fi

cat <<EOF

EdgeRec Alert installed.

  Binaries:  ${BIN_DIR}
  Config:    ${CONFIG_DIR}/config.json
  Data:      ${DATA_DIR}/recordings

Next steps (on target host):
  1. Edit ${CONFIG_DIR}/config.json with your RTSP URLs
  2. sudo systemctl daemon-reload
  3. sudo systemctl enable --now edger-rec.target
  4. journalctl -u edger-record -f

EOF
