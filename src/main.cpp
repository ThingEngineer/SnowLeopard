#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <U8g2lib.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_Sensor.h>
#include <ESPmDNS.h>

#include "button_input.h"
#include "alarm_beeper.h"
#include "device_types.h"
#include "display_renderer.h"
#include "history_store.h"
#include "i2c_bus.h"
#include "json_builders.h"
#include "network_workflow.h"
#include "network_helpers.h"
#include "password_reset_gesture.h"
#include "relay_control.h"
#include "settings_apply.h"
#include "settings_auth.h"
#include "settings_parse.h"
#include "settings_request.h"
#include "settings_store.h"
#include "sensor_io.h"
#include "sensor_pipeline.h"
#include "setpoint_adjust.h"
#include "temperature.h"
#include "web_pages.h"
#include "web_routes.h"

#include <math.h>

// Pin map (verified against board images and docs)
static constexpr uint8_t PIN_OLED_SCL = 6;
static constexpr uint8_t PIN_OLED_SDA = 5;
static constexpr uint8_t PIN_HW_SCL = 9;
static constexpr uint8_t PIN_HW_SDA = 8;
static constexpr uint8_t PIN_RELAY = 7;
static constexpr uint8_t PIN_BTN_UP = 2;
static constexpr uint8_t PIN_BTN_DOWN = 3;
static constexpr uint8_t PIN_PIEZO = 4;

// Timing
static constexpr uint32_t SENSOR_PERIOD_MS = 2000;
static constexpr uint32_t DISPLAY_PAGE_MS = 3000;
static constexpr uint32_t BUTTON_DEBOUNCE_MS = 15;
static constexpr uint32_t BUTTON_HOLD_REPEAT_MS = 600;
static constexpr uint32_t SETPOINT_FOCUS_DISPLAY_MS = 3000;
static constexpr uint32_t ALARM_BEEP_STEP_MS = 220;
static constexpr uint32_t ALARM_BEEP_PAUSE_MS = 1000;
static constexpr uint32_t ALARM_TEST_DURATION_MS = 2000;
static constexpr uint32_t SENSOR_RETRY_MS = 10000;
static constexpr uint32_t HISTORY_SAMPLE_MS = 10000;
static constexpr uint32_t HISTORY_SNAPSHOT_SAMPLE_MS = 60000;
static constexpr uint32_t HISTORY_SNAPSHOT_PERSIST_MS = 300000;
static constexpr uint32_t DEFAULT_LOCKOUT_MS = 180000;
static constexpr uint32_t LOCKOUT_MIN_MS = 60000;
static constexpr uint32_t LOCKOUT_MAX_MS = 900000;

// Control
static constexpr float TARGET_MIN_C = -50.0f;
static constexpr float TARGET_MAX_C = 50.0f;
static constexpr float DEFAULT_SET_TEMP_C = 4.0f;
static constexpr float PROVISIONING_SETPOINT_MIN_C = 0.0f;
static constexpr float PROVISIONING_SETPOINT_MAX_C = 8.0f;
static constexpr float TARGET_STEP_C = 1.0f;
static constexpr float DEFAULT_RELAY_ON_DELTA_C = 1.0f;
static constexpr float DEFAULT_RELAY_OFF_DELTA_C = 0.5f;
static constexpr float RELAY_DELTA_MIN_C = 0.1f;
static constexpr float RELAY_DELTA_MAX_C = 5.0f;
static constexpr float DEFAULT_ALARM_LOW_C = 1.0f;
static constexpr float DEFAULT_ALARM_HIGH_C = 8.0f;
static constexpr uint16_t ALARM_TONE_HIGH_HZ = 3200;
static constexpr uint16_t ALARM_TONE_LOW_HZ = 2100;
static constexpr uint8_t PIEZO_LEDC_CHANNEL = 0;
static constexpr uint8_t PIEZO_LEDC_RESOLUTION_BITS = 8;
static constexpr uint8_t PIEZO_LEDC_DUTY = 128;
static constexpr int OLED_X_OFFSET = 0;
static constexpr int OLED_Y_OFFSET = 0;
static constexpr uint16_t HISTORY_LIVE_CAPACITY = 4320;      // 12h @ 10s
static constexpr uint16_t HISTORY_SNAPSHOT_CAPACITY = 1440;  // 24h @ 60s
static constexpr uint16_t HISTORY_API_MAX_POINTS = 600;

// AP config
static constexpr const char* AP_SSID = "SnowLeopard";
static constexpr byte DNS_PORT = 53;
static constexpr uint8_t AP_CHANNEL = 1;
static constexpr bool AP_HIDDEN = false;
static constexpr uint8_t AP_MAX_CLIENTS = 4;
static constexpr uint32_t STA_CONNECT_TIMEOUT_MS = 20000;
static constexpr uint8_t STA_CONNECT_ATTEMPTS = 2;
static constexpr uint32_t STA_FALLBACK_TO_AP_MS = 60000;
static constexpr const char* SETTINGS_AUTH_COOKIE_NAME = "snow_settings_auth";
static constexpr uint32_t SETTINGS_AUTH_COOKIE_TTL_SEC = 86400;
static constexpr bool ENABLE_CAPTIVE_DEBUG_LOGS = false;

U8G2_SSD1306_72X40_ER_F_SW_I2C u8g2(U8G2_R0, PIN_OLED_SCL, PIN_OLED_SDA, U8X8_PIN_NONE);

Adafruit_AHTX0 ahtInternal;
Adafruit_AHTX0 ahtExternal;

