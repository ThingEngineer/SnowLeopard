---
name: API Contract Sync
description: "Use when adding, removing, or changing HTTP routes, auth protection, request parsing, or JSON response fields in SnowLeopard. Enforces code-doc contract sync for API and web route work."
applyTo: src/web_routes.cpp, src/json_builders.cpp, include/web_routes.h, include/json_builders.h, src/settings_request.cpp, include/settings_request.h, docs/API-REFERENCE.md
---

# API Contract Sync

- Keep runtime behavior, headers, and docs aligned in the same change when API surface changes.
- If route path, method, auth requirements, request fields, or response fields change, update [docs/API-REFERENCE.md](../../docs/API-REFERENCE.md).
- When adding/removing protected settings endpoints, keep behavior consistent with [src/settings_auth.cpp](../../src/settings_auth.cpp) and update auth notes in [docs/API-REFERENCE.md](../../docs/API-REFERENCE.md).
- Preserve established access boundaries unless explicitly requested:
  - Open pages/status routes remain open.
  - Settings operations are protected when settings auth is enabled.
- For status/settings payload changes, update builders and parsing together:
  - [src/json_builders.cpp](../../src/json_builders.cpp)
  - [src/settings_request.cpp](../../src/settings_request.cpp)
  - [src/settings_apply.cpp](../../src/settings_apply.cpp)
- Do not ship undocumented API changes. If behavior intentionally diverges from existing docs, document the new behavior in the same PR.

## Quick Verification

1. Build: `pio run --environment esp32-c3-devkitm-1`
2. Confirm changed endpoints still match documented path/method/auth requirements.
3. Confirm documented request/response examples reflect current field names and units.
