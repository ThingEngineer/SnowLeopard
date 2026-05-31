# SnowLeopard Quick Start

This is a fast setup path to get SnowLeopard cooling and reachable from your browser.

## 1. Before You Power On

- Confirm your relay compressor contact uses COM + NO (normally open).
- Power the controller.

## 2. Join Provisioning Wi-Fi

1. On your phone or laptop, connect to Wi-Fi SSID: SnowLeopard.
2. If a captive page does not open automatically, browse to http://192.168.4.1.

## 3. Save Your Home Wi-Fi

1. Enter your normal Wi-Fi SSID and password.
2. Press Save Wi-Fi.
3. Wait for reboot.

## 4. Open the Device in STA Mode

After reboot, open one of these:

- http://snowleopard.local
- http://<device-ip>

If snowleopard.local does not resolve on your network, use the device IP.

## 5. Set Initial Cooling Values

Open Settings and apply this safe baseline:

- Temperature unit: C
- Relay mode: Auto
- Set temperature: 4
- Audible alarm enabled: ON
- Low alarm threshold: 1
- High alarm threshold: 8
- Turn-on delta above setpoint: 1.0
- Turn-off delta below setpoint: 0.5
- Minimum compressor off time: 180 seconds

Optional:

- Open the Auth section if you want Settings protected by a password.
- If you turn Auth off later, the saved password is cleared and you can create a new one when re-enabling it.

Wait for the Settings saved confirmation.

## 6. Confirm It Is Running

On the Status page verify:

- Internal temperature updates
- Relay mode is AUTO
- Relay state changes as temperature crosses thresholds

On the History page verify:

- Internal line appears
- Relay shading toggles during compressor cycles

## 7. Wi-Fi Loss Behavior

If Wi-Fi drops long enough, the device can return to AP fallback mode.
Cooling still runs in AUTO in this state.

Provisioning/AP effective setpoint rule:

- Uses saved setpoint only when it is between 0 C and 8 C
- Otherwise uses 4 C

## 8. Reset the Settings Password

If you forget the Settings password, hold both physical buttons and press Down twice within 5 seconds.

- The password is cleared
- The OLED briefly shows `PW Cleared`

## 9. If Something Fails

1. Cannot connect to SnowLeopard AP:

- Reboot the device and retry.

2. Cannot open snowleopard.local:

- Use the device IP directly.

3. Relay not switching:

- Ensure relay mode is Auto.
- Check internal sensor is reporting valid data.
- Confirm minimum off-time lockout has elapsed.

For full troubleshooting and all features, see [USER-GUIDE.md](USER-GUIDE.md).
