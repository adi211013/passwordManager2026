FROM alpine:edge AS builder

RUN apk add --no-cache \
    g++ \
    make \
    cmake \
    git \
    postgresql-dev \
    libpqxx-dev \
    libsodium-dev \
    openssl-dev \
    pkgconf \
    asio-dev

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --target server_app -j$(nproc)

FROM alpine:edge

RUN apk add --no-cache \
    libpq \
    libpqxx \
    libsodium \
    libstdc++ \
    openssl \
    tzdata

WORKDIR /app

COPY --from=builder /app/build/server/server_app .

EXPOSE 18080
CMD ["./server_app"]