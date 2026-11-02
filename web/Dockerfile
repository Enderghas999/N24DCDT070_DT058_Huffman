# DT058 - Ma Huffman: anh Docker de trien khai len Render (hoac bat ky dich vu Docker nao).
# Buoc 1: bien dich server (co PostgreSQL) va CHAY bo kiem tra; neu kiem tra sai thi dung build.
FROM ubuntu:24.04 AS build
RUN apt-get update && apt-get install -y --no-install-recommends g++ libpq-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN g++ -std=c++17 -O2 tests/kiem_tra.cpp -o kiem_tra && ./kiem_tra 2000 2026 > kiem_tra.log \
    && tail -n 1 kiem_tra.log
RUN g++ -std=c++17 -O2 -DUSE_PG -I/usr/include/postgresql server.cpp -o huffman_app -lpthread -lpq

# Buoc 2: anh chay gon nhe
FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends libpq5 ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=build /src/huffman_app /app/huffman_app
COPY --from=build /src/kiem_tra.log /app/kiem_tra.log
COPY public /app/public
ENV PUBLIC_DIR=/app/public HISTORY_FILE=/app/data/lich_su.jsonl PORT=10000
EXPOSE 10000
CMD ["/app/huffman_app"]
