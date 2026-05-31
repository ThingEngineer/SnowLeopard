---
name: release-doc-check
description: "Run pre-release and pre-PR validation for SnowLeopard firmware changes. Use when preparing to merge, open a pull request, or cut a release. Verifies PlatformIO build success and checks for API, architecture, and history documentation drift based on changed files."
argument-hint: "Optional scope: API only, history only, full check"
user-invocable: true
disable-model-invocation: false
---

# Release Doc Check

Run a repeatable quality gate before merge/release to ensure firmware behavior and docs stay aligned.

## When To Use

- Before opening a PR.
- Before merging behavior changes.
- Before publishing release notes.
- After touching routes, payloads, auth boundaries, control logic, history logic, or hardware behavior.

## Inputs

- Optional scope hint:
  - API only
  - history only
  - full check (default)

## Procedure

1. Identify changed files.
2. Run build validation:
   - `pio run --environment esp32-c3-devkitm-1`
3. Map changed files to required documentation checks.
4. Verify doc content matches runtime behavior.
5. Produce a short report with pass/fail and exact follow-up edits.

## Decision Rules

### A) Always required

- Build must pass with PlatformIO command above.

### B) If API or route surface changed

Trigger files include:

- [src/web_routes.cpp](../../../src/web_routes.cpp)
- [include/web_routes.h](../../../include/web_routes.h)
- [src/json_builders.cpp](../../../src/json_builders.cpp)
- [include/json_builders.h](../../../include/json_builders.h)
- [src/settings_request.cpp](../../../src/settings_request.cpp)
- [include/settings_request.h](../../../include/settings_request.h)
- [src/settings_auth.cpp](../../../src/settings_auth.cpp)

Then validate:

- [docs/API-REFERENCE.md](../../../docs/API-REFERENCE.md) has correct route path, method, auth, fields, and units.
- Settings auth boundary remains intentional unless explicitly changed.

### C) If history behavior changed

Trigger files include:

- [src/history_store.cpp](../../../src/history_store.cpp)
- [include/history_store.h](../../../include/history_store.h)

Then validate:

- [docs/history.md](../../../docs/history.md) matches retention, sampling, API behavior, and discontinuity semantics.
- If architecture implications changed, update [docs/ARCHITECTURE.md](../../../docs/ARCHITECTURE.md).

### D) If module boundaries or runtime orchestration changed

Trigger files include:

- [src/main.cpp](../../../src/main.cpp)
- Any new or heavily modified file under [src](../../../src) or [include](../../../include)

Then validate:

- [docs/ARCHITECTURE.md](../../../docs/ARCHITECTURE.md) module inventory and responsibilities remain accurate.

### E) If operator-visible behavior changed

Trigger examples:

- Settings UX contract changes
- Provisioning or auth flow changes
- Alarm behavior changes

Then consider updating:

- [docs/USER-GUIDE.md](../../../docs/USER-GUIDE.md)
- [docs/QUICKSTART.md](../../../docs/QUICKSTART.md)
- [docs/PRD.md](../../../docs/PRD.md) when acceptance criteria are affected

## Completion Criteria

- Build status: pass.
- Required docs were either updated or explicitly confirmed unchanged with justification.
- Report includes:
  - Build result
  - Files inspected
  - Docs updated (or rationale for no update)
  - Remaining risks or TODOs

## Output Format

Return a concise checklist:

1. Build: pass or fail
2. API docs sync: pass or fail or not-applicable
3. History docs sync: pass or fail or not-applicable
4. Architecture docs sync: pass or fail or not-applicable
5. Operator docs sync: pass or fail or not-applicable
6. Required next edits before merge

## References

- [AGENTS.md](../../../AGENTS.md)
- [docs/ARCHITECTURE.md](../../../docs/ARCHITECTURE.md)
- [docs/API-REFERENCE.md](../../../docs/API-REFERENCE.md)
- [docs/history.md](../../../docs/history.md)
- [docs/USER-GUIDE.md](../../../docs/USER-GUIDE.md)
- [docs/QUICKSTART.md](../../../docs/QUICKSTART.md)
- [docs/PRD.md](../../../docs/PRD.md)
