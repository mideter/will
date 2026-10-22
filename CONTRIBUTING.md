# Contributing to Will

This document records project conventions so changes stay consistent. When in doubt, match existing code in `src/`.

## Language and toolchain

- C++ standard and compiler flags are defined in `xmake.lua`
- Build and run: see [README.md](README.md).

## Layout and modules

- Library and binary sources live under `src/`
- Public headers shipped or included across targets should stay minimal; prefer implementation detail in `.cpp` files.

## Naming

- **Namespace:** `will` for project code.
- **Types (classes, structs, enums, type aliases):** `PascalCase` (for example `SessionRegistry`, `ConnectionAcceptor`).
- **Functions and methods:** `snake_case` (for example `listen_fd`, `accept_next`).
- **Variables and parameters:** `snake_case`.
- **Private or implementation-detail members:** trailing underscore, for example `clients_`, `listen_socket_`.
- **Compile-time constants in class scope:** `PascalCase` for `static constexpr` names where they read as constants (for example `Backlog`).
- **Files:** lowercase, no separators, stem matches the primary type when practical (`sessionregistry.cpp` / `sessionregistry.h`, `connectionacceptor.h`).

## Headers

- Use `#pragma once` as the include guard mechanism.
- Include order: paired header first, then a blank line, then system/library headers, then other project headers. Group with blank lines when it improves readability.
- Prefer quoted includes for project headers (`#include "sessionregistry.h"`).

## Formatting

- **Indentation:** tabs for block indent (display width 4). Continuation lines keep the same tab indent as the enclosing block, then use spaces only for visual alignment under the previous token.
- **Braces:** opening brace on its own line after the function or control statement (Allman / BSD style).
- **Line length:** no hard limit in tooling yet; avoid very long lines when a natural break improves readability.
- **Blank lines:** at most two in a row.

## Types and safety

- Prefer `std::` facilities from the standard library over ad-hoc equivalents when they fit.
- Use `noexcept` where the operation is guaranteed not to throw and callers rely on that contract.

## Comments

A comment says only what the code cannot say itself; the meaning goes into names first.

- **Where:** above a class (in Kotlin, also above a Compose screen) — what it is and its rules; inside a body — one line on why, beside a decision that neither a name nor a test can tell. Never above a method, a function or a field: what a comment there would say goes into the name, into the class's comment as its rule, or into a test.
- **Keep:** a rule or intent the signatures do not show (`exercises in any order, approaches of an exercise in order`); cross-thread contracts — in the class's comment.
- **Names:** the Russian name of a domain class and its word in the interface live in `GLOSSARY.md`, not in comments; a new or renamed class is entered there.
- **Do not write:** retellings of a signature or a name; lists of what a function throws (the refusal classes in `src/domain/refusals/` say it); old or foreign names — a comment is neither compiled nor tested, and a misleading one is worse than none.
- When a rule changes, check the comment beside it.

## Commits and reviews

- Write commit messages in clear, complete sentences (imperative mood is fine: “Add …”, “Fix …”).
- Keep changes scoped to one concern when possible; unrelated refactors belong in separate commits or PRs.
