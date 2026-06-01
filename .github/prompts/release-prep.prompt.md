---
name: Release Prep
description: "One-command SnowLeopard release prep run via the Release Operator agent with a prefilled version template."
argument-hint: "Optional override: version=<X.Y.Z>"
agent: "Release Operator"
---

Run SnowLeopard release preparation with this fixed template:

- phase: prep
- version: 0.1.2

If the user provided an override, apply:

- `version=<X.Y.Z>`

Then execute the prep workflow and return the standard release report:

1. Phase
2. Version target
3. Firmware build status
4. Portal build status (if applicable)
5. Release metadata alignment
6. Tag/workflow status (if applicable)
7. OTA smoke checks (if applicable)
8. Required follow-ups
