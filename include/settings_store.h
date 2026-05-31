#pragma once

#include <Preferences.h>

#include <stdint.h>

struct PersistedSettingsData {
  bool tempUnitF;
  uint8_t relayMode;
  uint8_t oledLayout;
  float setTempC;
  float relayOnDeltaC;
  float relayOffDeltaC;
  float alarmLowTempC;
  float alarmHighTempC;
  bool tempAlarmEnabled;
  bool settingsPasswordEnabled;
  String settingsPassword;
  uint32_t relayLockoutMs;
};

void savePersistedSettings(Preferences& preferences, const PersistedSettingsData& data);
PersistedSettingsData loadPersistedSettings(Preferences& preferences, const PersistedSettingsData& defaults);
