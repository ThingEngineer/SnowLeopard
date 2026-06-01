# SnowLeopard Architecture

## 1. System Overview

SnowLeopard is an ESP32-C3 based refrigeration controller with:

- Internal and external AHT30 temperature/humidity sensors
- Compressor relay control with hysteresis and minimum off-time lockout
- Local OLED status display
- Web UI for status, settings, and history
- GitHub-hosted release manifest checks and OTA firmware update flow
- Per-sensor calibration offsets and selectable OLED display layouts
- Captive portal provisioning AP mode and STA mode with fallback handling
- Reboot-survivable history snapshots plus high-resolution live trend data

Primary orchestration lives in [../src/main.cpp](../src/main.cpp), with extracted subsystems in dedicated module files under `include/` and `src/`.

## 2. Hardware Architecture

### 2.1 MCU and Peripherals

- Board target: ESP32-C3 DevKitM-1 compatible board
- OLED: SSD1306 72x40 (software I2C on GPIO5/GPIO6)
- Internal sensor bus: GPIO8 (SDA) / GPIO9 (SCL)
- External sensor bus: GPIO5 (SDA) / GPIO6 (SCL), shared with OLED
- Relay control output: GPIO7 (active high)
- Buttons wired: GPIO2 (Up), GPIO3 (Down), input pullups
- Piezo alarm output: GPIO4 (PWM tone output)

See [pinout.md](pinout.md) and [wiring-diagram.mmd](wiring-diagram.mmd).
See [application-block-diagram.mmd](application-block-diagram.mmd) for the runtime block view.

### 2.2 Relay Contact Philosophy

Recommended compressor contact wiring uses COM + NO for fail-safe OFF on controller power loss.

## 3. Software Architecture

### 3.1 Core Runtime Model

Firmware uses a single main loop with cooperative timing plus AsyncWebServer callbacks.

Main loop responsibilities:

- DNS captive processing while in provisioning mode
- Pending reboot handling after credential save
- STA disconnect tracking and AP fallback transition
- Sensor read/control/log pipeline
- Periodic history persistence
- OLED updates

### 3.2 Networking States

SnowLeopard has two network operating states:

1. Provisioning AP mode

- AP SSID: SnowLeopard
- Captive portal and probe endpoints active
- Root route serves provisioning UI
- Settings/history pages are blocked and redirected to root

2. STA mode

- Connects to saved SSID/password
- mDNS host: snowleopard.local
- Status/settings/history routes available

Fallback behavior:

- If STA disconnect persists past timeout, device enters AP fallback path.
- Cooling remains active under AUTO logic while in provisioning/AP mode.
- Provisioning/AP mode uses an effective setpoint policy:
  - Use saved setpoint only when it is within 0 C to 8 C
  - Otherwise use 4 C

### 3.3 Control Algorithm

Relay modes:

- AUTO
- MANUAL_ON
- MANUAL_OFF

AUTO control behavior:

- Relay ON when internal temp >= setpoint + on_delta and lockout expired
- Relay OFF when internal temp <= setpoint - off_delta
- Relay forced OFF if internal sensor is invalid

Safety and guardrails:

- Setpoint clamped to firmware hard range (-50 C to 50 C)
- Delta values clamped to configured min/max
- Minimum off-time lockout enforced

### 3.4 Data and Persistence

NVS namespace stores:

- Temperature unit preference
- Relay mode
- OLED layout mode
- Setpoint
- Alarm low/high thresholds
- Settings auth enable flag and password
- Relay on/off deltas
- Internal and external temperature calibration offsets
- Lockout duration
- Wi-Fi credentials
- History boot sequence counter
- History snapshot header and snapshot buffer blobs

History strategy:

- Live ring buffer: 10-second samples, high resolution
- Snapshot ring buffer: 60-minute samples, reboot-survivable
- Snapshot ring capacity: 48 samples, about 48 hours retained internally
- Snapshot writes batched periodically to reduce flash wear
- HTTP history requests are capped to 24 hours even when older coarse snapshot data exists

For detailed history internals, see [history.md](history.md).

### 3.5 Web UI and API Surfaces

UI pages:

- / (status in STA, provisioning page in AP)
- /settings
- /history

Repository-hosted release surfaces:

- `release-data/current.json` for latest OTA manifest data
- `release-data/releases/*.md` for release notes
- `portal/` GitHub Pages app for the public release site

APIs:

- /api/status
- /api/settings (GET/POST)
- /api/settings_login (POST)
- /api/history
- /api/firmware
- /api/firmware_check
- /api/firmware_update
- /api/provision
- /api/reconfigure
- /api/alarm_test

Captive probe endpoints are implemented for major OS behaviors.

Detailed schemas are in [API-REFERENCE.md](API-REFERENCE.md).

### 3.6 Temperature Threshold Alarm

Alarm trigger rule:

- Alarm is active only when internal temperature is valid and either:
  - internal temp < low threshold, or
  - internal temp > high threshold.

