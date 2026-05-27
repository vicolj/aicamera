#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMAGE="${EDGER_IMAGE:-edger-rec:demo}"
OUTPUT="${1:-${ROOT}/edger-rec-demo.tar}"

"${ROOT}/scripts/docker-build.sh"

echo "==> saving ${IMAGE} -> ${OUTPUT}"
docker save "${IMAGE}" -o "${OUTPUT}"
ls -lh "${OUTPUT}"

echo ""
echo "Import on another machine:"
echo "  docker load -i edger-rec-demo.tar"
echo "  docker run -d -p 8080:8080 -p 18080:18080 -v edger-data:/data ${IMAGE}"
