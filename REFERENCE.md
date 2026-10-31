# Reference

The [README](README.md) is the quick start; this file is the complete
reference: every option of every binary, every client command, the public demo
world, and Debian packaging. Run any binary with `--help` for the same option
list.

## CLI reference

### will-server

| Option | Default | Description |
|--------|---------|-------------|
| `--port` | `7770` | Plain (open) listen port |
| `--tls-port` | `17771` | TLS listen port, used when a certificate and key are given |
| `--tls-cert` | — | TLS certificate chain (PEM) |
| `--tls-key` | — | TLS private key (PEM) |
| `--no-plain` | off | Do not listen in the open (TLS only) |
| `--db-path` | `.` | Existing directory of the SQLite databases (`will.eternity.db`, `will.space.db`, `will.time.db`) |
| `--max-clients` | `4096` | Max concurrent sessions |
| `--keepalive-interval` | `30` | gRPC HTTP/2 keepalive ping interval (seconds) |
| `--keepalive-timeout` | `10` | gRPC HTTP/2 keepalive ping timeout (seconds) |

Given both `--tls-cert` and `--tls-key`, the server listens with TLS on
`--tls-port` beside the open port. Given neither, it speaks only in the open.
`--no-plain` leaves only TLS.

### will-client

| Option | Default | Description |
|--------|---------|-------------|
| `--host` | `127.0.0.1` | Server IPv4 address |
| `--port` | `7770` | Server TCP port; with `--tls-cert`, TLS defaults to `17771` |
| `--tls-cert` | — | The server's certificate (PEM) to trust: speak TLS and trust it alone |
| `--device-token-path` | `will.device_token` | Path to persistent device token file |
| `--history` | `50` | Request last *N* words on connect |
| `--no-history` | — | Skip the history request |
| `--quiet` | off | Suppress the delivery checkmarks on `[me]` lines |
| `--color` | `auto` | Color output: `auto`, `always`, or `never` |

### will-load-clients

| Option | Default | Description |
|--------|---------|-------------|
| `--host` | `127.0.0.1` | Server IPv4 address |
| `--port` | `7770` | Server TCP port |
| `--clients` | `100` | Concurrent sessions |
| `--messages` | `0` | Words per client (`0` = idle only) |
| `--hold-seconds` | `30` | Keep sessions open |

Example — 2000 idle sessions for 10 seconds:

```bash
./build/will-server &
./build/will-load-clients --host 127.0.0.1 --port 7770 --clients 2000 --hold-seconds 10
```

## Client commands

`/help` prints the same list inside the client. Numbers in commands refer to the
latest displayed list, so they are client-session state, not world state.

A word is spoken from a place, and from anywhere else the domain refuses it:
`/birth` and `/admit` keep your own gates, `/knock` sounds at the gates you
contemplate, and `/helm`, `/arrange`, `/regard`, `/accept`, and `/reject` want you
at your helm. To another abode you travel from your own gates, with `/visit <name>`.

- **Places** — `/rooms [name]`, `/room <number or name>`, `/home`, `/visit <name> [room]`
- **Gates** — `/gates [name]`, `/helm`, `/knock`, `/admit <name>`
- **Dwellers** — `/dwellers`, `/regard <name> acquaintance|neighbour|friend`, `/dwellings`
- **Arranging** — `/arrange inner|outer <number or room>`
- **Obedience** — `/ask <name>`, `/accept <name>`, `/reject <name>`, `/supplications`
- **Training** — `/train [title] | <exercise> <sets> | …`, `/begin <training> <exercise> <approach>`, `/end <training>`, `/finish <training> <sets>`, `/remark <training> <exercise> <text>`
- **Birth** — `/birth`, `/bear <mark>`

`/done` fulfils what is owed, naming it from the list it was shown in and carrying a
report. A training additionally lists what was performed, each exercise under its own
name: `/done 3 light day | press 60x10 70x8 |`.

An exercise is written as `60x10 1:30/70x8 2:00/80x6`, where `1:30/` is the rest
before the next set; `0x8` is a set at one's own weight. A training's exercises may be
done in any order, while the approaches within one exercise stay ordered. Trailing
`!` marks on `/finish` record strain: `/finish 3 60x10 !!`.

## Visiting the public world

A demo server stands at `83.217.202.145`, speaking TLS on `17771`; handing a
certificate to `will-client` is enough — it moves to that port by itself. The
certificate is self-signed, `CN=will`, good until 21 October 2036, and its SHA-256
fingerprint is:

```text
FE:DD:3E:F4:F9:5B:8F:4D:5E:D5:26:97:CA:38:0A:B3:03:10:5B:2F:D9:FE:45:E0:15:2E:D7:63:26:7A:00:21
```

Fetch the certificate, compare the fingerprint against the line above — read it from
somewhere other than the connection you are about to trust — and then enter:

```bash
openssl s_client -connect 83.217.202.145:17771 </dev/null 2>/dev/null | openssl x509 > will.server.crt
openssl x509 -in will.server.crt -noout -fingerprint -sha256

./build/will-client --host 83.217.202.145 --tls-cert will.server.crt
```

That world is shared and it is not fresh: you arrive as an unborn vessel and wait for
a host standing at his gates to bear you, as in the quick start. Give it its own token
file with `--device-token-path`: one token in two worlds ties those identities
together, and a leak in either is a leak in both.

## Debian packaging

Build and install:

```bash
./packaging/build-deb.sh
sudo dpkg -i build/deb-packaging/will-server_*.deb
```

The package installs `will-server.service` (enabled on install) and stores the
three databases in `/var/lib/will-server`. The packaged service starts in
TLS-only mode and requires `server.crt` and `server.key` under
`/etc/will-server/tls/`; without them it refuses to start. See
[`packaging/deb/README.Debian`](packaging/deb/README.Debian) for deployment notes
and scaling recommendations.
