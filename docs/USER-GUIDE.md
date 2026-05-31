# SnowLeopard User Guide

This guide explains how to use every current SnowLeopard feature in day-to-day operation.

## 1. What SnowLeopard Does

SnowLeopard controls a refrigeration relay from internal temperature readings and lets you monitor/tune behavior from:

- OLED screen on the controller
- Browser status page
- Browser settings page
- Browser history page

## 2. First-Time Setup

### 2.1 Power and Wiring

Before first boot, confirm wiring against:

- [pinout.md](pinout.md)
- [wiring-diagram.mmd](wiring-diagram.mmd)
- [esp32_wiring.png](esp32_wiring.png)

### 2.2 Boot and Provisioning AP

On first boot (or when no STA connection is available), the device enters provisioning AP mode.

1. Connect your phone/laptop to Wi-Fi SSID: SnowLeopard.
2. Open a browser to http://192.168.4.1 if portal does not auto-open.
3. Enter your Wi-Fi SSID/password and save.
4. Device reboots and attempts STA connection.

## 3. Accessing the Web Interface

In STA mode, open either:

- http://snowleopard.local
- or the device IP from your router/OLED splash

Pages:

- Status: /
- Settings: /settings
- History: /history

The Status and History pages are always open. The Settings page may optionally require a password.

## 4. Status Page

The Status page shows:

- Internal temperature (primary control variable)
- External temperature (if sensor available)
- Humidity values
- Relay state (ON/OFF)
- Relay mode
- Current set temperature
- Temp Alarm state when audible alarm is enabled:
  - NORMAL
  - ACTIVE (shown in red)

Use Settings button to tune behavior and History button for trend analysis.

## 5. Settings Page

### 5.1 Available Controls

- Temperature unit: C or F
- Relay mode:
  - Auto
  - Manual locked on
  - Manual locked off
- Auth:
  - Require password for Settings and changes
  - Password and confirmation fields are visible text inputs so you can verify what you typed
  - Turning the checkbox off clears the saved password immediately
- Set temperature
- Temperature threshold alarm:
  - Audible alarm enabled (on/off)
  - Low alarm threshold (whole number)
  - High alarm threshold (whole number)
  - Test alarm (3s) button for install verification
- OLED display configuration:
  - Standard layout (default)
  - Setpoint only (large)
  - Setpoint + internal temp (large)
  - Setpoint + internal temp + compressor state
  - Internal + external temp (large)
- Advanced compressor control:
  - Turn-on delta above setpoint
  - Turn-off delta below setpoint
  - Minimum compressor off time (seconds)

### 5.2 Temperature Threshold Audible Alarm

- Alarm thresholds apply to the internal temperature sensor.
- Audible alarm can be turned on/off from Settings.
- If internal temperature goes below low threshold or above high threshold, the piezo alarm beeps.
- Alarm sound pattern alternates high/low tones to be noticeable.
- Alarm stops automatically when internal temperature returns inside the configured range.
- Threshold values are whole numbers only.
- Use Test alarm (3s) to force a short beep/display cycle for installation checks.

### 5.3 How Saving Works

- Changes auto-save shortly after edits.
- UI shows save status at the bottom.
- Values are validated/clamped by firmware and persisted.

### 5.4 Reconfigure Wi-Fi

- Press Reconfigure Wi-Fi to switch into provisioning AP mode.
- You can then connect to SnowLeopard AP and enter new credentials.

### 5.5 Reset the Settings Password

If you forget the Settings password, hold both physical buttons, then press Down twice within 5 seconds.

- The password is cleared
- The OLED shows `PW Cleared` for 3 seconds
- You can then re-enable Auth and set a new password

## 6. History Page

History page features:

- Internal temperature trend line
- Optional external temperature line
- Relay ON shading overlay
- Range buttons: 5m, 15m, 30m, 1h, 3h, 6h, 12h
- Auto-refresh while page is open

Use this page to verify cycle behavior after settings changes.

For internals, see [history.md](history.md).

## 7. Cooling Behavior and Safety Rules

### 7.1 Normal AUTO Control

AUTO mode uses hysteresis thresholds around setpoint:

- Turns ON above upper threshold (setpoint + on-delta), respecting lockout
- Turns OFF below lower threshold (setpoint - off-delta)

### 7.2 Sensor Safety

If internal sensor data is invalid, relay is forced OFF.

### 7.3 Wi-Fi Loss and AP Fallback

If STA Wi-Fi is lost too long, SnowLeopard falls back to AP/provisioning mode.

Cooling still runs in AUTO while in this state using this effective setpoint policy:

- If saved setpoint is between 0 C and 8 C, use saved setpoint.
- Otherwise, use 4 C.

This prevents cooling shutdown due only to Wi-Fi loss.

## 8. OLED Screen Behavior

Provisioning/AP state:

- Shows SnowLeopard SSID and configure prompt.

Normal runtime:

- Shows setpoint, internal temp, external temp, and relay state.
- During active temperature alarm, OLED switches to full-screen high-visibility display showing both values simultaneously:
  - `Set <setpoint>`
  - `Int <current internal temp>`
- While physical setpoint buttons are being used, setpoint-adjust display takes priority.

## 9. Troubleshooting

### 9.1 Cannot Reach snowleopard.local

- Confirm device is in STA mode and connected to same LAN.
- Use direct IP from router/OLED and test that URL.
- Some networks block mDNS across VLANs/subnets.

### 9.2 Device Keeps Returning to AP Mode

- Saved Wi-Fi credentials may be wrong or network unavailable.
- Reconfigure credentials from AP portal.
- Check router band/security compatibility.

### 9.3 Relay Does Not Turn On in AUTO

- Check internal temperature relative to setpoint + on-delta.
- Check minimum off-time lockout has expired.
- Verify internal sensor is healthy; invalid internal sensor forces relay OFF.
- Verify wiring and relay power (logic side and contact side).

### 9.4 Temperature Overshoot or Rapid Cycling

- Increase minimum off-time.
- Increase on/off deltas slightly.
- Check sensor placement and airflow.

### 9.5 Alarm Beeping Unexpectedly

- Check current internal temperature vs configured low/high alarm thresholds.
- Verify the temperature unit on Settings before entering thresholds.
- If thresholds are too narrow, widen the alarm range.
- Verify internal sensor is healthy; alarm only uses internal sensor data.

### 9.6 History Appears Sparse After Reboot

- Recent buffer repopulates over time.
- Snapshot history is coarse by design; this is expected behavior.

## 10. Operating Tips

1. Start with conservative defaults (4 C setpoint, moderate deltas).
2. Change one parameter at a time.
3. Watch at least 1-2 compressor cycles on History after each change.
4. Keep compressor wiring fail-safe (COM+NO recommended).

## 11. Feature Checklist (Current)

- Provisioning captive portal AP
- STA operation with fallback to AP
- mDNS in STA mode
- Status web page
- Settings web page
- History web page
- Optional settings password protection
- Physical settings-password reset gesture
- Relay auto/manual modes
- Adjustable deltas and lockout
- Audible temperature threshold alarm (piezo)
- Internal+external sensor reads
- OLED runtime display
- Reboot-survivable blended history

## 12. Related Docs

- Technical architecture: [ARCHITECTURE.md](ARCHITECTURE.md)
- Product requirements: [PRD.md](PRD.md)
- API details: [API-REFERENCE.md](API-REFERENCE.md)
- History internals: [history.md](history.md)