ActiveI2cBus activeI2cBus = I2C_BUS_OLED;
bool i2cInitialized = false;

DNSServer dnsServer;
AsyncWebServer webServer(80);
Preferences preferences;

bool tempUnitF = true;
RelayMode relayMode = RELAY_MODE_AUTO;
OledLayoutMode oledLayoutMode = OLED_LAYOUT_STANDARD;
float setTempC = DEFAULT_SET_TEMP_C;
float relayOnDeltaC = DEFAULT_RELAY_ON_DELTA_C;
float relayOffDeltaC = DEFAULT_RELAY_OFF_DELTA_C;
float alarmLowTempC = DEFAULT_ALARM_LOW_C;
float alarmHighTempC = DEFAULT_ALARM_HIGH_C;
bool tempAlarmEnabled = true;
bool settingsPasswordEnabled = false;
String settingsPassword;
String settingsAuthToken;
uint32_t relayLockoutMs = DEFAULT_LOCKOUT_MS;

float internalTempC = NAN;
float internalHumidity = NAN;
float externalTempC = NAN;
float externalHumidity = NAN;

bool internalValid = false;
bool externalValid = false;
bool relayOn = false;

uint32_t relayOffSinceMs = 0;
uint32_t lastSensorMs = 0;
uint32_t lastDisplayFlipMs = 0;
uint32_t lastInternalRetryMs = 0;
uint32_t lastExternalRetryMs = 0;

bool upStable = HIGH;
bool upLastRead = HIGH;
uint32_t upDebounceStartMs = 0;
uint32_t upNextRepeatMs = 0;

bool downStable = HIGH;
bool downLastRead = HIGH;
uint32_t downDebounceStartMs = 0;
uint32_t downNextRepeatMs = 0;

PasswordResetGestureState passwordResetGesture{};

bool provisioningMode = true;
String staSsid;
String staPass;
bool pendingStaReboot = false;
uint32_t staDisconnectedSinceMs = 0;
bool haveSensorReading = false;
uint32_t staInfoDisplayUntilMs = 0;
uint32_t setpointFocusUntilMs = 0;
bool tempAlarmBeepActive = false;
uint8_t tempAlarmBeepStep = 0;
uint32_t tempAlarmNextStepMs = 0;
uint32_t tempAlarmTestUntilMs = 0;
uint32_t oledNoticeUntilMs = 0;
String oledNoticeText;

const char* tempUnitText() {
  return tempUnitF ? "F" : "C";
}

float displayTempC(float celsius) {
  return tempconv::displayTempC(celsius, tempUnitF);
}

float displayTempToC(float value) {
  return tempconv::displayTempToC(value, tempUnitF);
}

float clampTempC(float value) {
  return tempconv::clampFloatRange(value, TARGET_MIN_C, TARGET_MAX_C);
}

float clampRelayDeltaC(float value) {
  return tempconv::clampFloatRange(value, RELAY_DELTA_MIN_C, RELAY_DELTA_MAX_C);
}

uint32_t clampRelayLockoutMs(uint32_t value) {
  return tempconv::clampUInt32Range(value, LOCKOUT_MIN_MS, LOCKOUT_MAX_MS);
}

void clampSetpointToBounds() {
  setTempC = clampTempC(setTempC);
}

void normalizeSetpointToWholeDisplayUnit() {
  if (!isfinite(setTempC)) {
    setTempC = DEFAULT_SET_TEMP_C;
  }
  const float roundedDisplay = roundf(displayTempC(setTempC));
  setTempC = displayTempToC(roundedDisplay);
  clampSetpointToBounds();
}

void normalizeAlarmThresholdsToWholeDisplayUnit() {
  if (!isfinite(alarmLowTempC)) {
    alarmLowTempC = DEFAULT_ALARM_LOW_C;
  }
  if (!isfinite(alarmHighTempC)) {
    alarmHighTempC = DEFAULT_ALARM_HIGH_C;
  }

  alarmLowTempC = clampTempC(alarmLowTempC);
  alarmHighTempC = clampTempC(alarmHighTempC);

  const float lowRoundedDisplay = roundf(displayTempC(alarmLowTempC));
  const float highRoundedDisplay = roundf(displayTempC(alarmHighTempC));
  alarmLowTempC = clampTempC(displayTempToC(lowRoundedDisplay));
  alarmHighTempC = clampTempC(displayTempToC(highRoundedDisplay));

  if (alarmLowTempC >= alarmHighTempC) {
    if (alarmHighTempC < TARGET_MAX_C) {
      alarmLowTempC = alarmHighTempC - 1.0f;
    } else {
      alarmHighTempC = alarmLowTempC + 1.0f;
    }
    alarmLowTempC = clampTempC(alarmLowTempC);
    alarmHighTempC = clampTempC(alarmHighTempC);
  }
}

bool internalTempOutsideAlarmThresholds() {
  alarmbeeper::AlarmBeeperState state{};
  state.tempAlarmEnabled = tempAlarmEnabled;
  state.internalValid = internalValid;
  state.internalTempC = internalTempC;
  state.alarmLowTempC = alarmLowTempC;
  state.alarmHighTempC = alarmHighTempC;
  return alarmbeeper::internalTempOutsideAlarmThresholds(state);
}

