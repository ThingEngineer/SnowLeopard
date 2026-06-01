#pragma once

#include <Arduino.h>

struct StatusJsonInput {
  const char* tempUnit = "F";
  float internalTemp = NAN;
  float internalHumidity = NAN;
  float externalTemp = NAN;
  float externalHumidity = NAN;
  const char* relayState = "OFF";
  const char* relayMode = "AUTO";
  uint8_t relayModeId = 0;
  bool alarmEnabled = true;
  bool alarmActive = false;
  float setTemp = NAN;
  float relayOnDelta = NAN;
  float relayOffDelta = NAN;
  uint32_t minOffSeconds = 0;
};

struct SettingsJsonInput {
  const char* tempUnit = "F";
  const char* relayMode = "auto";
  const char* oledLayout = "standard";
  float setTemp = NAN;
  bool settingsAuthEnabled = false;
  bool buttonsEnabled = true;
  bool alarmEnabled = true;
  float alarmLow = NAN;
  float alarmHigh = NAN;
  float relayOnDelta = NAN;
  float relayOffDelta = NAN;
  float internalTempOffsetF = NAN;
  float externalTempOffsetF = NAN;
  uint32_t minOffSeconds = 0;
};

String buildStatusJson(const StatusJsonInput& input);
String buildSettingsJson(const SettingsJsonInput& input, bool includeOk);
