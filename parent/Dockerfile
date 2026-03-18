FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        pkg-config \
        protobuf-compiler \
        libprotobuf-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY parent ./parent
COPY enclave/proto ./enclave/proto

RUN cmake -S parent -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release -j"$(nproc)"

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libprotobuf-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/sum5_parent /app/sum5_parent

ENTRYPOINT ["/app/sum5_parent"]
