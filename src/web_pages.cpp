#include "web_pages.h"

const char PROVISION_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>SnowLeopard Provisioning</title>
  <style>
    :root {
      --bg: #f2f7f8;
      --card: #ffffff;
      --fg: #102a43;
      --muted: #486581;
      --accent: #0e7490;
      --err: #b91c1c;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: "Segoe UI", -apple-system, BlinkMacSystemFont, sans-serif;
      background: radial-gradient(circle at top, #e0f2fe 0%, var(--bg) 55%);
      color: var(--fg);
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 14px;
    }
    .card {
      width: 100%;
      max-width: 420px;
      background: var(--card);
      border-radius: 16px;
      box-shadow: 0 10px 26px rgba(15, 23, 42, 0.14);
      padding: 16px;
    }
    h1 {
      margin: 0 0 8px;
      font-size: 1.2rem;
    }
    p {
      margin: 0 0 12px;
      color: var(--muted);
      font-size: 0.92rem;
    }
    input[type="text"], input[type="password"] {
      width: 100%;
      border: 1px solid #cbd5e1;
      border-radius: 10px;
      padding: 10px;
      font-size: 1rem;
      margin-bottom: 10px;
    }
    button {
      width: 100%;
      border: 0;
      border-radius: 10px;
      padding: 10px;
      background: var(--accent);
      color: #fff;
      font-weight: 700;
      font-size: 1rem;
    }
    #err {
      margin-top: 10px;
      color: var(--err);
      min-height: 1.2em;
      font-size: 0.9rem;
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>SnowLeopard Provisioning</h1>
    <p>Connect SnowLeopard to your local Wi-Fi network.</p>
    <input id="ssid" type="text" placeholder="Wi-Fi SSID" autocomplete="off" />
    <input id="pwd" type="password" placeholder="Wi-Fi Password" autocomplete="off" />
    <button onclick="saveWifi()">Save Wi-Fi</button>
    <div id="err"></div>
  </div>
  <script>
    async function saveWifi() {
      const ssid = document.getElementById('ssid').value.trim();
      const pwd = document.getElementById('pwd').value;
      const err = document.getElementById('err');

      if (!ssid) {
        err.textContent = 'SSID is required';
        return;
      }

      const body = new URLSearchParams();
      body.set('ssid', ssid);
      body.set('password', pwd);

      const res = await fetch('/api/provision', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: body.toString()
      });

      if (res.ok) {
        err.style.color = '#0f766e';
        err.textContent = 'Saved. Device is trying to join Wi-Fi now...';
        return;
      }

      err.style.color = '#b91c1c';
      err.textContent = 'Failed to save Wi-Fi. Check SSID/password and try again.';
    }
  </script>
</body>
</html>
)HTML";

const char CAPTIVE_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <meta http-equiv="refresh" content="0; url=http://192.168.4.1/" />
  <title>SnowLeopard Network Login</title>
</head>
<body>
  <p>SnowLeopard network login required.</p>
  <p><a href="http://192.168.4.1/">Open Portal</a></p>
</body>
</html>
)HTML";

const char SETTINGS_AUTH_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>SnowLeopard Settings Login</title>
  <style>
    :root {
      --bg: #edf7f6;
      --card: #ffffff;
      --fg: #102a43;
      --muted: #486581;
      --accent: #0e7490;
      --err: #b91c1c;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: "Segoe UI", -apple-system, BlinkMacSystemFont, sans-serif;
      background: radial-gradient(circle at top, #e0f2fe 0%, var(--bg) 55%);
      color: var(--fg);
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 14px;
    }
    .card {
      width: 100%;
      max-width: 420px;
      background: var(--card);
      border-radius: 16px;
      box-shadow: 0 10px 26px rgba(15, 23, 42, 0.14);
      padding: 16px;
    }
    h1 { margin: 0 0 8px; font-size: 1.2rem; }
    p { margin: 0 0 12px; color: var(--muted); font-size: 0.92rem; }
    input[type="text"] {
      width: 100%;
      border: 1px solid #cbd5e1;
      border-radius: 10px;
      padding: 10px;
      font-size: 1rem;
      margin-bottom: 10px;
    }
    button {
      width: 100%;
      border: 0;
      border-radius: 10px;
      padding: 10px;
      background: var(--accent);
      color: #fff;
      font-weight: 700;
      font-size: 1rem;
    }
    #err {
      margin-top: 10px;
      color: var(--err);
      min-height: 1.2em;
      font-size: 0.9rem;
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>SnowLeopard Settings</h1>
    <p>Enter the settings password to continue.</p>
    <input id="settingsPwd" type="text" placeholder="Settings password" autocomplete="off" />
    <button onclick="unlockSettings()">Unlock Settings</button>
    <div id="err"></div>
  </div>
  <script>
    async function unlockSettings() {
      const pwd = document.getElementById('settingsPwd').value;
      const err = document.getElementById('err');

      const body = new URLSearchParams();
      body.set('password', pwd);

      const res = await fetch('/api/settings_login', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: body.toString()
      });

      if (res.ok) {
        location.href = '/settings';
        return;
      }

      err.textContent = 'Incorrect password.';
    }
  </script>
