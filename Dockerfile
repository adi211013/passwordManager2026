FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    wget \
    g++-14 \
    gcc-14 \
    make \
    git \
    libpq-dev \
    libsodium-dev \
    libssl-dev \
    pkg-config \
    libasio-dev

RUN ARCH=$(uname -m) && \
    if [ "$ARCH" = "arm64" ]; then ARCH="aarch64"; fi && \
    wget https://github.com/Kitware/CMake/releases/download/v3.30.3/cmake-3.30.3-linux-${ARCH}.sh -O cmake.sh && \
    sh cmake.sh --skip-license --prefix=/usr/local && \
    rm cmake.sh

ENV CC=/usr/bin/gcc-14
ENV CXX=/usr/bin/g++-14

RUN git clone --branch 7.10.1 https://github.com/jtv/libpqxx.git && \
    cd libpqxx && \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DSKIP_BUILD_TEST=ON -DBUILD_SHARED_LIBS=ON && \
    cmake --build build -j$(nproc) && \
    cmake --install build

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --target server_app -j$(nproc)

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

COPY --from=builder /usr/local/lib/libpqxx.so* /usr/local/lib/
RUN ldconfig

RUN apt-get update && apt-get install -y \
    libpq5 \
    libsodium-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/server/server_app .

EXPOSE 18080
CMD ["./server_app"]