void updateTemperatureAlarmBeeper(uint32_t nowMs) {
  alarmbeeper::AlarmBeeperState state{};
  state.tempAlarmEnabled = tempAlarmEnabled;
  state.internalValid = internalValid;
  state.internalTempC = internalTempC;
  state.alarmLowTempC = alarmLowTempC;
  state.alarmHighTempC = alarmHighTempC;
  state.tempAlarmBeepActive = tempAlarmBeepActive;
  state.tempAlarmBeepStep = tempAlarmBeepStep;
  state.tempAlarmNextStepMs = tempAlarmNextStepMs;
  state.tempAlarmTestUntilMs = tempAlarmTestUntilMs;

  alarmbeeper::AlarmBeeperConfig config{};
  config.toneHighHz = ALARM_TONE_HIGH_HZ;
  config.toneLowHz = ALARM_TONE_LOW_HZ;
  config.ledcChannel = PIEZO_LEDC_CHANNEL;
  config.ledcDuty = PIEZO_LEDC_DUTY;
  config.stepMs = ALARM_BEEP_STEP_MS;
  config.pauseMs = ALARM_BEEP_PAUSE_MS;

  alarmbeeper::updateTemperatureAlarmBeeper(nowMs, state, config);

  tempAlarmBeepActive = state.tempAlarmBeepActive;
  tempAlarmBeepStep = state.tempAlarmBeepStep;
  tempAlarmNextStepMs = state.tempAlarmNextStepMs;
}

float provisioningSetTempC() {
  if (isfinite(setTempC) && setTempC >= PROVISIONING_SETPOINT_MIN_C && setTempC <= PROVISIONING_SETPOINT_MAX_C) {
    return setTempC;
  }
  return DEFAULT_SET_TEMP_C;
}

const char* relayModeText() {
  return settingsparse::relayModeText(static_cast<uint8_t>(relayMode));
}

const char* relayModeApiValue() {
  return settingsparse::relayModeApiValue(static_cast<uint8_t>(relayMode));
}

const char* oledLayoutApiValue() {
  return settingsparse::oledLayoutApiValue(static_cast<uint8_t>(oledLayoutMode));
}

bool parseOledLayoutValue(const String& rawValue, OledLayoutMode& outMode) {
  uint8_t parsed = static_cast<uint8_t>(outMode);
  if (!settingsparse::parseOledLayoutValue(rawValue, parsed)) {
    return false;
  }
  outMode = static_cast<OledLayoutMode>(parsed);
  return true;
}

const char* relayStateText() {
  if (relayOn) {
    return "ON";
  }

  return "OFF";
}

const char* relayStateTextForOled() {
  return relayOn ? "Cooling" : "Off";
}

float tempFromDisplay(float value) {
  return tempconv::tempFromDisplay(value, tempUnitF);
}

float tempToDisplay(float value) {
  return tempconv::tempToDisplay(value, tempUnitF);
}

float deltaToDisplay(float valueC) {
  return tempconv::deltaToDisplay(valueC, tempUnitF);
}

float deltaFromDisplay(float valueDisplay, bool unitIsF) {
  return tempconv::deltaFromDisplay(valueDisplay, unitIsF);
}

void loadHistorySnapshot() {
  historystore::loadSnapshot(preferences);
}

void persistHistorySnapshot(bool force) {
  historystore::persistSnapshot(preferences, force);
}

void recordHistorySample(uint32_t nowMs) {
  historystore::recordSample(nowMs, internalTempC, externalTempC, internalValid, externalValid, relayOn);
}

String buildBlendedHistoryJson(uint32_t windowSeconds, uint32_t requestedStepSeconds, bool includeExternal) {
  return historystore::buildBlendedHistoryJson(windowSeconds, requestedStepSeconds, includeExternal, tempUnitText(), tempToDisplay);
}

void saveSettings() {
  PersistedSettingsData data{};
  data.tempUnitF = tempUnitF;
  data.relayMode = static_cast<uint8_t>(relayMode);
  data.oledLayout = static_cast<uint8_t>(oledLayoutMode);
  data.setTempC = setTempC;
  data.relayOnDeltaC = relayOnDeltaC;
  data.relayOffDeltaC = relayOffDeltaC;
  data.alarmLowTempC = alarmLowTempC;
  data.alarmHighTempC = alarmHighTempC;
  data.tempAlarmEnabled = tempAlarmEnabled;
  data.settingsPasswordEnabled = settingsPasswordEnabled;
  data.settingsPassword = settingsPassword;
  data.relayLockoutMs = relayLockoutMs;
  savePersistedSettings(preferences, data);
}

void loadSettings() {
  PersistedSettingsData defaults{};
  defaults.tempUnitF = true;
  defaults.relayMode = static_cast<uint8_t>(RELAY_MODE_AUTO);
  defaults.oledLayout = static_cast<uint8_t>(OLED_LAYOUT_STANDARD);
  defaults.setTempC = DEFAULT_SET_TEMP_C;
  defaults.relayOnDeltaC = DEFAULT_RELAY_ON_DELTA_C;
  defaults.relayOffDeltaC = DEFAULT_RELAY_OFF_DELTA_C;
  defaults.alarmLowTempC = DEFAULT_ALARM_LOW_C;
  defaults.alarmHighTempC = DEFAULT_ALARM_HIGH_C;
  defaults.tempAlarmEnabled = true;
  defaults.settingsPasswordEnabled = false;
  defaults.settingsPassword = "";
  defaults.relayLockoutMs = DEFAULT_LOCKOUT_MS;

  const PersistedSettingsData loaded = loadPersistedSettings(preferences, defaults);

  tempUnitF = loaded.tempUnitF;
  relayMode = static_cast<RelayMode>(loaded.relayMode);
  oledLayoutMode = static_cast<OledLayoutMode>(loaded.oledLayout);
  setTempC = clampTempC(loaded.setTempC);
  relayOnDeltaC = clampRelayDeltaC(loaded.relayOnDeltaC);
  relayOffDeltaC = clampRelayDeltaC(loaded.relayOffDeltaC);
  alarmLowTempC = clampTempC(loaded.alarmLowTempC);
  alarmHighTempC = clampTempC(loaded.alarmHighTempC);
  tempAlarmEnabled = loaded.tempAlarmEnabled;
  settingsPasswordEnabled = loaded.settingsPasswordEnabled;
  settingsPassword = loaded.settingsPassword;
  if (settingsPasswordEnabled && settingsPassword.length() == 0) {
    settingsPasswordEnabled = false;
  }
  relayLockoutMs = clampRelayLockoutMs(loaded.relayLockoutMs);

  if (relayMode > RELAY_MODE_MANUAL_OFF) {
    relayMode = RELAY_MODE_AUTO;
  }

  if (oledLayoutMode > OLED_LAYOUT_INTERNAL_EXTERNAL) {
    oledLayoutMode = OLED_LAYOUT_STANDARD;
  }

  normalizeSetpointToWholeDisplayUnit();
  normalizeAlarmThresholdsToWholeDisplayUnit();
  clampSetpointToBounds();
}

