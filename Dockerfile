FROM debian:bookworm-slim AS builder

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    cmake \
    git \
    libpq-dev \
    libpqxx-dev \
    libsodium-dev \
    libssl-dev \
    pkg-config \
    libasio-dev

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --target server_app -j$(nproc)

FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    libpq5 \
    libpqxx-dev \
    libsodium-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/server/server_app .

EXPOSE 18080
CMD ["./server_app"]