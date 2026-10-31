# Will architecture

This document describes the architecture present in the source tree. It is a
companion to the quick start in `README.md`, the terminology in `GLOSSARY.md`,
and the agent-oriented summary in `AGENTS.md`. Update it when the corresponding
behavior changes.

## 1. System overview

Will consists of a dedicated server, an interactive terminal client, and a load
generator. The server owns a single persistent domain world. Clients authenticate
with device tokens and communicate with that world over gRPC.

The system is not only a shared chat. Its domain includes personal abodes and
rooms, controlled visibility, birth and parentage, trainer–novice relationships,
assignments and their fulfilment, and training sessions with exercises, approaches,
efforts, remarks, and reviews.

Build targets form the following dependency structure:

```text
will-server       → will-transport + will-persistence → will-domain
will-client       → will-transport + will-domain
will-load-clients → will-transport + will-domain
```

The targets keep domain, persistence, and wire transport as separate concerns.

- **Domain** defines beings, values, rules, and storage ports.
- **Persistence** implements those ports with SQLite.
- **Transport** defines protobuf messages and one gRPC service.
- **Server** adapts wire events to domain acts and domain results back to wire events.
- **Client** mostly handles protobuf events and terminal state rather than keeping
  a second domain model.

## 2. Domain composition

The principal composition is:

```text
Eternity
   ↓
Life ── creates one Creation
           ├── Spatiality
           ├── Temporality
           └── World = Heaven + Earth
```

- `Life` owns one `Creation`, restores recollections of words, and caches them
  while gazes hold them.
- `Creation` connects the three dimensions, creates the world, and awakens it.
- `World` owns living men and unborn vessels, welcomes device tokens, gives birth,
  and manages parentage.
- `Heaven` tracks souls, current contemplations, and unanswered supplications.
- `Earth` tracks vessels and the device-token-to-vessel mapping.
- `Space` allocates place IDs; `Time` supplies the common clock.

Important source files are `src/domain/horizons/life.*`, `creation.*`, `world.*`,
`heaven.*`, and `earth.*`.

The horizons use static current-instance pointers. The present design therefore
assumes one living cosmos in a process, even though much of the API uses ordinary
instance types.

### Identity, values, matter, and living beings

- `id::*` strongly distinguishes soul, vessel, place, and word IDs.
- Value objects validate tokens, names, sayings, timestamps, exercises, approaches,
  weights, and strain.
- `matter::*` types are persisted records, not living beings. Composite matter
  gathers records from several dimensions and validates their shared IDs.
- Living objects under `men/`, `places/`, `relations/`, and `words/` implement acts
  and permissions.
- `Birth<Parent>` is a capability that reserves protected creation or acts for the
  proper parent object.
- `Immanent<Horizon>` binds a living object to its horizon and prevents copying or
  moving it.
- Dimensions persist and retrieve matter. They do not decide who may act or what
  an act means.

## 3. People, birth, and parentage

`World::welcome()` serializes token entry:

1. A known token returns its existing vessel and therefore its existing identity.
2. The first new token in an empty world immediately receives a soul, vessel,
   embodiment, and living man.
3. A later new token creates only an unborn vessel.

A host standing at his own gates can bear a waiting vessel. Birth creates the soul
and embodiment, assigns a generated soul name, makes the host the father by flesh,
creates the child's abode, and establishes the initial dwelling relationships.
Unborn state survives restart.

A `Man` inherits the roles `Witness`, `Novice`, and `Testator` simultaneously. A man
is born asleep; waking turns his gaze to his own abode, and sleeping removes that
gaze. Active sessions and gaze state are not persisted.

Parentage rules include:

- one immutable father by flesh;
- a replaceable father by spirit, chosen by the child without the father's consent;
- no self-parenting or spiritual-parentage cycles;
- spiritual ancestors receive friend-level visibility in descendants' abodes;
- lineage is the transitive spiritual descent.

The domain and schema support spiritual parentage, but the current wire handlers
reject `ChooseFather` and `ListLineage` with `NOTICE_FATHER_BY_SPIRIT_CLOSED`.

## 4. Places, rooms, and visibility