void setRelay(bool on) {
  relayOn = on;
  digitalWrite(PIN_RELAY, relayOn ? HIGH : LOW);
  if (!relayOn) {
    relayOffSinceMs = millis();
  }
}

bool selectI2cBus(ActiveI2cBus bus) {
  uint8_t activeBus = static_cast<uint8_t>(activeI2cBus);
  const bool ok = i2cbus::selectI2cBus(static_cast<uint8_t>(bus),
                                       activeBus,
                                       i2cInitialized,
                                       PIN_OLED_SDA,
                                       PIN_OLED_SCL,
                                       PIN_HW_SDA,
                                       PIN_HW_SCL);
  activeI2cBus = static_cast<ActiveI2cBus>(activeBus);
  return ok;
}

void scanI2cBus(const char* label) {
  i2cbus::scanI2cBus(label);
}

bool initInternalAht() {
  sensorio::SensorEndpointState state{internalValid, internalTempC, internalHumidity};
  const sensorio::SensorInitConfig config{static_cast<uint8_t>(I2C_BUS_INTERNAL),
                                          "internal GPIO8/9",
                                          "Internal AHT30: I2C bus init failed (GPIO8/9)",
                                          "Internal AHT30: OK (HW I2C GPIO8/9)",
                                          "Internal AHT30: NOT FOUND"};
  return sensorio::initAht(
    ahtInternal, state, config, [](uint8_t bus) { return selectI2cBus(static_cast<ActiveI2cBus>(bus)); }, scanI2cBus);
}

bool initExternalAht() {
  sensorio::SensorEndpointState state{externalValid, externalTempC, externalHumidity};
  const sensorio::SensorInitConfig config{static_cast<uint8_t>(I2C_BUS_OLED),
                                          "shared OLED/external bus GPIO5/6",
                                          "External AHT30: I2C bus init failed (shared OLED bus GPIO5/6)",
                                          "External AHT30: OK (shared OLED bus GPIO5/6)",
                                          "External AHT30: NOT FOUND"};
  return sensorio::initAht(
    ahtExternal, state, config, [](uint8_t bus) { return selectI2cBus(static_cast<ActiveI2cBus>(bus)); }, scanI2cBus);
}

bool readAhtBridge(Adafruit_AHTX0& sensor, uint8_t bus, float& tempC, float& humidity) {
  return sensorio::readAht(
    sensor, bus, tempC, humidity, [](uint8_t activeBus) { return selectI2cBus(static_cast<ActiveI2cBus>(activeBus)); });
}

void applyControl(uint32_t nowMs) {
  const bool provisioningCoolingMode = provisioningMode;
  const float effectiveSetTempC = provisioningCoolingMode ? provisioningSetTempC() : setTempC;
  const RelayMode effectiveRelayMode = provisioningCoolingMode ? RELAY_MODE_AUTO : relayMode;
  relaycontrol::RelayControlInputs inputs{};
  inputs.relayOn = relayOn;
  inputs.internalValid = internalValid;
  inputs.internalTempC = internalTempC;
  inputs.setTempC = effectiveSetTempC;
  inputs.relayOnDeltaC = relayOnDeltaC;
  inputs.relayOffDeltaC = relayOffDeltaC;
  inputs.nowMs = nowMs;
  inputs.relayOffSinceMs = relayOffSinceMs;
  inputs.relayLockoutMs = relayLockoutMs;
  inputs.manualOn = (effectiveRelayMode == RELAY_MODE_MANUAL_ON);
  inputs.manualOff = (effectiveRelayMode == RELAY_MODE_MANUAL_OFF);

  const relaycontrol::RelayAction action = relaycontrol::evaluateRelayAction(inputs);
  switch (action) {
    case relaycontrol::RelayAction::TurnOnManual:
      Serial.println("Relay -> ON (manual lock on)");
      setRelay(true);
      break;
    case relaycontrol::RelayAction::TurnOffManual:
      Serial.println("Relay -> OFF (manual lock off)");
      setRelay(false);
      break;
    case relaycontrol::RelayAction::TurnOffSensorInvalid:
      Serial.println("Relay -> OFF (safe-off: internal sensor invalid)");
      setRelay(false);
      break;
    case relaycontrol::RelayAction::TurnOffLowerThreshold:
      Serial.println("Relay -> OFF (temperature reached lower threshold)");
      setRelay(false);
      break;
    case relaycontrol::RelayAction::TurnOnUpperThreshold:
      Serial.println("Relay -> ON (temperature above upper threshold)");
      setRelay(true);
      break;
    case relaycontrol::RelayAction::None:
    default:
      break;
  }
}

