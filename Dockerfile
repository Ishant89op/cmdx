# ── Stage 1: Build ──────────────────────────────────────────────────────
FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential cmake \
    qt6-base-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN cmake -B build \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j$(nproc) \
    && DESTDIR=/install cmake --install build

# ── Stage 2: Runtime ───────────────────────────────────────────────────
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    libqt6widgets6 \
    libqt6gui6 \
    libqt6core6 \
    libgl1 \
    libfontconfig1 \
    libxkbcommon0 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /install/usr/bin/cmdx /usr/bin/cmdx
COPY --from=builder /install/usr/bin/optionsgui /usr/bin/optionsgui
COPY --from=builder /install/usr/bin/optgui /usr/bin/optgui
COPY --from=builder /install/usr/bin/cmdxgui /usr/bin/cmdxgui
COPY --from=builder /install/usr/share/cmdx/ /usr/share/cmdx/

# Verify the build works
RUN cmdx --version && cmdx list | head -3

ENTRYPOINT ["cmdx"]
CMD ["--help"]