</body>
</html>
)HTML";

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>SnowLeopard Status</title>
  <style>
    :root {
      --bg: #edf7f6;
      --card: #ffffff;
      --fg: #102a43;
      --muted: #486581;
      --accent: #0e7490;
      --accent-dark: #155e75;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: "Segoe UI", -apple-system, BlinkMacSystemFont, sans-serif;
      background: radial-gradient(circle at top, #e0f2fe 0%, var(--bg) 55%);
      color: var(--fg);
      padding: 14px;
    }
    .card {
      max-width: 560px;
      margin: 0 auto;
      background: var(--card);
      border-radius: 16px;
      box-shadow: 0 10px 26px rgba(15, 23, 42, 0.14);
      padding: 14px;
    }
    h1 {
      margin: 0 0 12px;
      font-size: 1.2rem;
      color: var(--accent-dark);
    }
    .row {
      border-bottom: 1px solid #dbe7ef;
      padding: 9px 0;
      display: grid;
      grid-template-columns: 1fr auto auto;
      gap: 10px;
      align-items: baseline;
    }
    .k { color: var(--muted); }
    .v { font-weight: 700; text-align: right; min-width: 74px; }
    .h { font-weight: 700; text-align: right; min-width: 54px; color: var(--accent-dark); }
    button {
      border: 0;
      border-radius: 10px;
      padding: 10px;
      font-weight: 700;
      font-size: 1rem;
      background: var(--accent);
      color: white;
    }
    .actions {
      margin-top: 12px;
      display: flex;
      gap: 8px;
    }
    .foot {
      margin-top: 10px;
      text-align: center;
      font-size: 0.8rem;
      color: var(--muted);
    }
    .small {
      width: 100%;
      background: #334155;
      font-size: 0.95rem;
      padding: 8px;
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>SnowLeopard Status</h1>
    <div class="row"><div class="k" id="inLabel">Internal (Set --)</div><div class="v" id="inT">--.-</div><div class="h" id="inH">--%</div></div>
    <div class="row"><div class="k">External</div><div class="v" id="outT">--.-</div><div class="h" id="outH">--%</div></div>
    <div class="row"><div class="k">Relay State</div><div class="v" id="relay">OFF</div></div>
    <div class="row"><div class="k">Relay Mode</div><div class="v" id="mode">AUTO</div></div>
    <div class="row" id="alarmRow"><div class="k">Temp Alarm</div><div class="v" id="alarmState">NORMAL</div></div>
    <div class="actions">
      <button class="small" onclick="openSettings()">Settings</button>
      <button class="small" onclick="openHistory()">History</button>
    </div>
    <div class="foot">Refresh interval: 2 seconds</div>
  </div>

  <script>
    function fmtNum(v) {
      return Number.isFinite(v) ? v.toFixed(1) : '--.-';
    }

    function fmtSetpoint(v) {
      return Number.isFinite(v) ? String(Math.round(v)) : '--';
    }

    function fmtHum(v) {
      return Number.isFinite(v) ? v.toFixed(0) + '%' : '--%';
    }

    async function refresh() {
      try {
        const r = await fetch('/api/status', { cache: 'no-store' });
        const d = await r.json();
        document.getElementById('inLabel').textContent = 'Internal (Set ' + fmtSetpoint(d.set_temp) + ' ' + d.temp_unit + ')';
        document.getElementById('inT').textContent = fmtNum(d.internal_temp) + ' ' + d.temp_unit;
        document.getElementById('outT').textContent = fmtNum(d.external_temp) + ' ' + d.temp_unit;
        document.getElementById('inH').textContent = fmtHum(d.internal_humidity);
        document.getElementById('outH').textContent = fmtHum(d.external_humidity);
        document.getElementById('relay').textContent = d.relay_state;
        document.getElementById('mode').textContent = d.relay_mode;
        const alarmRow = document.getElementById('alarmRow');
        const alarmState = document.getElementById('alarmState');
        const alarmEnabled = d.alarm_enabled !== false;
        alarmRow.style.display = alarmEnabled ? 'grid' : 'none';
        alarmState.textContent = d.alarm_active ? 'ACTIVE' : 'NORMAL';
        alarmState.style.color = d.alarm_active ? '#dc2626' : '';
      } catch (_) {
      }
    }

    function openSettings() {
      location.href = '/settings';
    }

    function openHistory() {
      location.href = '/history';
    }

    refresh();
    setInterval(refresh, 2000);
  </script>
</body>
</html>
)HTML";

const char SETTINGS_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>SnowLeopard Settings</title>
  <style>
    :root {
      --bg: #edf7f6;
      --card: #ffffff;
      --fg: #102a43;
      --muted: #486581;
      --accent: #0e7490;
      --accent-dark: #155e75;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: "Segoe UI", -apple-system, BlinkMacSystemFont, sans-serif;
      background: radial-gradient(circle at top, #dff5ef 0%, var(--bg) 55%);
      color: var(--fg);
      padding: 14px;
    }
    .card {
      max-width: 560px;
      margin: 0 auto;
      background: var(--card);
      border-radius: 16px;
      box-shadow: 0 10px 26px rgba(15, 23, 42, 0.14);
      padding: 14px;
    }
    h1 {
      margin: 0 0 8px;
      font-size: 1.2rem;
      color: var(--accent-dark);
    }
    .back {
      display: inline-block;
      margin-bottom: 12px;
      color: var(--accent-dark);
      text-decoration: none;
      font-weight: 800;
      font-size: 1rem;
      padding: 6px 10px;
      border-radius: 999px;
      background: #e8f5f5;
    }
    p {
      margin: 0 0 12px;
      color: var(--muted);
      font-size: 0.92rem;
    }
    .field {
      margin: 10px 0;
    }
    label {
      display: block;
      margin-bottom: 6px;
      color: var(--muted);
      font-size: 0.9rem;
    }
    select, input[type="number"] {
      width: 100%;
      border: 1px solid #cbd5e1;
      border-radius: 10px;
      padding: 10px;
      font-size: 1rem;
      background: #fff;
    }
    .setpoint-row {
      display: grid;
      grid-template-columns: 48px 1fr 48px;
      gap: 8px;
      align-items: center;
    }
    .mini {
      font-size: 1.15rem;
      padding: 8px;
    }
    .buttons {
      display: grid;
      gap: 8px;
      margin-top: 14px;
    }
    details.advanced {
      margin-top: 14px;
      border: 1px solid #dbe7ef;
      border-radius: 10px;
      background: #f8fcfc;
      padding: 8px 10px;
    }
    details.advanced summary {
      cursor: pointer;
      font-weight: 700;
      color: var(--accent-dark);
      margin-bottom: 8px;
    }
    button {
      border: 0;
      border-radius: 10px;
      padding: 10px;
      font-weight: 700;
      font-size: 1rem;
      background: var(--accent);
      color: #fff;
    }
    .secondary {
      background: #334155;
    }
    .foot {
      margin-top: 10px;
      text-align: center;
      font-size: 0.8rem;
      color: var(--muted);
      min-height: 1.2em;
    }
  </style>
</head>
<body>
  <div class="card">
    <a class="back" href="/">&lt; Back to status</a>
    <h1>SnowLeopard Settings</h1>
    <p>Unit, relay mode, and temperature/alarm behavior.</p>

    <div class="field">
      <label for="unit">Temperature unit</label>
      <select id="unit">
        <option value="F">F</option>
        <option value="C">C</option>
      </select>
    </div>

    <div class="field">
      <label for="relayMode">Relay mode</label>
      <select id="relayMode">
        <option value="auto">Auto</option>
        <option value="manual_on">Manual locked on</option>
        <option value="manual_off">Manual locked off</option>
      </select>
    </div>

    <div class="field">
      <label for="setTemp">Set temperature</label>
      <div class="setpoint-row">
        <button class="mini" type="button" onclick="nudgeSetTemp(-stepAmount())">-</button>
        <input id="setTemp" type="number" step="1" />
        <button class="mini" type="button" onclick="nudgeSetTemp(stepAmount())">+</button>
      </div>
    </div>

    <details class="advanced">
      <summary>Temperature Threshold Alarm</summary>
      <div class="field">
        <label><input id="alarmEnabled" type="checkbox" /> Audible alarm enabled</label>
      </div>
      <div class="field">
        <label for="alarmLow" id="alarmLowLabel">Low alarm threshold (C)</label>
        <input id="alarmLow" type="number" step="1" />
      </div>
      <div class="field">
        <label for="alarmHigh" id="alarmHighLabel">High alarm threshold (C)</label>
        <input id="alarmHigh" type="number" step="1" />
      </div>
      <div class="field">
        <button class="secondary" type="button" onclick="runAlarmTest()">Test alarm</button>
      </div>
    </details>

    <details class="advanced">
      <summary>Advanced Compressor Control</summary>
      <div class="field">
        <label for="onDelta" id="onDeltaLabel">Turn-on delta above setpoint (C)</label>
        <input id="onDelta" type="number" step="0.1" />
      </div>
      <div class="field">
        <label for="offDelta" id="offDeltaLabel">Turn-off delta below setpoint (C)</label>
        <input id="offDelta" type="number" step="0.1" />
      </div>
      <div class="field">
        <label for="minOffSeconds">Minimum compressor off time (seconds)</label>
        <input id="minOffSeconds" type="number" step="10" />
      </div>
    </details>

    <details class="advanced">
      <summary>OLED Display Configuration</summary>
      <div class="field">
        <label><input type="radio" name="oledLayout" value="standard" checked /> Standard layout (default)</label>
      </div>
      <div class="field">
        <label><input type="radio" name="oledLayout" value="setpoint_only" /> Show only setpoint (large)</label>
      </div>
      <div class="field">
        <label><input type="radio" name="oledLayout" value="setpoint_internal" /> Show setpoint + internal temp (large)</label>
      </div>
      <div class="field">
        <label><input type="radio" name="oledLayout" value="setpoint_internal_relay" /> Show setpoint + internal temp + compressor state</label>
      </div>
      <div class="field">
        <label><input type="radio" name="oledLayout" value="internal_external" /> Show internal + external temp (large)</label>
      </div>
    </details>

    <details class="advanced">
      <summary>Auth</summary>
      <div class="field">
        <label><input id="settingsAuthEnabled" type="checkbox" /> Require password for Settings and changes</label>
      </div>
      <div class="field">
        <label for="settingsPassword1">Password</label>
        <input id="settingsPassword1" type="text" autocomplete="off" />
      </div>
      <div class="field">
        <label for="settingsPassword2">Confirm password</label>
        <input id="settingsPassword2" type="text" autocomplete="off" />
      </div>
      <div class="legend-note" id="settingsAuthNote">Leave both fields blank to keep the current password.</div>
    </details>

    <div class="buttons">
      <button class="secondary" onclick="openHistory()">History</button>
      <button class="secondary" onclick="reconfigureWifi()">Reconfigure Wi-Fi</button>
    </div>

    <div class="foot" id="msg"></div>
  </div>

  <script>
    let currentUnit = 'F';
    let saveTimer = null;
    let saveNonce = 0;
    let settingsLoaded = false;

    function cToF(value) {
      return (value * 9 / 5) + 32;
    }

    function fToC(value) {
      return (value - 32) * 5 / 9;
    }

    function convertForCurrentUnit(value, fromUnit, toUnit) {
      if (!Number.isFinite(value)) {
        return value;
      }

      if (fromUnit === toUnit) {
        return value;
      }

      return toUnit === 'F' ? cToF(value) : fToC(value);
    }

    function convertDeltaForCurrentUnit(value, fromUnit, toUnit) {
      if (!Number.isFinite(value) || fromUnit === toUnit) {
        return value;
      }
      return toUnit === 'F' ? (value * 9 / 5) : (value * 5 / 9);
    }

    function setStatus(text, color) {
      const msg = document.getElementById('msg');
      msg.style.color = color || '#486581';
      msg.textContent = text;
    }

    function stepAmount() {
      return 1.0;
    }

    function readNumber(id) {
      const value = Number(document.getElementById(id).value);
      return Number.isFinite(value) ? value : null;
    }

    function normalizeRelayMode(value) {
      if (!value) {
        return 'auto';
      }

      const v = String(value).trim().toLowerCase();
      if (v === 'manual_on' || v === 'man on' || v === 'manual locked on') {
        return 'manual_on';
      }
      if (v === 'manual_off' || v === 'man off' || v === 'manual locked off') {
        return 'manual_off';
      }
      return 'auto';
    }

    function getSelectedOledLayout() {
      const selected = document.querySelector('input[name="oledLayout"]:checked');
      return selected ? selected.value : 'standard';
    }

    function setSelectedOledLayout(value) {
      const normalized = value || 'standard';
      const candidate = document.querySelector('input[name="oledLayout"][value="' + normalized + '"]');
      if (candidate) {
        candidate.checked = true;
      } else {
        const fallback = document.querySelector('input[name="oledLayout"][value="standard"]');
        if (fallback) {
          fallback.checked = true;
        }
      }
    }

    function updateAuthNote() {
      const authEnabled = document.getElementById('settingsAuthEnabled').checked;
      const note = document.getElementById('settingsAuthNote');
      note.textContent = authEnabled
        ? 'Leave both fields blank to keep the current password.'
        : 'Turning this off clears the saved password.';
    }

    function applySettingsPayload(d) {
      currentUnit = d.temp_unit || 'F';
      document.getElementById('unit').value = currentUnit;
      document.getElementById('relayMode').value = normalizeRelayMode(d.relay_mode);
      document.getElementById('setTemp').value = Number.isFinite(d.set_temp) ? String(Math.round(d.set_temp)) : '';
      document.getElementById('settingsAuthEnabled').checked = d.settings_auth_enabled === true;
      document.getElementById('settingsPassword1').value = '';
      document.getElementById('settingsPassword2').value = '';
      document.getElementById('alarmEnabled').checked = d.alarm_enabled !== false;
      setSelectedOledLayout(d.oled_layout || 'standard');
      updateAuthNote();
      document.getElementById('alarmLow').value = Number.isFinite(d.alarm_low) ? String(Math.round(d.alarm_low)) : '';
      document.getElementById('alarmHigh').value = Number.isFinite(d.alarm_high) ? String(Math.round(d.alarm_high)) : '';
      document.getElementById('onDelta').value = Number.isFinite(d.relay_on_delta) ? d.relay_on_delta.toFixed(2) : '';
      document.getElementById('offDelta').value = Number.isFinite(d.relay_off_delta) ? d.relay_off_delta.toFixed(2) : '';
      document.getElementById('minOffSeconds').value = Number.isFinite(d.min_off_seconds) ? Math.round(d.min_off_seconds) : '';

      document.getElementById('alarmLowLabel').textContent = 'Low alarm threshold (' + currentUnit + ')';
      document.getElementById('alarmHighLabel').textContent = 'High alarm threshold (' + currentUnit + ')';
      document.getElementById('onDeltaLabel').textContent = 'Turn-on delta above setpoint (' + currentUnit + ')';
      document.getElementById('offDeltaLabel').textContent = 'Turn-off delta below setpoint (' + currentUnit + ')';
    }

    async function loadSettings() {
      const r = await fetch('/api/settings', { cache: 'no-store' });
      const d = await r.json();
      applySettingsPayload(d);
      if (saveTimer) { clearTimeout(saveTimer); saveTimer = null; }
      settingsLoaded = true;
    }

    function scheduleSave() {
      if (!settingsLoaded) {
        return;
      }

      if (saveTimer) {
        clearTimeout(saveTimer);
      }

      saveTimer = setTimeout(() => {
        persistSettings();
      }, 250);
    }

    async function persistSettings() {
      saveTimer = null;
      const current = ++saveNonce;
      const setTemp = readNumber('setTemp');
      const alarmLow = readNumber('alarmLow');
      const alarmHigh = readNumber('alarmHigh');
      const onDelta = readNumber('onDelta');
      const offDelta = readNumber('offDelta');
      const minOffSeconds = readNumber('minOffSeconds');
      const settingsAuthEnabled = document.getElementById('settingsAuthEnabled').checked;
      const settingsPassword1 = document.getElementById('settingsPassword1').value;
      const settingsPassword2 = document.getElementById('settingsPassword2').value;

      if (setTemp === null || alarmLow === null || alarmHigh === null || onDelta === null || offDelta === null ||
          minOffSeconds === null) {
        return;
      }

      if (Math.round(alarmLow) >= Math.round(alarmHigh)) {
        setStatus('Low alarm must be less than high alarm.', '#b91c1c');
        return;
      }

      const passwordFieldsFilled = settingsPassword1.length > 0 || settingsPassword2.length > 0;
      if (settingsAuthEnabled && passwordFieldsFilled && settingsPassword1 !== settingsPassword2) {
        setStatus('Settings passwords do not match.', '#b91c1c');
        return;
      }

      setStatus('Saving...', '#486581');

      const body = new URLSearchParams();
      body.set('temp_unit', document.getElementById('unit').value);
      body.set('relay_mode', normalizeRelayMode(document.getElementById('relayMode').value));
      body.set('oled_layout', getSelectedOledLayout());
      body.set('set_temp', String(Math.round(setTemp)));
      body.set('settings_auth_enabled', settingsAuthEnabled ? '1' : '0');
      if (settingsPassword1.length > 0 || settingsPassword2.length > 0) {
        body.set('settings_auth_password', settingsPassword1);
        body.set('settings_auth_password_confirm', settingsPassword2);
      }
      body.set('alarm_enabled', document.getElementById('alarmEnabled').checked ? '1' : '0');
      body.set('alarm_low', String(Math.round(alarmLow)));
      body.set('alarm_high', String(Math.round(alarmHigh)));
      body.set('relay_on_delta', String(onDelta));
      body.set('relay_off_delta', String(offDelta));
      body.set('min_off_seconds', String(Math.round(minOffSeconds)));

      let res;
      try {
        res = await fetch('/api/settings', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: body.toString()
        });
      } catch (_) {
        if (current === saveNonce) {
          setStatus('Failed to save settings.', '#b91c1c');
        }
        return;
      }

      if (current !== saveNonce) {
        return;
      }

      if (!res.ok) {
        let errCode = '';
        try {
          const errPayload = await res.json();
          errCode = errPayload && errPayload.error ? String(errPayload.error) : '';
        } catch (_) {
        }

        if (errCode === 'password_required') {
          setStatus('Enter a password and confirmation to enable settings protection.', '#b91c1c');
          return;
        }

        if (errCode === 'password_mismatch') {
          setStatus('Settings passwords do not match.', '#b91c1c');
          return;
        }

        if (errCode === 'auth_required') {
          setStatus('Unlock Settings before making changes.', '#b91c1c');
          return;
        }

        setStatus('Failed to save settings.', '#b91c1c');
        return;
      }

      try {
        const data = await res.json();
        applySettingsPayload(data);
        setStatus('Settings saved.', '#0f766e');
      } catch (_) {
        setStatus('Settings saved.', '#0f766e');
      }
    }

    document.getElementById('unit').addEventListener('change', () => {
      const nextUnit = document.getElementById('unit').value;
      if (nextUnit === currentUnit) {
        return;
      }

      const setTemp = readNumber('setTemp');
      const alarmLow = readNumber('alarmLow');
      const alarmHigh = readNumber('alarmHigh');
      const onDelta = readNumber('onDelta');
      const offDelta = readNumber('offDelta');

      if (setTemp !== null) {
        document.getElementById('setTemp').value = String(Math.round(convertForCurrentUnit(setTemp, currentUnit, nextUnit)));
      }
      if (alarmLow !== null) {
        document.getElementById('alarmLow').value = String(Math.round(convertForCurrentUnit(alarmLow, currentUnit, nextUnit)));
      }
      if (alarmHigh !== null) {
        document.getElementById('alarmHigh').value = String(Math.round(convertForCurrentUnit(alarmHigh, currentUnit, nextUnit)));
      }
      if (onDelta !== null) {
        document.getElementById('onDelta').value = convertDeltaForCurrentUnit(onDelta, currentUnit, nextUnit).toFixed(2);
      }
      if (offDelta !== null) {
        document.getElementById('offDelta').value = convertDeltaForCurrentUnit(offDelta, currentUnit, nextUnit).toFixed(2);
      }
      currentUnit = nextUnit;
      document.getElementById('alarmLowLabel').textContent = 'Low alarm threshold (' + currentUnit + ')';
      document.getElementById('alarmHighLabel').textContent = 'High alarm threshold (' + currentUnit + ')';
      document.getElementById('onDeltaLabel').textContent = 'Turn-on delta above setpoint (' + currentUnit + ')';
      document.getElementById('offDeltaLabel').textContent = 'Turn-off delta below setpoint (' + currentUnit + ')';
      scheduleSave();
    });

    function nudgeSetTemp(delta) {
      const next = readNumber('setTemp');
      if (next === null) {
        return;
      }
      document.getElementById('setTemp').value = String(Math.round(next + delta));
      scheduleSave();
    }

    async function reconfigureWifi() {
      if (!confirm('Switch to SnowLeopard provisioning AP now?')) {
        return;
      }

      const res = await fetch('/api/reconfigure', { method: 'POST' });
      if (res.ok) {
        setStatus('Reconfigure request sent.', '#0f766e');
      } else {
        setStatus('Failed to start provisioning mode.', '#b91c1c');
      }
    }

    async function runAlarmTest() {
      setStatus('Running alarm test...', '#486581');
      const res = await fetch('/api/alarm_test', { method: 'POST' });
      if (res.ok) {
        setStatus('Alarm test started.', '#0f766e');
      } else {
        setStatus('Failed to start alarm test.', '#b91c1c');
      }
    }

    function openHistory() {
      location.href = '/history';
    }

    ['relayMode', 'setTemp', 'alarmEnabled', 'alarmLow', 'alarmHigh', 'onDelta', 'offDelta', 'minOffSeconds'].forEach((id) => {
      document.getElementById(id).addEventListener('change', scheduleSave);
    });

    ['settingsAuthEnabled', 'settingsPassword1', 'settingsPassword2'].forEach((id) => {
      document.getElementById(id).addEventListener('change', scheduleSave);
    });

    document.getElementById('settingsAuthEnabled').addEventListener('change', updateAuthNote);

    document.querySelectorAll('input[name="oledLayout"]').forEach((el) => {
      el.addEventListener('change', scheduleSave);
    });

    loadSettings();
  </script>
</body>
</html>
)HTML";

