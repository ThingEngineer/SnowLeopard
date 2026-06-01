#include "settings_store.h"

void savePersistedSettings(Preferences& preferences, const PersistedSettingsData& data) {
  preferences.putBool("temp_unit_f", data.tempUnitF);
  preferences.putUChar("relay_mode", data.relayMode);
  preferences.putUChar("oled_layout", data.oledLayout);
  preferences.putFloat("set_temp_c", data.setTempC);
  preferences.putFloat("relay_on_d_c", data.relayOnDeltaC);
  preferences.putFloat("relay_off_d_c", data.relayOffDeltaC);
  preferences.putFloat("alarm_low_c", data.alarmLowTempC);
  preferences.putFloat("alarm_high_c", data.alarmHighTempC);
  preferences.putBool("alarm_en", data.tempAlarmEnabled);
  preferences.putBool("buttons_en", data.buttonsEnabled);
  preferences.putBool("set_auth_en", data.settingsPasswordEnabled);
  preferences.putString("set_auth_pw", data.settingsPassword);
  preferences.putFloat("in_t_ofs_f", data.internalTempOffsetF);
  preferences.putFloat("ex_t_ofs_f", data.externalTempOffsetF);
  preferences.putUInt("relay_lock_ms", data.relayLockoutMs);
}

PersistedSettingsData loadPersistedSettings(Preferences& preferences, const PersistedSettingsData& defaults) {
  PersistedSettingsData loaded = defaults;
  loaded.tempUnitF = preferences.getBool("temp_unit_f", defaults.tempUnitF);
  loaded.relayMode = preferences.getUChar("relay_mode", defaults.relayMode);
  loaded.oledLayout = preferences.getUChar("oled_layout", defaults.oledLayout);
  loaded.setTempC = preferences.getFloat("set_temp_c", defaults.setTempC);
  loaded.relayOnDeltaC = preferences.getFloat("relay_on_d_c", defaults.relayOnDeltaC);
  loaded.relayOffDeltaC = preferences.getFloat("relay_off_d_c", defaults.relayOffDeltaC);
  loaded.alarmLowTempC = preferences.getFloat("alarm_low_c", defaults.alarmLowTempC);
  loaded.alarmHighTempC = preferences.getFloat("alarm_high_c", defaults.alarmHighTempC);
  loaded.tempAlarmEnabled = preferences.getBool("alarm_en", defaults.tempAlarmEnabled);
  loaded.buttonsEnabled = preferences.getBool("buttons_en", defaults.buttonsEnabled);
  loaded.settingsPasswordEnabled = preferences.getBool("set_auth_en", defaults.settingsPasswordEnabled);
  loaded.settingsPassword = preferences.isKey("set_auth_pw")
                              ? preferences.getString("set_auth_pw", defaults.settingsPassword)
                              : defaults.settingsPassword;
  loaded.internalTempOffsetF = preferences.getFloat("in_t_ofs_f", defaults.internalTempOffsetF);
  loaded.externalTempOffsetF = preferences.getFloat("ex_t_ofs_f", defaults.externalTempOffsetF);
  loaded.relayLockoutMs = preferences.getUInt("relay_lock_ms", defaults.relayLockoutMs);
  return loaded;
}
