
FROM archlinux:base-devel AS builder

RUN pacman -Syu --noconfirm cmake git postgresql-libs libpqxx libsodium openssl pkgconf asio

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --target server_app -j$(nproc)


FROM archlinux:base

RUN pacman -Syu --noconfirm postgresql-libs libpqxx libsodium openssl && \
    pacman -Scc --noconfirm

WORKDIR /app

COPY --from=builder /app/build/server/server_app .

EXPOSE 18080
CMD ["./server_app"]