const char HISTORY_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>SnowLeopard History</title>
  <style>
    :root {
      --bg: #edf7f6;
      --card: #ffffff;
      --fg: #102a43;
      --muted: #486581;
      --accent: #0e7490;
      --accent-dark: #155e75;
      --in: #0891b2;
      --out: #ea580c;
      --relay: rgba(51, 65, 85, 0.16);
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: "Segoe UI", -apple-system, BlinkMacSystemFont, sans-serif;
      background: radial-gradient(circle at top, #dff5ef 0%, var(--bg) 55%);
      color: var(--fg);
      padding: 14px;
    }
    .card {
      max-width: 900px;
      margin: 0 auto;
      background: var(--card);
      border-radius: 16px;
      box-shadow: 0 10px 26px rgba(15, 23, 42, 0.14);
      padding: 14px;
    }
    h1 {
      margin: 0 0 10px;
      font-size: 1.2rem;
      color: var(--accent-dark);
    }
    .back {
      display: inline-block;
      margin-bottom: 12px;
      color: var(--accent-dark);
      text-decoration: none;
      font-weight: 800;
      font-size: 1rem;
      padding: 6px 10px;
      border-radius: 999px;
      background: #e8f5f5;
    }
    .toolbar {
      display: flex;
      flex-wrap: wrap;
      gap: 8px;
      align-items: center;
      margin-bottom: 10px;
    }
    .range {
      border: 1px solid #cbd5e1;
      border-radius: 999px;
      padding: 6px 10px;
      background: #fff;
      color: var(--muted);
      font-weight: 700;
      cursor: pointer;
    }
    .range.active {
      border-color: var(--accent);
      background: #d9f3f3;
      color: var(--accent-dark);
    }
    .chk {
      margin-left: auto;
      display: flex;
      align-items: center;
      gap: 6px;
      color: var(--muted);
      font-size: 0.9rem;
      font-weight: 600;
    }
    .legend {
      display: flex;
      flex-wrap: wrap;
      gap: 12px;
      margin-bottom: 8px;
      color: var(--muted);
      font-size: 0.9rem;
      align-items: center;
    }
    .legend-item {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      font-weight: 600;
    }
    .swatch-line {
      width: 14px;
      height: 0;
      border-top: 3px solid transparent;
      border-radius: 999px;
      display: inline-block;
    }
    .swatch-dot {
      width: 11px;
      height: 11px;
      border-radius: 50%;
      display: inline-block;
    }
    .swatch-gap {
      width: 10px;
      height: 16px;
      border-left: 2px dashed #64748b;
      display: inline-block;
      opacity: 0.9;
    }
    .legend-note {
      margin: 2px 0 10px;
      color: #64748b;
      font-size: 0.8rem;
    }
    .chart-wrap {
      border: 1px solid #dbe7ef;
      border-radius: 12px;
      padding: 8px;
      background: #f9fbfc;
    }
    canvas {
      width: 100%;
      height: 320px;
      display: block;
    }
    .meta {
      margin-top: 8px;
      color: var(--muted);
      font-size: 0.82rem;
    }
  </style>
</head>
<body>
  <div class="card">
    <a class="back" href="/">&lt; Back to status</a>
    <h1>SnowLeopard History</h1>
    <div class="toolbar" id="ranges">
      <button class="range" data-window="300">5m</button>
      <button class="range" data-window="900">15m</button>
      <button class="range" data-window="1800">30m</button>
      <button class="range active" data-window="3600">1h</button>
      <button class="range" data-window="10800">3h</button>
      <button class="range" data-window="21600">6h</button>
      <button class="range" data-window="43200">12h</button>
      <button class="range" data-window="86400">24h</button>
      <label class="chk"><input id="ext" type="checkbox" checked /> Show external</label>
    </div>
    <div class="legend">
      <span class="legend-item"><span class="swatch-line" style="border-color:#0891b2"></span>Internal temp (control)</span>
      <span class="legend-item"><span class="swatch-line" style="border-color:#ea580c"></span>External temp</span>
      <span class="legend-item"><span class="swatch-dot" style="background:rgba(51,65,85,0.5)"></span>Compressor ON</span>
      <span class="legend-item"><span class="swatch-gap"></span>Restart/session gap</span>
    </div>
    <div class="legend-note">Gap lines indicate where history crosses a restart or session boundary.</div>
    <div class="chart-wrap"><canvas id="chart" width="860" height="320"></canvas></div>
    <div class="meta" id="meta">Loading history...</div>
  </div>

  <script>
    const canvas = document.getElementById('chart');
    const ctx = canvas.getContext('2d');
    let windowSec = 3600;

    function resizeCanvas() {
      const dpr = window.devicePixelRatio || 1;
      const cssW = canvas.clientWidth;
      const cssH = canvas.clientHeight;
      canvas.width = Math.round(cssW * dpr);
      canvas.height = Math.round(cssH * dpr);
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    }

    function drawHistory(data) {
      resizeCanvas();
      const w = canvas.clientWidth;
      const h = canvas.clientHeight;
      ctx.clearRect(0, 0, w, h);

      const left = 44;
      const right = w - 14;
      const top = 14;
      const bottom = h - 28;
      const plotW = Math.max(20, right - left);
      const plotH = Math.max(20, bottom - top);

      const ages = Array.isArray(data.age_s) ? data.age_s : [];
      const internal = Array.isArray(data.internal) ? data.internal : [];
      const external = Array.isArray(data.external) ? data.external : null;
      const relay = Array.isArray(data.relay) ? data.relay : [];
      const discontinuity = Array.isArray(data.discontinuity) ? data.discontinuity : [];
      const discontinuityAges = Array.isArray(data.discontinuity_age_s)
        ? data.discontinuity_age_s.filter((v) => Number.isFinite(v))
        : [];

      const vals = [];
      for (let i = 0; i < internal.length; i++) if (Number.isFinite(internal[i])) vals.push(internal[i]);
      if (external) for (let i = 0; i < external.length; i++) if (Number.isFinite(external[i])) vals.push(external[i]);

      if (!ages.length || !vals.length) {
        ctx.fillStyle = '#486581';
        ctx.font = '14px Segoe UI, sans-serif';
        ctx.fillText('No history yet. Samples populate every 10s (live) / 60s (snapshot).', left, top + 20);
        return;
      }

      let minV = Math.min(...vals);
      let maxV = Math.max(...vals);
      if (maxV - minV < 1) {
        maxV += 0.5;
        minV -= 0.5;
      }
      const pad = (maxV - minV) * 0.1;
      minV -= pad;
      maxV += pad;

      const maxAge = Math.max(...ages);
      const minAge = Math.min(...ages);
      const ageSpan = Math.max(1, maxAge - minAge);

      function xFor(i) {
        return left + (i / Math.max(1, ages.length - 1)) * plotW;
      }
      function xForAge(age) {
        return left + ((maxAge - age) / ageSpan) * plotW;
      }
      function yFor(v) {
        return top + (maxV - v) / (maxV - minV) * plotH;
      }

      ctx.strokeStyle = '#d7e2ec';
      ctx.lineWidth = 1;
      for (let i = 0; i <= 4; i++) {
        const y = top + i * (plotH / 4);
        ctx.beginPath();
        ctx.moveTo(left, y);
        ctx.lineTo(right, y);
        ctx.stroke();
      }

      ctx.fillStyle = '#64748b';
      ctx.font = '11px Segoe UI, sans-serif';
      ctx.fillText(maxV.toFixed(1), 4, top + 4);
      ctx.fillText(minV.toFixed(1), 4, bottom + 4);
      ctx.fillText('-' + Math.round(maxAge / 60) + 'm', left, h - 8);
      ctx.fillText('now', right - 20, h - 8);

      ctx.fillStyle = 'rgba(51,65,85,0.16)';
      for (let i = 0; i < relay.length - 1; i++) {
        if (relay[i] === 1) {
          const x1 = xFor(i);
          const x2 = xFor(i + 1);
          ctx.fillRect(x1, top, Math.max(1, x2 - x1), plotH);
        }
      }

      function drawSeries(values, color, width) {
        ctx.strokeStyle = color;
        ctx.lineWidth = width;
        ctx.beginPath();
        let started = false;
        for (let i = 0; i < values.length; i++) {
          const v = values[i];
          if (!Number.isFinite(v)) {
            started = false;
            continue;
          }
          const x = xFor(i);
          const y = yFor(v);
          if (!started) {
            ctx.moveTo(x, y);
            started = true;
          } else {
            ctx.lineTo(x, y);
          }
        }
        ctx.stroke();
      }

      drawSeries(internal, '#0891b2', 2);
      if (external) {
        drawSeries(external, '#ea580c', 1.6);
      }

      if (discontinuityAges.length) {
        ctx.strokeStyle = 'rgba(100,116,139,0.9)';
        ctx.lineWidth = 1;
        ctx.setLineDash([4, 4]);

        const markerXs = [];
        for (let i = 0; i < discontinuityAges.length; i++) {
          const age = discontinuityAges[i];
          if (age >= minAge && age <= maxAge) {
            markerXs.push(xForAge(age));
          }
        }
        markerXs.sort((a, b) => a - b);

        const clustered = [];
        for (let i = 0; i < markerXs.length; i++) {
          const x = markerXs[i];
          if (!clustered.length || Math.abs(x - clustered[clustered.length - 1]) > 3) {
            clustered.push(x);
          } else {
            clustered[clustered.length - 1] = (clustered[clustered.length - 1] + x) / 2;
          }
        }

        for (let i = 0; i < clustered.length; i++) {
          const x = clustered[i];
          ctx.beginPath();
          ctx.moveTo(x, top);
          ctx.lineTo(x, bottom);
          ctx.stroke();
        }
        ctx.setLineDash([]);
      }

      ctx.strokeStyle = '#94a3b8';
      ctx.lineWidth = 1;
      ctx.strokeRect(left, top, plotW, plotH);

      const meta = document.getElementById('meta');
      const requestedMinutes = Math.round((Number.isFinite(data.requested_window_s) ? data.requested_window_s : data.window_s) / 60);
      const availableMinutes = Math.round(data.window_s / 60);
      const capped = requestedMinutes > availableMinutes ? ` (capped from ${requestedMinutes}m)` : '';
      const gapCount = Number.isFinite(data.discontinuity_count)
        ? data.discontinuity_count
        : discontinuityAges.length;
      const discontinuityNote = gapCount > 0 ? ` | Gaps: ${gapCount}` : '';
      meta.textContent = `Source: ${data.source} | Window: ${availableMinutes}m${capped} | Step: ${data.effective_step_s}s | Points: ${ages.length}${discontinuityNote}`;
    }

    async function refreshHistory() {
      const includeExternal = document.getElementById('ext').checked ? 1 : 0;
      try {
        const r = await fetch(`/api/history?window_s=${windowSec}&include_external=${includeExternal}`,
          { cache: 'no-store' });
        const d = await r.json();
        drawHistory(d);
      } catch (e) {
        document.getElementById('meta').textContent = 'Failed to load history.';
      }
    }

    document.querySelectorAll('.range').forEach((btn) => {
      btn.addEventListener('click', () => {
        document.querySelectorAll('.range').forEach((b) => b.classList.remove('active'));
        btn.classList.add('active');
        windowSec = Number(btn.dataset.window);
        refreshHistory();
      });
    });

    document.getElementById('ext').addEventListener('change', refreshHistory);
    window.addEventListener('resize', refreshHistory);

    refreshHistory();
    setInterval(refreshHistory, 15000);
  </script>
</body>
</html>
)HTML";