Alarm output behavior:

- Piezo output is generated on GPIO4 using LEDC PWM tone generation.
- Pattern is alternating high/low tones in short bursts with a brief pause for noticeability.
- A manual short alarm test cycle can be triggered from Settings for installation checks.
- Alarm tone stops immediately when temperature returns within configured limits.

Settings behavior:

- Audible alarm enable state is persisted in NVS.
- Low and high alarm thresholds are persisted in NVS in Celsius.
- UI/API expose thresholds in active display unit (C/F).
- Threshold inputs are normalized to whole-number display values.
- Sensor calibration offsets are persisted in Fahrenheit and converted in the UI when Celsius is selected.

### 3.7 Module Layout

The firmware was refactored from a monolithic implementation into behavior-preserving modules.

Current extracted modules include:

- `temperature`: unit conversion and range clamping helpers
- `settings_store`: persisted settings load/save wrappers around Preferences
- `settings_auth`: settings session token/cookie auth helpers
- `password_reset_gesture`: hidden physical password-reset gesture state machine
- `button_input`: debounce and press-repeat handling for buttons
- `json_builders`: `/api/status` and settings payload JSON construction
- `settings_request`: `/api/settings` POST request parsing/validation helpers
- `settings_parse`: relay mode and OLED layout parsing/mapping helpers
- `settings_apply`: settings mutation/apply/persist workflow helpers
- `web_routes`: shared HTTP route registration helpers
- `device_types`: shared firmware enums for relay mode, OLED layout mode, and active I2C bus selection
- `network_workflow`: STA connect retry and AP provisioning transition workflow
- `network_helpers`: captive-host checks, active IP selection, and STA credential persistence helpers
- `i2c_bus`: low-level I2C bus selection and address scanning helpers shared by sensor init paths
- `sensor_io`: AHT sensor init/read helpers with callback-based I2C bus selection and scan hooks
- `sensor_pipeline`: periodic sensor-read/init retry, control apply trigger, and history sample pipeline wiring
- `setpoint_adjust`: setpoint button press/repeat handling, persistence trigger, and immediate control refresh hook
- `relay_control`: relay decision evaluation for manual/auto modes, thresholds, and lockout gating
- `display_renderer`: OLED rendering pipeline for runtime/provisioning/alarm pages
- `web_pages`: embedded HTML/CSS/JS for status, settings, history, and provisioning pages
- `alarm_beeper`: temperature-threshold alarm evaluation and piezo beep pattern state machine
- `firmware_update`: GitHub-hosted manifest fetch, version comparison, OTA queueing, and update status JSON
- `history_store`: history snapshot persistence and blended history query assembly

`setupWeb()` in `main.cpp` is now primarily orchestration that wires route registrations to callbacks.

## 4. Authentication and Security Posture

- Settings password protection is optional and session-cookie based.
- Root `/` and `/history` remain open; protected operations are limited to Settings-related routes.
- Protected routes when enabled include `/settings`, `/api/settings`, `/api/reconfigure`, `/api/alarm_test`, `/api/firmware`, `/api/firmware_check`, and `/api/firmware_update`.
- `/api/settings_login` is used to establish the settings session cookie.
- Turning settings protection off clears the saved password.
- Provisioning AP is open by default in current configuration.

Operational guidance:

- Use network segmentation/VLANs when exposing device on broader networks.
- If deploying in less trusted environments, enable Settings password protection and secure AP policy in firmware.

## 5. Display Model

OLED output paths:

- Provisioning mode: SSID + configure prompt
- Temporary STA info splash after successful connect
- Normal runtime page with setpoint/internal/external/relay state
- Selectable alternate layouts for setpoint-only, setpoint+internal, setpoint+internal+relay, and internal+external views
- Alarm mode: full-screen high-visibility showing both `Set <setpoint>` and `Int <current internal>` simultaneously
- Setpoint adjustment view has priority over alarm view while physical buttons are actively adjusting setpoint

## 6. Limits and Constraints

- Single compressor relay control path
- Internal sensor validity is required for AUTO cooling decisions
- History window and point count are server-bounded
- No external cloud dependency required
- OTA uses GitHub-hosted files and release assets, but no database or separate backend service

## 7. Build and Deployment

Build system: PlatformIO.

Typical commands:

- Build: pio run --environment esp32-c3-devkitm-1
- Flash: pio run --environment esp32-c3-devkitm-1 --target upload
- Portal build: `cd portal && npm run build`

## 8. Verification Checklist

After firmware changes, verify:

1. STA connect and AP fallback transitions.
2. Relay behavior in AUTO/manual modes.
3. Settings save/load and unit conversion.
4. History API and chart rendering.
5. Provisioning workflow and reboot path.

## 9. Cross-References

- Product requirements: [PRD.md](PRD.md)
- User operations: [USER-GUIDE.md](USER-GUIDE.md)
- API details: [API-REFERENCE.md](API-REFERENCE.md)
- History internals: [history.md](history.md)
