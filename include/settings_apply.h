#pragma once

#include <Arduino.h>

struct SettingsApplyParams {
  bool nextUnitF = true;
  float nextSetC = 0.0f;
  float nextAlarmLowC = 0.0f;
  float nextAlarmHighC = 0.0f;
  bool nextAlarmEnabled = true;
  bool nextSettingsAuthEnabled = false;
  String nextSettingsPassword;
  bool passwordFieldsProvided = false;
  float nextOnDeltaC = 0.0f;
  float nextOffDeltaC = 0.0f;
  float nextInternalTempOffsetF = 0.0f;
  float nextExternalTempOffsetF = 0.0f;
  uint32_t minOffSeconds = 0;
  uint32_t nowMs = 0;
};

struct SettingsApplyState {
  bool& tempUnitF;
  float& setTempC;
  float& alarmLowTempC;
  float& alarmHighTempC;
  bool& tempAlarmEnabled;
  bool& settingsPasswordEnabled;
  String& settingsPassword;
  String& settingsAuthToken;
  float& relayOnDeltaC;
  float& relayOffDeltaC;
  float& internalTempOffsetF;
  float& externalTempOffsetF;
  uint32_t& relayLockoutMs;
};

void applySettingsUpdate(const SettingsApplyParams& params,
                         SettingsApplyState state,
                         float (*clampTempC)(float),
                         float (*clampRelayDeltaC)(float),
                         uint32_t (*clampRelayLockoutMs)(uint32_t),
                         void (*normalizeSetpointToWholeDisplayUnit)(),
                         void (*normalizeAlarmThresholdsToWholeDisplayUnit)(),
                         void (*clampSetpointToBounds)(),
                         void (*saveSettings)(),
                         void (*applyControl)(uint32_t));
