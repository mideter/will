# AGENTS.md

## Cursor Cloud specific instructions

`will` is a single C++20 product (no monorepo, no package manager). Build/run/test
commands live in `README.md`; system prerequisites are listed there too. Notes below
only cover non-obvious things discovered during setup.

### Build / test / run (see README for full detail)
- Configure + build: `cmake -S . -B build` then `cmake --build build`. Binaries land in `build/`.
- Tests: `ctest --test-dir build --output-on-failure` (12 CTest targets; includes integration
  tests that spawn `will-server`/`will-client`).
- Run (dev): `./build/will-server --dev-fixed-otp 123456 --otp-hash-salt dev --log-otp-for-dev`
  (listens on TCP 7770), then connect clients:
  `./build/will-client --host 127.0.0.1 --port 7770 --phone +15551234567 --otp 123456`.
- No linter/formatter is configured in this repo (no `.clang-format`/`.clang-tidy`); style is
  enforced manually per `CONTRIBUTING.md`. There is no lint command to run.

### Gotchas
- Default `c++`/`cc` is Clang (llvm-18), which pulls in the newest GCC toolchain's libstdc++.
  The build needs `libstdc++-14-dev` installed to match, in addition to the README's
  `libsqlite3-dev` and `libssl-dev`; without it CMake fails at `project()` with
  `cannot find -lstdc++`. These system packages are baked into the VM image, not the update script.
- ASIO (`asio-1-30-2`) and CLI11 (`v2.5.0`) are fetched over the network at `cmake` configure
  time via `FetchContent`; the first configure needs internet access.
- The optional `will-load-clients` target currently fails to compile (`clientconfigvalidator.cpp`
  includes `<asio.hpp>` but that target's CMake include dirs omit the ASIO include path). This is
  a pre-existing source/CMake issue, unrelated to environment setup. `will-server` and
  `will-client` build fine. To build everything except it, use `cmake --build build -- -k`.
- The client is interactive and reads messages from stdin. To send a message non-interactively,
  keep stdin open until the async write flushes, e.g. `{ printf 'hi\n'; sleep 2; } | ./build/will-client ...`.
  Piping a bare `echo` closes stdin (EOF) before the message is sent.
- Chat messages persist to SQLite (`will.db` in the working dir, auto-created). Delete it to reset state.
