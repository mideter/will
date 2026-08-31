# Will

Will is a C++20 gRPC messenger with a dedicated server and chat client. Clients authenticate with a persistent device token (one user per device), exchange messages in real time, and can load recent chat history from SQLite on connect.

## Features

- **Device-token authentication** — each client generates a persistent token on first run; one token maps to one user account
- **Persistent chat** — messages stored in SQLite (`will.db` by default)
- **Chat history** — clients can request the last *N* messages on connect (`--history`, default 50)
- **Load testing** — `will-load-clients` opens many concurrent authenticated sessions
- **Layered design** — domain use cases, protobuf/gRPC transport, SQLite persistence

## Requirements

- [xmake](https://xmake.io)
- C++20 compiler (GCC 10+, Clang 12+, or equivalent)
- SQLite3 development libraries
- gRPC and Protobuf development libraries (`libgrpc++-dev`, `libprotobuf-dev`, `protobuf-compiler`, `protobuf-compiler-grpc`)

[CLI11](https://github.com/CLIUtils/CLI11) is pulled automatically by xmake. gRPC/Protobuf come from the system via pkg-config.

On Debian/Ubuntu:

```bash
sudo apt install build-essential xmake libsqlite3-dev \
  libgrpc++-dev libprotobuf-dev protobuf-compiler protobuf-compiler-grpc
```

## Build

```bash
xmake
```

All binaries are written to `build/`:

| Binary | Description |
|--------|-------------|
| `will-server` | gRPC server with device-token auth and message persistence |
| `will-client` | Interactive chat client |
| `will-load-clients` | Concurrent session / message load generator |

## Quick start (local development)

**Terminal 1 — server:**

```bash
./build/will-server
```

**Terminal 2 — client:**

```bash
./build/will-client --host 127.0.0.1 --port 7770
```

On first run the client creates `will.device_token` in the current directory and registers with the server. Type messages in the client terminal; incoming messages from other connected clients appear in real time. On connect the client requests the last 50 messages by default.

Open a second client with a different token file to chat between two users:

```bash
./build/will-client --device-token-path will.device_token.user2
```

## CLI reference

Run any binary with `--help` for the full option list.

### will-server

Listens on gRPC port **7770** by default. Key options:

| Option | Default | Description |
|--------|---------|-------------|
| `--port` | `7770` | Listen port |
| `--db-path` | `will.db` | SQLite database file |
| `--max-clients` | `4096` | Max concurrent sessions |
| `--keepalive-interval` | `30` | gRPC HTTP/2 keepalive ping interval (seconds) |
| `--keepalive-timeout` | `10` | gRPC HTTP/2 keepalive ping timeout (seconds) |

### will-client

| Option | Default | Description |
|--------|---------|-------------|
| `--host` | `127.0.0.1` | Server IPv4 address |
| `--port` | `7770` | Server gRPC port |
| `--device-token-path` | `will.device_token` | Path to persistent device token file |
| `--history` | `50` | Request last *N* messages on connect |
| `--no-history` | — | Skip history request |
| `--quiet` | off | Suppress server receipt messages on stderr |

### will-load-clients

| Option | Default | Description |
|--------|---------|-------------|
| `--clients` | `100` | Concurrent sessions |
| `--messages` | `0` | Chat messages per client (`0` = idle only) |
| `--hold-seconds` | `30` | Keep sessions open |

Example — 2000 idle sessions for 10 seconds:

```bash
./build/will-server &
./build/will-load-clients --host 127.0.0.1 --port 7770 --clients 2000 --hold-seconds 10
```

## Tests

```bash
xmake
xmake test
```

The test suite covers domain logic, SQLite persistence, CLI parsing, and integration tests that spawn `will-server` / `will-client`.

## Project layout

```
src/
  domain/              Entities, ports, and use cases (auth, chat, history)
  infra/
    persistence/       SQLite repositories
    transport/         messenger.proto (gRPC Messenger.Session)
  server/              will-server: sessions, auth, gRPC service
  client/              will-client: session, inbound handler, CLI
tests/
  load/                will-load-clients load generator
  *.cpp                Unit and integration tests (xmake test)
packaging/deb/         Debian package and systemd unit
```

## Debian packaging

Build and install:

```bash
./packaging/build-deb.sh
sudo dpkg -i build/deb-packaging/will-server_*.deb
```

The package installs `will-server.service` (enabled on install). Message history is stored in `/var/lib/will-server/will.db`.

See [`packaging/deb/README.Debian`](packaging/deb/README.Debian) for deployment notes and scaling recommendations.
