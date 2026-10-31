# Will — guide for AI agents

## What this project is

Will is a C++20 client/server system with a gRPC transport and a persistent,
domain-driven world. The README gives the quick start; behind it the code models
personal abodes, visibility and access, birth and parentage, trainer–novice ties,
assignments, deeds, and detailed training sessions.

The domain model is the source of truth for behavior. Do not reduce a task to
its transport or database detail when the domain rules are relevant.

## Read first

1. `README.md` — build, launch, and basic CLI usage.
2. `ARCHITECTURE.md` — the current system and domain architecture.
3. `GLOSSARY.md` — canonical Russian domain terms, English code names, and UI words.
4. `CONTRIBUTING.md` — naming, formatting, comments, and other project conventions.
5. `.cursor/rules/*.mdc` — additional repository guidance, applicable even when
   working outside Cursor. In particular, preserve domain invariants and the
   separation between living beings and persisted matter.

For behavior not covered by these documents, read the implementation and tests
before changing anything.

## Source map

- `src/domain/identity/` — strongly typed IDs.
- `src/domain/values/` — validated value objects.
- `src/domain/matter/` — persisted records used to reconstruct living objects.
- `src/domain/men/` — people, their embodied state, and their roles.
- `src/domain/places/` — abodes, rooms, gates, helm, and ties.
- `src/domain/relations/` — acquaintance, contemplation, and supplication.
- `src/domain/words/` — letters, behests, deeds, recollections, and training.
- `src/domain/dimensions/` — persistence ports: Eternity, Spatiality, Temporality.
- `src/domain/horizons/` — Life, Creation, World, Heaven, Earth, Space, and Time.
- `src/domain/properties/` — Birth capabilities and Immanent horizon membership.
- `src/domain/refusals/` — typed domain refusals.
- `src/infra/transport/messenger.proto` — the complete wire contract.
- `src/infra/persistence/` — SQLite adapters and schema.
- `src/server/` — sessions, protocol/domain mapping, and server lifecycle.
- `src/client/` — client transport, interactive workflow, and terminal UI.
- `tests/` — unit, component, process, and gRPC integration tests.
- `packaging/` — Debian package, systemd service, and deployment scripts.

## Architecture rules to preserve

- Domain code must not depend on gRPC, protobuf, SQLite, or terminal UI.
- Living domain objects enforce permissions and invariants; dimensions store
  matter and do not make domain decisions.
- Reconstruct living objects from matter rather than manually unpacking records
  at every call site.
- Respect `Birth<Parent>` capabilities and ownership encoded in constructors.
- Preserve strongly typed IDs and validated values instead of passing raw integers
  or strings through domain APIs.
- A server process owns one shared world. All sessions must observe the same state.
- The first token in an empty world produces a born man; later new tokens produce
  an unborn vessel until an existing host bears it at the gates.
- Words belong to places, not to a global chat. Visibility depends on place,
  dweller kind, current contemplation, and role.
- A man is simultaneously Witness, Novice, and Testator; these are roles, not
  mutually exclusive lifecycle stages.
- One device vessel has at most one active session; binding the same vessel again
  displaces the previous session.
- Keep protobuf-specific conversion in the transport/protocol layer and UI-only
  state in the client.

## Build and verification

From the repository root:

```bash
xmake
xmake test
```

The main binaries are written to `build/`:

- `will-server`
- `will-client`
- `will-load-clients`

Tests use doctest. Integration tests launch real server processes and use local
ports, temporary SQLite databases, and POSIX process APIs. Run the relevant
focused test while iterating, then run the full suite before finishing.

For a protocol change, check the proto contract, server adapter, client handling,
and integration tests together. For a persistence change, verify reconstruction
after reopening all three databases, not only successful writes.

## Change discipline

- Keep changes scoped and follow the existing style in `CONTRIBUTING.md`.
- Preserve the ontological meaning in domain class comments and the glossary.
- Add or update tests for changed domain rules, wire behavior, or recovery rules.
- Update `ARCHITECTURE.md` when architecture, workflows, storage, or important
  limitations change. Update `GLOSSARY.md` when domain names change.
- Do not treat transient working-tree state, test-run timestamps, or version bumps
  as durable architecture documentation.