void readSensorsControlAndLog(uint32_t nowMs) {
  SensorPipelineState state{internalValid,
                            externalValid,
                            internalTempC,
                            internalHumidity,
                            externalTempC,
                            externalHumidity,
                            haveSensorReading,
                            lastInternalRetryMs,
                            lastExternalRetryMs};
  SensorPipelineConfig config{};
  config.sensorRetryMs = SENSOR_RETRY_MS;

  ::readSensorsControlAndLog(nowMs,
                             lastSensorMs,
                             SENSOR_PERIOD_MS,
                             state,
                             config,
                             ahtInternal,
                             ahtExternal,
                             static_cast<uint8_t>(I2C_BUS_INTERNAL),
                             static_cast<uint8_t>(I2C_BUS_OLED),
                             initInternalAht,
                             initExternalAht,
                             readAhtBridge,
                             applyControl,
                             recordHistorySample);
}

void updateDisplay(uint32_t nowMs) {
  DisplayRenderInput input{u8g2};
  input.nowMs = nowMs;
  input.provisioningMode = provisioningMode;
  input.staInfoDisplayUntilMs = staInfoDisplayUntilMs;
  input.oledNoticeUntilMs = oledNoticeUntilMs;
  input.oledNoticeText = &oledNoticeText;
  input.haveSensorReading = haveSensorReading;
  input.setpointFocusUntilMs = setpointFocusUntilMs;
  input.tempAlarmBeepActive = tempAlarmBeepActive;
  input.tempAlarmTestUntilMs = tempAlarmTestUntilMs;
  input.setTempC = setTempC;
  input.internalTempC = internalTempC;
  input.externalTempC = externalTempC;
  input.relayOn = relayOn;
  input.oledLayoutMode = static_cast<int>(oledLayoutMode);
  input.oledXOffset = OLED_X_OFFSET;
  input.oledYOffset = OLED_Y_OFFSET;
  input.apSsid = AP_SSID;
  input.relayStateText = relayStateTextForOled();
  input.tempUnitTextFn = tempUnitText;
  input.tempToDisplayFn = tempToDisplay;
  renderDisplay(input);
}

void handleSetpointButtons(uint32_t nowMs) {
  setpointadjust::SetpointButtonState buttonState{
    upStable, upLastRead, upDebounceStartMs, upNextRepeatMs, downStable, downLastRead, downDebounceStartMs, downNextRepeatMs};

  setpointadjust::SetpointAdjustState adjustState{tempUnitF, setTempC, setpointFocusUntilMs};

  setpointadjust::SetpointAdjustConfig config{};
  config.pinBtnUp = PIN_BTN_UP;
  config.pinBtnDown = PIN_BTN_DOWN;
  config.targetStepC = TARGET_STEP_C;
  config.buttonDebounceMs = BUTTON_DEBOUNCE_MS;
  config.buttonHoldRepeatMs = BUTTON_HOLD_REPEAT_MS;
  config.setpointFocusDisplayMs = SETPOINT_FOCUS_DISPLAY_MS;

  setpointadjust::handleSetpointButtons(
    nowMs, buttonState, adjustState, config, normalizeSetpointToWholeDisplayUnit, saveSettings, applyControl);
}

String makeStatusJson(uint32_t nowMs) {
  (void)nowMs;
  StatusJsonInput input{};
  input.tempUnit = tempUnitText();
  input.internalTemp = tempToDisplay(internalTempC);
  input.internalHumidity = internalHumidity;
  input.externalTemp = tempToDisplay(externalTempC);
  input.externalHumidity = externalHumidity;
  input.relayState = relayStateText();
  input.relayMode = relayModeText();
  input.relayModeId = static_cast<uint8_t>(relayMode);
  input.alarmEnabled = tempAlarmEnabled;
  input.alarmActive = internalTempOutsideAlarmThresholds();
  input.setTemp = tempToDisplay(setTempC);
  input.relayOnDelta = deltaToDisplay(relayOnDeltaC);
  input.relayOffDelta = deltaToDisplay(relayOffDeltaC);
  input.minOffSeconds = relayLockoutMs / 1000U;
  return buildStatusJson(input);
}

String makeStatusJsonNow() {
  return makeStatusJson(millis());
}

String makeSettingsJson() {
  SettingsJsonInput input{};
  input.tempUnit = tempUnitText();
  input.relayMode = relayModeApiValue();
  input.oledLayout = oledLayoutApiValue();
  input.setTemp = tempToDisplay(setTempC);
  input.settingsAuthEnabled = settingsPasswordEnabled;
  input.alarmEnabled = tempAlarmEnabled;
  input.alarmLow = tempToDisplay(alarmLowTempC);
  input.alarmHigh = tempToDisplay(alarmHighTempC);
  input.relayOnDelta = deltaToDisplay(relayOnDeltaC);
  input.relayOffDelta = deltaToDisplay(relayOffDeltaC);
  input.minOffSeconds = relayLockoutMs / 1000U;
  return buildSettingsJson(input, false);
}

NetworkWorkflowState makeNetworkWorkflowState() {
  return NetworkWorkflowState{provisioningMode, staSsid, staPass, dnsServer};
}

