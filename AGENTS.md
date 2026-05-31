# AGENTS.md

## Purpose

Instructions for AI coding agents working in this SnowLeopard firmware repo.

## Project Snapshot

- Target: ESP32-C3 Arduino firmware (PlatformIO).
- Entry point/orchestration: [src/main.cpp](src/main.cpp).
- Architecture and module responsibilities: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).
- API contracts: [docs/API-REFERENCE.md](docs/API-REFERENCE.md).
- History subsystem behavior/details: [docs/history.md](docs/history.md).
- Product and acceptance constraints: [docs/PRD.md](docs/PRD.md).

## Build and Run

Use PlatformIO commands from repo root:

- Build: `pio run --environment esp32-c3-devkitm-1`
- Flash: `pio run --environment esp32-c3-devkitm-1 --target upload`
- Serial monitor: `pio device monitor --baud 115200`
- Portal build (from `portal/`): `npm run build`
- Portal lint (from `portal/`): `npm run lint`

Notes:

- Prefer these `pio` commands over unrelated workspace tasks.
- This repo currently has no meaningful host-side unit test suite under [test/README](test/README).

## Instruction Files

- API contract sync rules live in [.github/instructions/api-contract-sync.instructions.md](.github/instructions/api-contract-sync.instructions.md).
- When changing routes, auth protection, request parsing, or JSON fields, update all files covered by that instruction in the same change.

## Repo Skills

- OTA release automation: [.github/skills/publish-ota-release/SKILL.md](.github/skills/publish-ota-release/SKILL.md)
- Pre-PR/release validation and docs drift check: [.github/skills/release-doc-check/SKILL.md](.github/skills/release-doc-check/SKILL.md)
- Split mixed diffs into scoped commits: [.github/skills/split-logical-commits/SKILL.md](.github/skills/split-logical-commits/SKILL.md)

## Where to Change Code

- Keep `main.cpp` as orchestration glue; prefer implementing behavior in focused modules under [include](include) and [src](src).
- For HTTP/API changes, update route wiring and JSON builders consistently:
  - [src/web_routes.cpp](src/web_routes.cpp)
  - [src/json_builders.cpp](src/json_builders.cpp)
  - [docs/API-REFERENCE.md](docs/API-REFERENCE.md)
- For settings/auth changes, preserve cookie/session behavior and protected-route scope documented in:
  - [src/settings_auth.cpp](src/settings_auth.cpp)
  - [docs/API-REFERENCE.md](docs/API-REFERENCE.md)

## Project-Specific Guardrails

- History sizing/source-of-truth: validate effective history behavior in [src/history_store.cpp](src/history_store.cpp) before changing constants elsewhere. `main.cpp` may contain stale duplicate history constants.
- I2C on ESP32-C3: use one I2C controller (`Wire`) and remap pins as implemented in [src/i2c_bus.cpp](src/i2c_bus.cpp); avoid introducing `TwoWire(1)` for this target.
- Settings auth boundaries are intentional: root/status/history remain open while settings operations are gated. Do not broaden or narrow access without updating docs and behavior together.
- Keep temperature/setpoint/alarm normalization behavior compatible with existing clamping and whole-display-unit rounding logic in [src/main.cpp](src/main.cpp) and settings modules.

## Change Validation Checklist

For most code changes:

1. Build with `pio run --environment esp32-c3-devkitm-1`.
2. If API/settings/history logic changed, verify docs stay aligned:
   - [docs/API-REFERENCE.md](docs/API-REFERENCE.md)
   - [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)
   - [docs/history.md](docs/history.md)
3. If hardware-facing logic changed (I2C, sensors, relay, alarm), prefer conservative edits and preserve existing fallback/safety paths.
4. If `release-data/` or `portal/` changed, run `npm run build` from [portal](portal) so `portal/public/release-data` is synced by the prebuild hook.

## Documentation Map

- Operator setup and troubleshooting: [docs/QUICKSTART.md](docs/QUICKSTART.md), [docs/USER-GUIDE.md](docs/USER-GUIDE.md)
- System overview and module inventory: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)
- API details and auth flow: [docs/API-REFERENCE.md](docs/API-REFERENCE.md)
- History storage and blending model: [docs/history.md](docs/history.md)
