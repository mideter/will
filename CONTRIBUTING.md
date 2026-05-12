# Contributing to Will

This document records project conventions so changes stay consistent. When in doubt, match existing code in `src/`.

## Language and toolchain

- C++ standard and compiler flags are defined in CMake; prefer portable constructs unless a platform-specific block is already established.
- Build and run: see [README.md](README.md).

## Layout and modules

- Library and binary sources live under `src/`; keep new features near related modules rather than introducing parallel trees without reason.
- Public headers shipped or included across targets should stay minimal; prefer implementation detail in `.cpp` files.

## Naming

- **Namespace:** `will` for project code.
- **Types (classes, structs, enums, type aliases):** `PascalCase` (for example `ClientHub`, `ConnectionAcceptor`).
- **Functions and methods:** `snake_case` (for example `listen_fd`, `accept_next`).
- **Variables and parameters:** `snake_case`.
- **Private or implementation-detail members:** trailing underscore, for example `clients_`, `listen_socket_`.
- **Compile-time constants in class scope:** `PascalCase` for `static constexpr` names where they read as constants (for example `Backlog`).
- **Files:** lowercase, no separators, stem matches the primary type when practical (`clienthub.cpp` / `clienthub.h`, `connectionacceptor.h`).

## Headers

- Use `#pragma once` as the include guard mechanism.
- Include order: paired header first, then a blank line, then system/library headers, then other project headers. Group with blank lines when it improves readability.
- Prefer quoted includes for project headers (`#include "clienthub.h"`).

## Formatting

- **Indentation:** tab for block indent; align continuation lines in a way consistent with surrounding files.
- **Braces:** opening brace on its own line after the function or control statement (Allman / BSD style).
- **Line length:** no hard limit in tooling yet; avoid very long lines when a natural break improves readability.

## Types and safety

- Prefer `std::` facilities from the standard library over ad-hoc equivalents when they fit.
- Use `noexcept` where the operation is guaranteed not to throw and callers rely on that contract.
- Document non-obvious behavior or cross-thread contracts with brief comments; use `/** ... */` for short API notes on public types when it helps (as elsewhere in the tree).

## Commits and reviews

- Write commit messages in clear, complete sentences (imperative mood is fine: “Add …”, “Fix …”).
- Keep changes scoped to one concern when possible; unrelated refactors belong in separate commits or PRs.

If these rules conflict with a small local pattern in a file you are editing, prefer consistency **within that file** first, then open a follow-up to normalize if needed.