NetworkWorkflowConfig makeNetworkWorkflowConfig() {
  NetworkWorkflowConfig config{};
  config.apSsid = AP_SSID;
  config.dnsPort = DNS_PORT;
  config.apChannel = AP_CHANNEL;
  config.apHidden = AP_HIDDEN;
  config.apMaxClients = AP_MAX_CLIENTS;
  config.staConnectTimeoutMs = STA_CONNECT_TIMEOUT_MS;
  config.staConnectAttempts = STA_CONNECT_ATTEMPTS;
  return config;
}

void startReconfigureAction() {
  ::startProvisioningAp(makeNetworkWorkflowState(), makeNetworkWorkflowConfig(), "manual reconfigure");
}

void startAlarmTestAction() {
  tempAlarmTestUntilMs = millis() + ALARM_TEST_DURATION_MS;
  tempAlarmBeepActive = false;
  tempAlarmBeepStep = 0;
  tempAlarmNextStepMs = 0;
}

void markPendingStaRebootAction() {
  pendingStaReboot = true;
}

void loadStaCredentials() {
  ::loadStaCredentials(preferences, staSsid, staPass);
}

void saveStaCredentials(const String& ssid, const String& password) {
  ::saveStaCredentials(preferences, staSsid, staPass, ssid, password);
}

bool hasStoredStaCredentials() {
  return ::hasStoredStaCredentials(staSsid);
}

void clearSettingsPasswordAndNotice(uint32_t nowMs) {
  settingsPasswordEnabled = false;
  settingsPassword = String();
  ::refreshSettingsAuthToken(settingsPasswordEnabled, settingsAuthToken);
  saveSettings();
  oledNoticeText = "PW Cleared";
  oledNoticeUntilMs = nowMs + 3000U;
}

void handleApiSettingsPost(AsyncWebServerRequest* request) {
  if (!::settingsAuthSatisfied(settingsPasswordEnabled,
                               settingsAuthToken,
                               request,
                               SETTINGS_AUTH_COOKIE_NAME)) {
    request->send(401, "application/json", "{\"ok\":false,\"error\":\"auth_required\"}");
    return;
  }

  SettingsRequestDefaults requestDefaults{};
  requestDefaults.settingsAuthEnabled = settingsPasswordEnabled;
  requestDefaults.alarmEnabled = tempAlarmEnabled;
  requestDefaults.alarmLowDisplay = tempToDisplay(alarmLowTempC);
  requestDefaults.alarmHighDisplay = tempToDisplay(alarmHighTempC);
  requestDefaults.relayOnDeltaDisplay = deltaToDisplay(relayOnDeltaC);
  requestDefaults.relayOffDeltaDisplay = deltaToDisplay(relayOffDeltaC);
  requestDefaults.minOffSeconds = relayLockoutMs / 1000U;
  requestDefaults.currentlyPasswordEnabled = settingsPasswordEnabled;

  ParsedSettingsRequest parsed{};
  String parseError;
  if (!parseSettingsPostRequest(request, requestDefaults, parsed, parseError)) {
    request->send(400,
                  "application/json",
                  String("{\"ok\":false,\"error\":\"") + parseError + "\"}");
    return;
  }

  uint8_t parsedRelayMode = 0;
  if (!settingsparse::parseRelayModeValue(parsed.mode, parsedRelayMode)) {
    request->send(400, "application/json", "{\"ok\":false,\"error\":\"invalid_mode\"}");
    return;
  }
  RelayMode nextMode = static_cast<RelayMode>(parsedRelayMode);

  OledLayoutMode nextOledLayout = OLED_LAYOUT_STANDARD;
  if (!parseOledLayoutValue(parsed.oledLayoutValue, nextOledLayout)) {
    request->send(400, "application/json", "{\"ok\":false,\"error\":\"invalid_oled_layout\"}");
    return;
  }

  const bool nextUnitF = (parsed.unit == "F");
  auto postedDisplayToC = [nextUnitF](float value) {
    return nextUnitF ? ((value - 32.0f) * 5.0f / 9.0f) : value;
  };
  const float nextSetC = postedDisplayToC(parsed.setDisplay);
  const float nextAlarmLowC = postedDisplayToC(roundf(parsed.alarmLowDisplay));
  const float nextAlarmHighC = postedDisplayToC(roundf(parsed.alarmHighDisplay));
  const float nextOnDeltaC = deltaFromDisplay(parsed.onDeltaDisplay, nextUnitF);
  const float nextOffDeltaC = deltaFromDisplay(parsed.offDeltaDisplay, nextUnitF);

  if (!isfinite(nextSetC) || !isfinite(nextAlarmLowC) || !isfinite(nextAlarmHighC) || !isfinite(nextOnDeltaC) || !isfinite(nextOffDeltaC)) {
    request->send(400, "application/json", "{\"ok\":false,\"error\":\"invalid_temperature_range\"}");
    return;
  }

  if (nextAlarmLowC >= nextAlarmHighC) {
    request->send(400, "application/json", "{\"ok\":false,\"error\":\"invalid_alarm_range\"}");
    return;
  }

  const bool passwordFieldsProvided =
    parsed.nextSettingsPassword.length() > 0 || parsed.nextSettingsPasswordConfirm.length() > 0;

  relayMode = nextMode;
  oledLayoutMode = nextOledLayout;
  SettingsApplyParams applyParams{};
  applyParams.nextUnitF = nextUnitF;
  applyParams.nextSetC = nextSetC;
  applyParams.nextAlarmLowC = nextAlarmLowC;
  applyParams.nextAlarmHighC = nextAlarmHighC;
  applyParams.nextAlarmEnabled = parsed.nextAlarmEnabled;
  applyParams.nextSettingsAuthEnabled = parsed.nextSettingsAuthEnabled;
  applyParams.nextSettingsPassword = parsed.nextSettingsPassword;
  applyParams.passwordFieldsProvided = passwordFieldsProvided;
  applyParams.nextOnDeltaC = nextOnDeltaC;
  applyParams.nextOffDeltaC = nextOffDeltaC;
  applyParams.minOffSeconds = parsed.minOffSeconds;
  applyParams.nowMs = millis();

  SettingsApplyState applyState{tempUnitF,
                                setTempC,
                                alarmLowTempC,
                                alarmHighTempC,
                                tempAlarmEnabled,
                                settingsPasswordEnabled,
                                settingsPassword,
                                settingsAuthToken,
                                relayOnDeltaC,
                                relayOffDeltaC,
                                relayLockoutMs};

  applySettingsUpdate(applyParams,
                      applyState,
                      clampTempC,
                      clampRelayDeltaC,
                      clampRelayLockoutMs,
                      normalizeSetpointToWholeDisplayUnit,
                      normalizeAlarmThresholdsToWholeDisplayUnit,
                      clampSetpointToBounds,
                      saveSettings,
                      applyControl);

  SettingsJsonInput responseInput{};
  responseInput.tempUnit = tempUnitText();
  responseInput.relayMode = relayModeApiValue();
  responseInput.oledLayout = oledLayoutApiValue();
  responseInput.setTemp = tempToDisplay(setTempC);
  responseInput.settingsAuthEnabled = settingsPasswordEnabled;
  responseInput.alarmEnabled = tempAlarmEnabled;
  responseInput.alarmLow = tempToDisplay(alarmLowTempC);
  responseInput.alarmHigh = tempToDisplay(alarmHighTempC);
  responseInput.relayOnDelta = deltaToDisplay(relayOnDeltaC);
  responseInput.relayOffDelta = deltaToDisplay(relayOffDeltaC);
  responseInput.minOffSeconds = relayLockoutMs / 1000U;
  AsyncWebServerResponse* finalResponse =
    request->beginResponse(200, "application/json", buildSettingsJson(responseInput, true));
  ::addSettingsAuthCookie(settingsPasswordEnabled,
                          settingsAuthToken,
                          finalResponse,
                          SETTINGS_AUTH_COOKIE_NAME,
                          SETTINGS_AUTH_COOKIE_TTL_SEC);
  request->send(finalResponse);
}

