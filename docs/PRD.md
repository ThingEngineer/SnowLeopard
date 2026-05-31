# SnowLeopard Product Requirements Document (PRD)

## 1. Product Summary

SnowLeopard is a local-first refrigeration controller that prioritizes reliability, clear operator control, and low-friction setup.

It combines:

- Embedded relay control with sensor-based hysteresis
- On-device display feedback
- Browser-based configuration and monitoring
- Historical trend visibility for tuning and diagnostics

## 2. Goals

1. Keep controlled chamber temperatures stable and safe.
2. Keep cooling functional even when Wi-Fi connectivity is unavailable.
3. Allow non-developer operators to provision and tune behavior from a browser.
4. Preserve enough history to diagnose cycling and drift patterns.

## 3. Personas

- Operator: manages setpoint and mode from local web pages.
- Builder/Integrator: wires hardware and configures deployment.
- Maintainer/Developer: validates firmware behavior and extends features.

## 4. Use Cases

- Refrigerator/freezer retrofit control
- Beverage cooler automation
- Small lab cold storage stabilization
- Field install where internet/cloud access is unavailable

## 5. Functional Requirements

### FR-1: Provisioning and Connectivity

- Device shall provide AP provisioning mode for first-time setup.
- Device shall save STA credentials and attempt STA reconnect on boot.
- Device shall fall back to AP mode if STA remains disconnected past timeout.
- Device shall expose mDNS host snowleopard.local in STA mode.

### FR-2: Cooling Control

- Device shall control a single relay output for compressor control.
- Device shall support relay modes: AUTO, MANUAL_ON, MANUAL_OFF.
- In AUTO, device shall use setpoint with configurable on/off deltas.
- Device shall enforce minimum compressor off-time lockout.
- Device shall force relay OFF when internal sensor data is invalid.

### FR-3: Wi-Fi-Loss Safe Cooling Behavior

- When in provisioning/AP fallback state, controller shall still run AUTO cooling.
- Effective provisioning/AP setpoint rule:
  - Use saved setpoint only when in 0 C to 8 C
  - Otherwise use fallback 4 C

### FR-4: Local UX

- Device shall provide OLED runtime status visibility.
- Device shall provide browser pages for status, settings, and history.
- Settings shall include unit, relay mode, setpoint, alarm thresholds, on/off deltas, and lockout.
- Device shall support optional password protection for Settings and Settings-changing actions while keeping Status and History open.
- Disabling Settings password protection shall clear the saved password.
- Device shall support a physical password-reset gesture (hold both buttons, then Down twice within 5 seconds) and show `PW Cleared` on OLED for 3 seconds after reset.

### FR-6: Temperature Threshold Audible Alarm

- Device shall support configurable low and high internal-temperature alarm thresholds.
- Device shall support enabling/disabling audible alarm output from Settings.
- Device shall provide a one-button alarm test action in Settings for installation checks.
- Alarm threshold inputs shall be whole numbers in the active display unit.
- Device shall drive a piezo buzzer warning tone when internal temperature is below low threshold or above high threshold.
- Alarm tone shall be noticeable and use an alternating high/low cadence.
- While active, OLED shall switch to a high-visibility full-screen alarm view showing both setpoint and current internal temperature together.
- Alarm shall stop automatically once internal temperature returns within configured bounds.

### FR-5: History and Diagnostics

- Device shall record live high-resolution history in RAM.
- Device shall persist snapshot history in NVS for reboot survivability.
- Device shall provide history API and chart page with range selection.

## 6. Non-Functional Requirements

- NFR-1: Local-first operation without cloud dependency.
- NFR-2: Bounded memory and payload behavior for history API.
- NFR-3: Flash-wear-aware persistence cadence for history snapshots.
- NFR-4: Deterministic control behavior under sensor and network faults.

## 7. Constraints

- Single-relay architecture.
- Embedded resource limits (RAM/flash/CPU).
- Sensor-driven control requires healthy internal sensor data.

## 8. Success Criteria

- Reliable setpoint hold in expected operating range.
- Smooth operator provisioning and settings flows.
- No cooling outage solely due to STA Wi-Fi disconnect.
- Usable trend history for diagnosis over recent windows.

## 9. Out of Scope (Current)

- Multi-zone/multi-relay scheduling.
- Native mobile app.
- Cloud telemetry backend.
- Long-term multi-day archival beyond current history model.

## 10. Risks and Mitigations

1. Sensor failure risk.

- Mitigation: safe-off logic and clear status visibility.

2. Wi-Fi instability.

- Mitigation: AP fallback plus safe provisioning/AP setpoint policy.

3. Flash wear from persistence.

- Mitigation: batched snapshot writes.

4. Misconfiguration by operators.

- Mitigation: input clamping and guided settings UI.

## 11. Acceptance Criteria

1. Provisioning succeeds from AP UI and credentials persist across reboot.
2. STA disconnect timeout triggers AP fallback while cooling remains active in AUTO.
3. In provisioning/AP mode, effective setpoint behavior follows 0-8 C rule with 4 C fallback.
4. Settings round-trip via UI and API correctly with clamped values.
5. History page and API provide bounded, valid trend data.
6. When internal temperature exits configured threshold bounds, audible piezo alarm activates and stops on return to range.

## 12. Related Documents

- Technical design: [ARCHITECTURE.md](ARCHITECTURE.md)
- User operations: [USER-GUIDE.md](USER-GUIDE.md)
- API contracts: [API-REFERENCE.md](API-REFERENCE.md)
- History deep dive: [history.md](history.md)
