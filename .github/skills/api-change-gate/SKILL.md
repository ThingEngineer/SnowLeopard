---
name: api-change-gate
description: "Run a strict API contract gate before PR merge. Use when routes, auth protection, request parsing, or JSON response fields changed and you need an explicit pass/fail report with required doc updates."
argument-hint: "Optional mode: quick or strict"
user-invocable: true
disable-model-invocation: false
---

# API Change Gate

Run a repeatable contract check for SnowLeopard API changes so code, auth boundaries, and docs stay in sync.

## When To Use

- Before opening a PR that touches HTTP/API behavior.
- Before merging changes to route paths/methods/auth or payload fields.
- After edits in routing, JSON builders, or settings request parsing.

## Inputs

- Optional mode:
  - `quick`: checks changed files and required docs only
  - `strict` (default): checks changed files, docs, and boundary assumptions

## Procedure

1. Identify changed files:
   - `git status --short`
   - `git diff --name-only`
2. If any API surface file changed, enforce contract sync:
   - [src/web_routes.cpp](../../../src/web_routes.cpp)
   - [include/web_routes.h](../../../include/web_routes.h)
   - [src/json_builders.cpp](../../../src/json_builders.cpp)
   - [include/json_builders.h](../../../include/json_builders.h)
   - [src/settings_request.cpp](../../../src/settings_request.cpp)
   - [include/settings_request.h](../../../include/settings_request.h)
3. Verify auth boundary consistency with:
   - [src/settings_auth.cpp](../../../src/settings_auth.cpp)
   - [docs/API-REFERENCE.md](../../../docs/API-REFERENCE.md)
4. Confirm docs are updated when behavior changed:
   - [docs/API-REFERENCE.md](../../../docs/API-REFERENCE.md)
5. Run build validation:
   - `pio run --environment esp32-c3-devkitm-1`
6. Return an explicit gate report.

## Decision Rules

- If route path, method, auth requirement, request field, or response field changes, docs update is mandatory in the same change.
- Preserve intentional access boundaries unless explicitly requested:
  - Open: `/`, `/history`, `/api/status`, `/api/history`
  - Settings operations remain protected when settings auth is enabled
- Do not mark gate as pass if API behavior changed but docs remain stale.

## Output Format

Return exactly this checklist:

1. API files changed: yes or no
2. Auth boundary check: pass or fail
3. API reference sync: pass or fail
4. Build (`pio run`): pass or fail
5. Gate result: pass or fail
6. Required fixes before merge

## References

- [AGENTS.md](../../../AGENTS.md)
- [.github/instructions/api-contract-sync.instructions.md](../../../.github/instructions/api-contract-sync.instructions.md)
- [docs/API-REFERENCE.md](../../../docs/API-REFERENCE.md)
- [src/settings_auth.cpp](../../../src/settings_auth.cpp)