void setupWeb() {
  auto authorizeSettingsRequest = [](AsyncWebServerRequest* request) -> bool {
    return ::settingsAuthSatisfied(settingsPasswordEnabled,
                                   settingsAuthToken,
                                   request,
                                   SETTINGS_AUTH_COOKIE_NAME);
  };

  auto authenticateSettingsPasswordFn = [](const String& password) -> bool {
    return ::authenticateSettingsPassword(settingsPasswordEnabled, settingsPassword, password);
  };

  auto addSettingsAuthCookieFn = [](AsyncWebServerResponse* response) {
    ::addSettingsAuthCookie(settingsPasswordEnabled,
                            settingsAuthToken,
                            response,
                            SETTINGS_AUTH_COOKIE_NAME,
                            SETTINGS_AUTH_COOKIE_TTL_SEC);
  };

  auto refreshSettingsAuthTokenFn = []() {
    ::refreshSettingsAuthToken(settingsPasswordEnabled, settingsAuthToken);
  };

  auto redirectToPortal = [](AsyncWebServerRequest* request) {
    // Connectivity probe endpoints are best handled as redirects for OS captive agents.
    if (ENABLE_CAPTIVE_DEBUG_LOGS) {
      const String host = request->hasHeader("Host") ? request->getHeader("Host")->value() : String("(none)");
      Serial.printf("[CAPTIVE] redirect uri=%s host=%s\n", request->url().c_str(), host.c_str());
    }
    request->redirect("http://192.168.4.1/");
  };

  auto sendCaptiveProbePage = [](AsyncWebServerRequest* request) {
    if (ENABLE_CAPTIVE_DEBUG_LOGS) {
      const String host = request->hasHeader("Host") ? request->getHeader("Host")->value() : String("(none)");
      Serial.printf("[CAPTIVE] probe-page uri=%s host=%s\n", request->url().c_str(), host.c_str());
    }
    AsyncWebServerResponse* response = request->beginResponse(200, "text/html", CAPTIVE_HTML);
    response->addHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    response->addHeader("Pragma", "no-cache");
    request->send(response);
  };

  auto appleProbeToPortal = [](AsyncWebServerRequest* request) {
    // iOS/macOS captive assistant is more reliable when probe returns a small non-success body.
    if (ENABLE_CAPTIVE_DEBUG_LOGS) {
      const String host = request->hasHeader("Host") ? request->getHeader("Host")->value() : String("(none)");
      Serial.printf("[CAPTIVE] apple-probe uri=%s host=%s\n", request->url().c_str(), host.c_str());
    }
    AsyncWebServerResponse* response = request->beginResponse(200, "text/html", CAPTIVE_HTML);
    response->addHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    response->addHeader("Pragma", "no-cache");
    request->send(response);
  };

  registerRootRoute(webServer, provisioningMode, shouldCaptiveRedirect, PROVISION_HTML, INDEX_HTML);

  registerApiProvisionRoute(webServer,
                            provisioningMode,
                            saveStaCredentials,
                            markPendingStaRebootAction);

  registerApiReconfigureRoute(webServer, provisioningMode, authorizeSettingsRequest, startReconfigureAction);

  registerApiAlarmTestRoute(webServer, provisioningMode, authorizeSettingsRequest, startAlarmTestAction);

  registerCaptiveProbeRoutes(webServer,
                             provisioningMode,
                             sendCaptiveProbePage,
                             appleProbeToPortal,
                             redirectToPortal);

  registerSettingsPageRoute(webServer,
                            provisioningMode,
                            authorizeSettingsRequest,
                            SETTINGS_AUTH_HTML,
                            SETTINGS_HTML);

  registerHistoryPageRoute(webServer, provisioningMode, HISTORY_HTML);

  registerApiStatusRoute(webServer, makeStatusJsonNow);

  registerApiHistoryRoute(webServer, buildBlendedHistoryJson);

  registerApiSettingsGetRoute(webServer, authorizeSettingsRequest, makeSettingsJson);

  registerApiSettingsLoginRoute(webServer,
                                provisioningMode,
                                settingsPasswordEnabled,
                                settingsAuthToken,
                                authenticateSettingsPasswordFn,
                                addSettingsAuthCookieFn,
                                refreshSettingsAuthTokenFn);

  registerApiSettingsPostRoute(webServer, handleApiSettingsPost);

  registerNotFoundRoute(webServer, provisioningMode);

  webServer.begin();
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);

  pinMode(PIN_PIEZO, OUTPUT);
  ledcSetup(PIEZO_LEDC_CHANNEL, 2000, PIEZO_LEDC_RESOLUTION_BITS);
  ledcAttachPin(PIN_PIEZO, PIEZO_LEDC_CHANNEL);
  ledcWriteTone(PIEZO_LEDC_CHANNEL, 0);
  ledcWrite(PIEZO_LEDC_CHANNEL, 0);

  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);
  upStable = digitalRead(PIN_BTN_UP);
  upLastRead = upStable;
  downStable = digitalRead(PIN_BTN_DOWN);
  downLastRead = downStable;
  upDebounceStartMs = millis();
  downDebounceStartMs = upDebounceStartMs;
  upNextRepeatMs = 0;
  downNextRepeatMs = 0;
  initPasswordResetGestureState(passwordResetGesture, upStable, downStable, upDebounceStartMs);

  relayOffSinceMs = millis();

  preferences.begin("snowleopard", false);
  const uint32_t historyBootSequence = preferences.getUInt("hist_boot_seq", 0U) + 1U;
  preferences.putUInt("hist_boot_seq", historyBootSequence);
  historystore::begin(historyBootSequence);
  loadSettings();
  ::refreshSettingsAuthToken(settingsPasswordEnabled, settingsAuthToken);
  loadHistorySnapshot();
  loadStaCredentials();

  historystore::primeTimers(millis());

  u8g2.begin();
  u8g2.setBusClock(400000);
  u8g2.setPowerSave(0);
  u8g2.setContrast(255);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(OLED_X_OFFSET, OLED_Y_OFFSET + 8, "Booting...");
  u8g2.drawStr(OLED_X_OFFSET, OLED_Y_OFFSET + 18, "Starting...");
  u8g2.sendBuffer();

  initInternalAht();
  initExternalAht();

  // Prime the first telemetry sample immediately so status/OLED are not delayed by Wi-Fi connect attempts.
  lastSensorMs = millis() - SENSOR_PERIOD_MS;
  readSensorsControlAndLog(millis());
  updateDisplay(millis());

  if (hasStoredStaCredentials() &&
      ::connectStaWithRetry(makeNetworkWorkflowState(), makeNetworkWorkflowConfig(), "boot")) {
    provisioningMode = false;
    MDNS.begin("snowleopard");
    MDNS.addService("http", "tcp", 80);
    staInfoDisplayUntilMs = millis() + 5000;
  } else {
    ::startProvisioningAp(makeNetworkWorkflowState(), makeNetworkWorkflowConfig(), "boot/no valid STA connection");
  }

  // Start HTTP routes only after a network interface (AP or STA) is active.
  setupWeb();

  Serial.println("System ready");
}

