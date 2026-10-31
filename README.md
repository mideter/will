# Will

Will is a world where what is willed gets done, and is seen to be done. A behest
is given through a tie between a testator and a novice, fulfilled as a deed with
a report, and reviewed by the one who willed it. The rest of the world is built
around that: who you are to the host — acquaintance, neighbour, or friend —
decides what you may see and where you may stand; a stranger reaches only the
gates and knocks to be admitted; and a body is born into the world once, through
another person, and stays. You enter it from a terminal client; the world lives
on a server of its own and does not forget.

The name is the world's basic act: to *will* something is to leave a behest
behind, and what is willed becomes a deed when it is fulfilled.

![Two will-clients in one world: the first man writes in his room and bears a second body at his gates; the second waits unborn until it is born](docs/terminal.svg)

## It is not a chat room

It is not a chat room that happens to have users. Every man has an abode with
gates, a helm, and rooms. Words belong to places rather than to a global feed: a
letter is written in one's own room, a behest is given in a tie room, a training
carries exercises and approaches, and a deed is its one-time fulfilment.
Correspondence is one act among many, not the shape of the system.

The domain is named in Russian and the code speaks English;
[GLOSSARY.md](GLOSSARY.md) is where the two meet. The full model is described in
[ARCHITECTURE.md](ARCHITECTURE.md). This file is a quick start; the same
document lives in Russian in [README.ru.md](README.ru.md).

## What you can do

- **Ask and serve** — a novice asks a dweller of his abode to become his
  testator; acceptance forms a tie with two views, *shepherding* and *obedience*.
- **Fulfil and review** — a behest given through a tie is owed: the novice
  performs it and reports, and it becomes a one-time deed which the testator
  reviews. A training is a behest with exercises and planned approaches:
  approaches are reported one by one, and strain is marked with `!`.
- **Dwell** — arrange rooms between the outer and inner parts of your abode at
  the helm, and decide what an acquaintance, a neighbour, or a friend may see.
- **Visit** — walk to another abode, contemplate its gates, knock, and be
  admitted.
- **Write** — letters in your own room; they are seen only where that room is
  reflected and only by those allowed to see it.
- **Enter the world** — a device token identifies a body. The first token in an
  empty world is born at once; later ones wait as *unborn* vessels until a host
  bears them at his gates.

## Build

Requirements: [xmake](https://xmake.io), a C++20 compiler (GCC 10+, Clang 12+ or
equivalent), SQLite3 development libraries, and gRPC/Protobuf development
libraries (`libgrpc++-dev`, `libprotobuf-dev`, `protobuf-compiler`,
`protobuf-compiler-grpc`). [CLI11](https://github.com/CLIUtils/CLI11) is pulled
automatically by xmake; gRPC/Protobuf come from the system via pkg-config.

On Debian/Ubuntu:

```bash
sudo apt install build-essential xmake libsqlite3-dev \
  libgrpc++-dev libprotobuf-dev protobuf-compiler protobuf-compiler-grpc
```

Then:

```bash
xmake
```

All binaries are written to `build/`:

| Binary | Description |
|--------|-------------|
| `will-server` | gRPC server owning the world and its persistence |
| `will-client` | Interactive terminal client |
| `will-load-clients` | Concurrent session / message load generator |

## Quick start

The first client you open is born immediately. The second one is not: it is an
unborn vessel, visible to others only as a mark at the gates, and someone must
bear it. This is the intended first experience, not a defect.

**Terminal 1 — server:**

```bash
./build/will-server
```

The server creates `will.eternity.db`, `will.space.db`, and `will.time.db` in the
working directory. The world lives across restarts, and only these three files
are the world: remove them to start over. The mark `#2` below assumes they are
gone.

**Terminal 2 — the first man:**

```bash
./build/will-client --host 127.0.0.1 --port 7770
```

On first run the client creates `will.device_token` in the current directory and
registers with the server. You are born with a generated name, an abode, a room
called `Слово` in the interface, and the roles of witness, novice, and testator
at once. Type text in a room to write there; `/help` lists the commands, and
`Ctrl+D` leaves the client.

**Terminal 3 — a second body:**

```bash
./build/will-client --device-token-path will.device_token.user2
```

This one prints that it is not yet born and shows its mark: `#2` in a fresh
world. It can do nothing else until it is born.

**Back in terminal 2**, stand at your gates and bear it:

```text
/birth            stand at your gates and keep them
                  the client lists "Awaiting birth" with a mark
/bear 2           bear the vessel with that mark
```

Terminal 3 is now born, receives a name, his own abode, and a dwelling in the
host's abode. Both clients can talk, visit, and arrange their worlds.

Do not delete the token file to "start over": the body it created stays in the
world, and a new token produces another waiting body. Use `--device-token-path`
to keep identities apart; a lost token file cannot be recovered. Two clients
sharing one token file do not give you two sessions: a vessel has one active
session, and the later bind displaces the earlier one.

A public demo world you can visit is described in
[REFERENCE.md](REFERENCE.md#visiting-the-public-world).

## Project layout

```
src/
  domain/    living beings, values, matter, storage ports, horizons, refusals
  infra/     SQLite persistence (three databases), gRPC transport (messenger.proto)
  server/    will-server: sessions, protocol/domain mapping, lifecycle
  client/    will-client: transport, interactive workflow, terminal UI
tests/       unit, component, process, and gRPC integration tests
packaging/   Debian package, systemd service, deployment
```

## Tests

```bash
xmake
xmake test
```

The suite uses doctest and runs from unit rules up to tests that spawn real
`will-server` and `will-client` processes. What it covers, and where its
coverage is thin, is kept in one place:
[ARCHITECTURE.md §9](ARCHITECTURE.md#9-tests-and-delivery).

## Before exposing it

Device tokens are bearer credentials, stored in plaintext in the client file and
in SQLite, and a body keeps its identity permanently: a lost token file cannot be
recovered. Use TLS outside a trusted development setup. This is not a limitation
of the protocol but a property of the world — a man is one, and his token is what
points at him.

## Documentation

| File | Content |
|------|---------|
| [README.ru.md](README.ru.md) | This document in Russian |
| [REFERENCE.md](REFERENCE.md) | CLI reference, client commands, the demo world, Debian packaging |
| [ARCHITECTURE.md](ARCHITECTURE.md) | System and domain architecture, persistence, recovery, constraints |
| [GLOSSARY.md](GLOSSARY.md) | Russian domain terms, English code names, interface words |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Naming, formatting, comments, commits |
| [AGENTS.md](AGENTS.md) | Orientation for agents working in the tree |
| [`src/infra/transport/messenger.proto`](src/infra/transport/messenger.proto) | Complete wire contract |