Each man has an `Abode`, its outer `Gates`, its inner `Helm`, and an own room called
`Слово` in the interface.

A `Room` reflects another place rather than owning an independent message history.
It can reflect an abode or a tie. The host arranges rooms between the outer and
inner parts of the abode at the helm.

Visibility is directional and depends on the dweller kind:

- `Acquaintance` can enter the abode but sees no rooms.
- `Neighbour` sees outer rooms.
- `Friend` sees inner rooms as well.
- The host sees and administers everything in his own abode.

A person without access to another abode reaches only its gates. A knock exists
while the visitor contemplates those gates; the host must also be there to admit
the visitor. Loss of the required status or moving a room out of the visible part
withdraws a gaze that can no longer remain there.

## 5. Ties, words, and training

A novice asks a dweller of his abode to become his testator. A supplication is kept
until accepted or rejected. Acceptance requires the reciprocal dwelling conditions
and creates one `Tie` with two room views: `Ведение` for the testator and
`Послушание` for the novice.

Words are divided across the dimensions:

- author and body in Eternity;
- placement in Spatiality;
- creation time in Temporality.

The main word types are:

- `Letter` — written through the host's own abode room;
- `Behest` — assigned by a testator through a tie room;
- `Training` — a behest with exercises and planned approaches;
- `Deed` — a novice's one-time fulfilment of a behest;
- `Remark` — a novice's word about a training exercise, attached to that training
  rather than placed in the general tie history.

A training tracks approaches independently for each training. Exercises may be done
in any order, while approaches within an exercise stay ordered. The typical act
sequence is:

```text
begin → [end] → finish/exert → Effort
```

An ended approach must be reported before another begins or the training is
fulfilled. Fulfilment can assemble a deed from recorded efforts. After fulfilment,
new approaches and remarks are closed. Entering a tie room as its testator marks
visible deeds reviewed.

`Recollection` restores all words of a place oldest-first, shares them among gazes,
and accepts new words without rereading storage. Its cache uses weak ownership;
after the last gaze releases it, a later gaze may reconstruct the words again.

## 6. gRPC protocol and server runtime

The complete contract is in `src/infra/transport/messenger.proto`:

```proto
service Messenger {
    rpc Session(stream ClientEvent) returns (stream ServerEvent) {}
}
```

One bidirectional stream multiplexes authentication, navigation, words, history,
access administration, supplications, training, and birth. There are no separate
unary command RPCs.

A nonzero `ClientEvent.request_id` requests one correlated `Reply`. Otherwise,
protocol notices and ordinary server events are used. `Heartbeat` can provide an
application-level empty reply without changing the domain.

### Authentication and takeover

The device token is a bearer credential. `BindToken` calls `World::welcome()`.
`AuthOk` distinguishes a born man from an unborn vessel and can announce the father
by flesh after birth. Unborn sessions may bind and heartbeat but cannot perform
ordinary domain acts.

`SessionRegistry` indexes sessions by both session ID and vessel ID. Binding an
already active vessel displaces and cancels the old session before the new one takes
ownership. Farewell handling checks current vessel ownership so the displaced
session cannot put the newly bound man to sleep.

### Threads and backpressure

The server uses synchronous gRPC:

- each session handler reads client events sequentially on a gRPC worker thread;
- different sessions can execute concurrently;
- a per-session writer thread is the only thread that writes to that gRPC stream;
- other threads enqueue outgoing events in a bounded deque;
- the queue holds at most 1024 events, after which the session is cancelled rather
  than allowing a slow reader to block others indefinitely.

Session teardown informs the protocol adapter, removes registry entries, cancels
the context, clears pending output, and joins the writer. SIGINT/SIGTERM set an
atomic flag; a shutdown thread closes sessions and shuts down the gRPC server so
handlers blocked in reads can finish.

### Protocol/domain mapping

`ProtocolAdapter` and its implementation files translate client events into domain
acts and map domain results or refusals back to notices and replies. Author identity
comes from the authenticated session, not from the name supplied in `ChatMessage`.