void loop() {
  const uint32_t nowMs = millis();

  if (provisioningMode) {
    dnsServer.processNextRequest();
  }

  if (pendingStaReboot) {
    pendingStaReboot = false;
    Serial.println("Provisioning saved, rebooting into STA startup path");
    persistHistorySnapshot(true);
    delay(150);
    ESP.restart();
  }

  if (!provisioningMode) {
    if (WiFi.status() == WL_CONNECTED) {
      staDisconnectedSinceMs = 0;
    } else {
      if (staDisconnectedSinceMs == 0) {
        staDisconnectedSinceMs = nowMs;
        Serial.println("STA disconnected; waiting for auto-reconnect before AP fallback");
      } else if ((nowMs - staDisconnectedSinceMs) >= STA_FALLBACK_TO_AP_MS) {
        ::startProvisioningAp(makeNetworkWorkflowState(), makeNetworkWorkflowConfig(), "STA disconnected too long");
      }
    }
  }

  bool resetTriggered = false;
  const bool resetGestureActive = handlePasswordResetGesture(passwordResetGesture,
                                                             nowMs,
                                                             PIN_BTN_UP,
                                                             PIN_BTN_DOWN,
                                                             BUTTON_DEBOUNCE_MS,
                                                             5000U,
                                                             resetTriggered);
  if (resetTriggered) {
    clearSettingsPasswordAndNotice(nowMs);
  }
  if (!resetGestureActive) {
    handleSetpointButtons(nowMs);
  }

  readSensorsControlAndLog(nowMs);
  updateTemperatureAlarmBeeper(nowMs);
  persistHistorySnapshot(false);
  updateDisplay(nowMs);
}
