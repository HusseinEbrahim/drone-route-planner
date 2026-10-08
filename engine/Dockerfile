# Stage 1: compile the C++ server
FROM debian:bookworm-slim AS build
RUN apt-get update \
    && apt-get install -y --no-install-recommends g++ cmake ninja-build git ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF \
    && cmake --build build --target server

# Stage 2: a small image with only the finished program
FROM debian:bookworm-slim
WORKDIR /app
COPY --from=build /src/build/server /app/server
CMD ["/app/server"]