When a word is written, the author receives `ReceiptAck`; other current viewers of
the reflected place receive `Word`. Viewers elsewhere in a tie may receive `Stirred`.
The same word can have different `is_mine` values for different recipients.

History belongs to the currently contemplated place. The domain restores its words,
then the server selects the last requested items, up to 500, and sends them in
chronological order followed by `HistoryEnd`. Turning into a room automatically
requests its recent words. This is not a global inbox, and the wire limit does not
bound the initial domain reconstruction work.

## 7. Client

The client stores its device token locally and opens one stream. A reader `jthread`
handles server events while the main thread handles terminal input. Writes are
serialized with a mutex.

Startup is ordered:

1. open the stream and start the reader;
2. bind the token and await the first response;
3. wait for birth if necessary;
4. load the initial view and history;
5. enter the interactive terminal loop.

`ChatSession` implements slash commands and connection workflow. The inbound handler
tracks displayed rooms and current-place UI state. `ConsoleUi` handles colors,
history/live formatting, tables, and prompt redraw. Room numbers in commands refer
to the latest displayed list and are therefore client-session state only.

There is no automatic reconnect in `ChatSession`. Plain gRPC is the development
default; the client can instead trust a PEM root supplied with `--tls-cert`.

## 8. SQLite persistence and recovery

Three databases represent the three dimensions:

| Database | Dimension | Principal contents |
|---|---|---|
| `will.eternity.db` | Eternity | souls, word authors and bodies, parentage, exercise plans, remark links, place-ID high-water mark |
| `will.space.db` | Spatiality | abodes, gates and helm links, dwellers, rooms, placements, ties |
| `will.time.db` | Temporality | vessels and tokens, embodiments, word dates, supplications and answers, underways, efforts, executions, reviews |

Each database has its own connection and mutex. SQLite runs in WAL mode with
`synchronous=NORMAL`. Prepared statements and local transactions have RAII wrappers;
transactions use `BEGIN IMMEDIATE`.

On awakening, the world reconnects souls, vessels, and embodiments, restores unborn
vessels, parentage, ties, rooms, dwellers, and pending supplications, and repairs
certain required dwelling links. Missing abodes or own rooms are recreated where
required. Contradictory kept records are not silently erased; those that cannot be
recollected are reported through `World::unrecollected()`.

Active gRPC sessions, sleep/wake state, current gazes, knocks, and client UI state
are ephemeral. Unfinished training underways and the other records listed above
survive restart.

There is no transaction spanning all three databases. A single logical act can
therefore leave partially written state if failure occurs between independent
commits; recovery handles selected inconsistencies but is not a substitute for a
cross-database atomic journal. Referential consistency is primarily a domain and
recovery responsibility rather than a set of enforced SQLite foreign keys.

## 9. Tests and delivery

The test suite in `tests/xmake.lua` uses doctest and covers:

- IDs and value objects;
- domain acts, invariants, and reconstruction with in-memory dimensions;
- real SQLite persistence and reopen/recovery;
- server/client configuration and CLI parsing;
- session registry behavior;
- real server process integration for authentication, birth, history, rooms,
  access, ties, training, numbered replies, takeover, shutdown, and TLS.

Interactive slash-command handling and the complete terminal-client workflow have
less direct coverage. The load generator exercises concurrent connection and
authentication; its message mode does not, by itself, prove domain acceptance or
persistence of every written message.

`packaging/build-deb.sh` builds a Debian package containing the server, a systemd
unit, and a startup wrapper. The packaged service runs as `will-server`, stores its
databases in `/var/lib/will-server`, and starts in TLS-only mode. It requires the
server certificate and key under `/etc/will-server/tls/`.

## 10. Current constraints to keep in mind

- Device tokens are bearer credentials stored in plaintext in the client file and
  SQLite; use TLS outside a trusted development setup.
- Replacing an unreadable or invalid token file creates a new token, so the old
  identity cannot be recovered from that file.
- History limits bound wire output, not the full cost of recollection.
- The single-cosmos static horizon design is intentional in the current code and
  should not be accidentally multiplied by tests or additional server instances in
  one process.
- The README is a quick start, not a complete description of domain behavior; use
  the source, tests, glossary, and this document when working on the model.
