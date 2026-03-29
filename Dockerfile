FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    wget git make pkg-config \
    g++-14 gcc-14 \
    libpq-dev libsodium-dev libssl-dev \
    libasio-dev \
    && rm -rf /var/lib/apt/lists/*

RUN ARCH=$(uname -m) && \
  [ "$ARCH" = "arm64" ] && ARCH="aarch64" || true; \
    wget -q https://github.com/Kitware/CMake/releases/download/v3.30.3/cmake-3.30.3-linux-${ARCH}.sh -O cmake.sh && \
    sh cmake.sh --skip-license --prefix=/usr/local && \
    rm cmake.sh

ENV CC=/usr/bin/gcc-14
ENV CXX=/usr/bin/g++-14

RUN git clone --branch 7.10.1 --depth=1 https://github.com/jtv/libpqxx.git && \
    cmake -S libpqxx -B libpqxx/build \
        -DCMAKE_BUILD_TYPE=Release \
        -DSKIP_BUILD_TEST=ON \
        -DBUILD_SHARED_LIBS=ON && \
    cmake --build libpqxx/build -j$(nproc) && \
    cmake --install libpqxx/build && \
    rm -rf libpqxx

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SERVER=ON -DBUILD_CLIENT=OFF && \
    cmake --build build --target server_app -j$(nproc)

# --- Runtime image ---
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libpq5 libsodium23 ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/local/lib/libpqxx.so* /usr/local/lib/
RUN ldconfig

WORKDIR /app
COPY --from=builder /app/build/server/server_app .

EXPOSE 18080
CMD ["./server_app"]