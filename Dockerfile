# EdgeRec Alert — 一键演示镜像
# 构建: docker build -t edger-rec:demo .
# 导出: docker save edger-rec:demo -o edger-rec-demo.tar

FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake pkg-config git \
    libavformat-dev libavcodec-dev libavutil-dev libswscale-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY firmware/ firmware/
COPY config/ config/

RUN cmake -S firmware -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DEDGER_BOARD=x86 \
    -DEDGER_BUILD_TESTS=OFF \
    -DEDGER_BUILD_QT=OFF \
    -DEDGER_BUILD_WEB=ON \
    && cmake --build build -j"$(nproc)"

# -----------------------------------------------------------------------------

FROM ubuntu:22.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive
ENV EDGER_HOME=/opt/edger-rec
ENV EDGER_DATA=/data
ENV EDGER_CONFIG=/data/config.json
ENV EDGER_WEB_STATIC_DIR=/opt/edger-rec/share/web
ENV PATH="${EDGER_HOME}/bin:${PATH}"

RUN apt-get update && apt-get install -y --no-install-recommends \
    ffmpeg python3 ca-certificates curl \
    libavformat58 libavcodec58 libavutil56 libswscale5 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/x86/bin/edger-rec-demo \
                    /src/build/x86/bin/edger-record-svc \
                    /src/build/x86/bin/edger-ai-svc \
                    /src/build/x86/bin/edger-web-server \
                    ${EDGER_HOME}/bin/
COPY --from=builder /src/firmware/apps/web-server/static ${EDGER_WEB_STATIC_DIR}/
COPY scripts/webhook_server.py ${EDGER_HOME}/scripts/
COPY docker/entrypoint.sh ${EDGER_HOME}/entrypoint.sh

RUN chmod +x ${EDGER_HOME}/entrypoint.sh ${EDGER_HOME}/bin/*

VOLUME ["${EDGER_DATA}"]
EXPOSE 8080 18080

HEALTHCHECK --interval=30s --timeout=5s --start-period=60s --retries=3 \
    CMD curl -fsS http://127.0.0.1:8080/api/health || exit 1

ENTRYPOINT ["/opt/edger-rec/entrypoint.sh"]
CMD ["demo"]
