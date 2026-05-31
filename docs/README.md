# SnowLeopard Documentation

This folder contains technical, product, and operator documentation for the SnowLeopard controller.

## Start Here

- Fast setup: [QUICKSTART.md](QUICKSTART.md)
- New operator: [USER-GUIDE.md](USER-GUIDE.md)
- Product/stakeholder view: [PRD.md](PRD.md)
- Firmware/developer view: [ARCHITECTURE.md](ARCHITECTURE.md)
- API clients/integrations: [API-REFERENCE.md](API-REFERENCE.md)
- Release operators: [RELEASE-WORKFLOW.md](RELEASE-WORKFLOW.md)

## Existing Hardware and Feature References

- History subsystem deep dive: [history.md](history.md)
- Pin mapping: [pinout.md](pinout.md)
- Application block diagram: [application-block-diagram.mmd](application-block-diagram.mmd)
- Mermaid wiring diagram: [wiring-diagram.mmd](wiring-diagram.mmd)

## Documentation Set

- [QUICKSTART.md](QUICKSTART.md): rapid setup and first-run validation.
- [ARCHITECTURE.md](ARCHITECTURE.md): full technical architecture and runtime behavior.
- [PRD.md](PRD.md): product requirements and acceptance criteria.
- [USER-GUIDE.md](USER-GUIDE.md): complete, user-friendly operational manual.
- [API-REFERENCE.md](API-REFERENCE.md): HTTP endpoints, request/response fields, and examples.
- [RELEASE-WORKFLOW.md](RELEASE-WORKFLOW.md): exact single-repo release steps for OTA metadata, GitHub Releases, and GitHub Pages.

## Scope Note

These docs describe currently implemented behavior in firmware, including Wi-Fi provisioning/fallback, control logic, audible temperature alarm behavior, settings password protection and reset, sensor calibration offsets, OLED layout selection, status UI, history, OTA metadata checks, and the GitHub-hosted release portal.

Implementation note:

- Runtime orchestration remains in `src/main.cpp`.
- Core behaviors have been incrementally extracted into focused modules under `include/` and `src/`.
- See [ARCHITECTURE.md](ARCHITECTURE.md) for the current module inventory and responsibilities.
