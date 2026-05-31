# SnowLeopard API Reference

All endpoints are served by the embedded web server on the device.

Base URL examples:

- http://snowleopard.local (STA mode with mDNS)
- http://<device-ip>

## Auth

- Settings protection uses a password gate stored in firmware preferences.
- When enabled, `/settings`, `/api/settings`, `/api/reconfigure`, and `/api/alarm_test` require the settings session cookie.
- `/api/settings_login` is used to create that settings session cookie.
- The Status page (`/`) and History page (`/history`) remain open.
- Turning Settings auth off clears the saved password.

Settings login flow:

- `POST /api/settings_login` with `password`
- On success, the server returns a session cookie for Settings access

## Content Types

- Most API responses: application/json
- Settings POST body: application/x-www-form-urlencoded

## 1) GET /api/status

Returns live status for dashboard display.

Example response fields:

- temp_unit: "C" or "F"
- internal_temp: number or null
- internal_humidity: number or null
- external_temp: number or null
- external_humidity: number or null
- relay_state: "ON" or "OFF"
- relay_mode: mode label text
- relay_mode_id: numeric mode id
- alarm_enabled: boolean (audible alarm setting)
- alarm_active: boolean (internal temperature currently outside alarm thresholds)
- set_temp: numeric display-unit value
- relay_on_delta: numeric display-unit delta
- relay_off_delta: numeric display-unit delta
- min_off_seconds: integer seconds

## 2) GET /api/settings

Returns currently active configurable settings.

Response fields:

- temp_unit
- relay_mode
- oled_layout
- set_temp
- settings_auth_enabled
- alarm_enabled
- alarm_low
- alarm_high
- relay_on_delta
- relay_off_delta
- min_off_seconds

## 3) POST /api/settings

Updates settings.

Required form fields:

- temp_unit: C or F
- relay_mode: auto | manual_on | manual_off
- set_temp: number in posted unit

Optional form fields:

- oled_layout: standard | setpoint_only | setpoint_internal | setpoint_internal_relay | internal_external
- settings_auth_enabled: 1/0 or true/false
- settings_auth_password: new password text
- settings_auth_password_confirm: confirmation for the new password
- alarm_enabled: 1/0 or true/false
- alarm_low: whole-number low threshold in posted unit
- alarm_high: whole-number high threshold in posted unit
- relay_on_delta: number in posted unit
- relay_off_delta: number in posted unit
- min_off_seconds: integer seconds

Behavior:

- If settings auth is enabled and both password fields are blank, the existing password is kept when re-saving an existing protected configuration.
- If settings auth is disabled, the saved password is cleared.
- If settings auth is being enabled for the first time, a new password is required.

Validation behavior:

- unit must be C or F
- relay_mode must be supported value
- non-finite temperature values rejected
- values clamped by firmware guardrails

Error responses include JSON with ok:false and error code, for example:

- missing_settings
- invalid_unit
- invalid_mode
- invalid_temperature_range
- invalid_alarm_range
- invalid_oled_layout
- auth_required
- password_required
- password_mismatch

Success response includes:

- ok:true
- normalized, active setting fields in current display unit

## 3b) POST /api/alarm_test

Triggers a short audible/visual alarm test cycle for installation checks.

Behavior:

- Available in STA mode
- Starts a ~3 second piezo/OLED alarm cycle regardless of current temperature

## 3c) POST /api/settings_login

Creates a settings-auth session when settings password protection is enabled.

Expected form fields:

- password

Behavior:

- If settings auth is disabled, returns success and clears any existing settings auth cookie.
- If settings auth is enabled and password matches, returns success and sets the settings auth cookie.
- If password is wrong, returns `401` with `invalid_password`.

Error responses include JSON with ok:false and error code, for example:

- invalid_password

## 4) GET /api/history

Returns blended history series data.

Query parameters:

- window_s (optional): requested window size in seconds
- step_s (optional): requested step size in seconds
- include_external (optional): 1/0 or true/false

Response fields:

- ok
- source (blended)
- temp_unit
- period_s
- requested_window_s
- effective_step_s
- window_s
- retention_s
- age_s (seconds from now)
- internal (series)
- relay (0/1 series)
- external (series or null)
- discontinuity (0/1 series, where 1 marks reboot/session boundary in persisted summary history)
- discontinuity_age_s (age values in seconds where restart/session boundaries occur)
- discontinuity_count (number of detected restart/session boundaries in returned retention)

Server behavior:

- uses live history for recent ages and snapshot history for older ages
- down-samples to bounded point count
- caps returned window to available retention; compare requested_window_s vs window_s

## 5) POST /api/provision

Provisioning-mode credential save endpoint.

Expected form fields:

- ssid
- password

Behavior:

- only valid while in provisioning mode
- saves credentials and triggers reboot path

Potential errors:

- not_in_provisioning_mode
- invalid_wifi_credentials

## 6) POST /api/reconfigure

Switches running device back to provisioning AP mode.

Behavior:

- if already provisioning, returns ok with mode=provisioning
- otherwise transitions into AP mode

## 7) UI Routes (non-API)

- / : provisioning page in AP mode, status page in STA mode
- /settings : settings UI (STA mode). Shows a login page when settings password protection is enabled.
- /history : history UI (STA mode)

## 8) Captive Probe Routes

Multiple OS-specific probe endpoints are implemented and redirected/served during provisioning to support captive portal behavior.

## 9) Example Requests

Get status:

curl "http://snowleopard.local/api/status"

Get settings:

curl "http://snowleopard.local/api/settings"

Update settings:

curl -X POST "http://snowleopard.local/api/settings" \
 -H "Content-Type: application/x-www-form-urlencoded" \
 --data "temp_unit=C&relay_mode=auto&set_temp=4&alarm_enabled=1&alarm_low=1&alarm_high=8&relay_on_delta=1.0&relay_off_delta=0.5&min_off_seconds=180"

Get 1-hour history:

curl "http://snowleopard.local/api/history?window_s=3600&include_external=1"

## 10) Versioning Note

This API is currently unversioned and firmware-coupled. Keep docs aligned with route registration/helpers in [../src/main.cpp](../src/main.cpp) and [../src/web_routes.cpp](../src/web_routes.cpp) after firmware changes.
