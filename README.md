# Will

Will is a C++20 TCP messenger with a dedicated server and chat client. Clients authenticate with a persistent device token (one user per device), exchange messages in real time, and can load recent chat history from SQLite on connect.

## Features

- **Device-token authentication** — each client generates a persistent token on first run; one token maps to one user account
- **Persistent chat** — messages stored in SQLite (`will.db` by default)
- **Chat history** — clients can request the last *N* messages on connect (`--history`, default 50)
- **Load testing** — `will-load-clients` opens many concurrent authenticated connections
- **Layered design** — domain use cases, protocol codec, Asio-based TCP transport

## Requirements

- [xmake](https://xmake.io)
- C++20 compiler (GCC 10+, Clang 12+, or equivalent)
- SQLite3 development libraries
- OpenSSL development libraries (`libssl-dev` / `openssl-devel`)
- pthreads

[ASIO](https://github.com/chriskohlhoff/asio) and [CLI11](https://github.com/CLIUtils/CLI11) are pulled automatically by xmake.

On Debian/Ubuntu:

```bash
sudo apt install build-essential xmake libsqlite3-dev libssl-dev
```

## Build

```bash
xmake
```

All binaries are written to `build/`:

| Binary | Description |
|--------|-------------|
| `will-server` | TCP server with device-token auth and message persistence |
| `will-client` | Interactive chat client |
| `will-load-clients` | Concurrent connection / message load generator |

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

Listens on TCP port **7770** by default. Key options:

| Option | Default | Description |
|--------|---------|-------------|
| `--port` | `7770` | Listen port |
| `--db-path` | `will.db` | SQLite database file |
| `--max-clients` | `4096` | Max concurrent connections |
| `--io-threads` | `4` | `io_context` worker threads |

### will-client

| Option | Default | Description |
|--------|---------|-------------|
| `--host` | `127.0.0.1` | Server IPv4 address |
| `--port` | `7770` | Server TCP port |
| `--device-token-path` | `will.device_token` | Path to persistent device token file |
| `--history` | `50` | Request last *N* messages on connect |
| `--no-history` | — | Skip history request |
| `--quiet` | off | Suppress server receipt messages on stderr |

### will-load-clients

| Option | Default | Description |
|--------|---------|-------------|
| `--clients` | `100` | Concurrent connections |
| `--messages` | `0` | Chat messages per client (`0` = idle only) |
| `--hold-seconds` | `30` | Keep connections open |

Example — 2000 idle connections for 10 seconds:

```bash
./build/will-server &
./build/will-load-clients --host 127.0.0.1 --port 7770 --clients 2000 --hold-seconds 10
```

## Tests

```bash
xmake
xmake test
```

The test suite covers wire protocol encoding, domain logic, SQLite persistence, CLI parsing, and integration tests that spawn `will-server` / `will-client`.

## Project layout

```
src/
  domain/          Entities, ports, and use cases (auth, chat, history)
  infra/
    protocol/      Wire message codec and framing
    persistence/   SQLite repositories
    transport/     TCP framed channel (Asio)
  server/          will-server: connections, auth, protocol adapter
  client/          will-client: session, inbound handler, CLI
tools/
  load_clients.cpp   will-load-clients entry point
tests/               Unit and integration tests
packaging/deb/       Debian package and systemd unit
```

## Debian packaging

Build and install:

```bash
./packaging/build-deb.sh
sudo dpkg -i build/deb-packaging/will-server_*.deb
```

The package installs `will-server.service` (enabled on install). Message history is stored in `/var/lib/will-server/will.db`.

See [`packaging/deb/README.Debian`](packaging/deb/README.Debian) for deployment notes and scaling recommendations.
