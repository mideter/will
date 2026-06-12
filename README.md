# Will

Will is a C++20 TCP messenger with a dedicated server and chat client. Clients authenticate by phone number and OTP, exchange messages in real time, and can load recent chat history from SQLite on connect.

## Features

- **OTP authentication** — phone number in E.164 format, one-time codes with rate limiting and configurable TTL
- **Persistent chat** — messages stored in SQLite (`will.db` by default)
- **Chat history** — clients can request the last *N* messages on connect (`--history`, default 50)
- **Load testing** — `will-load-clients` opens many concurrent authenticated connections
- **Layered design** — domain use cases, protocol codec, Asio-based TCP transport

## Requirements

- CMake 3.14+
- C++20 compiler (GCC 10+, Clang 12+, or equivalent)
- SQLite3 development libraries
- OpenSSL development libraries (`libssl-dev` / `openssl-devel`)
- pthreads

[ASIO](https://github.com/chriskohlhoff/asio) and [CLI11](https://github.com/CLIUtils/CLI11) are fetched automatically at configure time.

On Debian/Ubuntu:

```bash
sudo apt install build-essential cmake libsqlite3-dev libssl-dev
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

All binaries are written to `build/`:

| Binary | Description |
|--------|-------------|
| `will-server` | TCP server with OTP auth and message persistence |
| `will-client` | Interactive chat client |
| `will-load-clients` | Concurrent connection / message load generator |

## Quick start (local development)

For local testing, run the server with a fixed OTP so you do not need SMS delivery:

**Terminal 1 — server:**

```bash
./build/will-server --dev-fixed-otp 123456 --otp-hash-salt dev --log-otp-for-dev
```

**Terminal 2 — client:**

```bash
./build/will-client --host 127.0.0.1 --port 7770 --phone +15551234567 --otp 123456
```

Type messages in the client terminal; incoming messages from other connected clients appear in real time. On connect the client requests the last 50 messages by default.

Open a second client with a different phone number to chat between two users:

```bash
./build/will-client --phone +15559876543 --otp 123456
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
| `--dev-fixed-otp` | — | Fixed OTP code for development |
| `--log-otp-for-dev` | off | Log generated OTP codes to stderr |
| `--otp-hash-salt` | — | Salt for OTP hashing (required in production) |

### will-client

| Option | Default | Description |
|--------|---------|-------------|
| `--host` | `127.0.0.1` | Server IPv4 address |
| `--port` | `7770` | Server TCP port |
| `--phone` | `+15551234567` | E.164 phone for OTP auth |
| `--otp` | prompt on stdin | OTP code |
| `--history` | `50` | Request last *N* messages on connect |
| `--no-history` | — | Skip history request |
| `--quiet` | off | Suppress server receipt messages on stderr |

### will-load-clients

| Option | Default | Description |
|--------|---------|-------------|
| `--clients` | `100` | Concurrent connections |
| `--messages` | `0` | Chat messages per client (`0` = idle only) |
| `--hold-seconds` | `30` | Keep connections open |
| `--otp` | — | OTP code (required for non-interactive runs) |

Example — 2000 idle connections for 10 seconds:

```bash
./build/will-server --dev-fixed-otp 123456 --otp-hash-salt dev &
./build/will-load-clients --host 127.0.0.1 --port 7770 --otp 123456 --clients 2000 --hold-seconds 10
```

## Tests

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

The test suite covers wire protocol encoding, domain logic, SQLite persistence, CLI parsing, and integration tests that spawn `will-server` / `will-client`.

## Project layout

```
src/
  domain/          Entities, ports, and use cases (OTP, chat, history)
  infra/
    protocol/      Wire message codec and framing
    persistence/   SQLite repositories and OTP storage
    transport/     TCP framed channel (Asio)
    sms/           SMS sender port (logging implementation for dev)
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

The package installs `will-server.service` (enabled on install). OTP hash salt is
generated in `/etc/will-server/environment` on first install. The server logs OTP
codes to the journal (`journalctl -u will-server -f`) — suitable for staging, not
production with real SMS.

See [`packaging/deb/README.Debian`](packaging/deb/README.Debian) for deployment notes and scaling recommendations.
