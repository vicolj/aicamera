#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMAGE="${EDGER_IMAGE:-edger-rec:demo}"

cd "${ROOT}"
docker build -t "${IMAGE}" .

echo "Built: ${IMAGE}"
echo "Run:   docker compose up -d"
echo "Save:  docker save ${IMAGE} -o edger-rec-demo.tar"
