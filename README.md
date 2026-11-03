# Will

C++20 gRPC server (`will-server`), terminal client (`will-client`), and load generator (`will-load-clients`).

## Build

Requirements: `xmake`, a C++20 compiler, SQLite3 development libraries, and gRPC/Protobuf development packages (`libgrpc++-dev`, `libprotobuf-dev`, `protobuf-compiler`, `protobuf-compiler-grpc`).

On Debian/Ubuntu:

```bash
sudo apt install build-essential xmake libsqlite3-dev \
  libgrpc++-dev libprotobuf-dev protobuf-compiler protobuf-compiler-grpc
```

Build and run tests:

```bash
xmake
xmake test
```

Binaries are written to `build/`:

| Binary | Description |
|--------|-------------|
| `will-server` | gRPC server |
| `will-client` | Interactive terminal client |
| `will-load-clients` | Concurrent session and message load generator |
