---
name: Release Run
description: "One-command SnowLeopard release run via the Release Operator agent. Use for prep, publish, or verify phases with a prefilled phase/version template."
argument-hint: "Optional overrides: phase=<prep|publish|verify> version=<X.Y.Z>"
agent: "Release Operator"
---

Run a SnowLeopard release operation using this default template:

- phase: prep
- version: 0.1.2

If the user provided overrides in the prompt argument, apply them using:

- `phase=<prep|publish|verify>`
- `version=<X.Y.Z>`

Then execute the release workflow for the selected phase and return the standard release report:

1. Phase
2. Version target
3. Firmware build status
4. Portal build status (if applicable)
5. Release metadata alignment
6. Tag/workflow status (if applicable)
7. OTA smoke checks (if applicable)
8. Required follow-ups
