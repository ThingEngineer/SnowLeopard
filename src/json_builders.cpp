#include "json_builders.h"

namespace {

String floatOrNull(float value, uint8_t decimals = 2) {
  if (isnan(value)) {
    return "null";
  }
  return String(value, static_cast<unsigned int>(decimals));
}

}  // namespace

String buildStatusJson(const StatusJsonInput& input) {
  String s = "{";
  s += "\"temp_unit\":\"" + String(input.tempUnit) + "\"";
  s += ",\"internal_temp\":" + floatOrNull(input.internalTemp, 2);
  s += ",\"internal_humidity\":" + floatOrNull(input.internalHumidity, 2);
  s += ",\"external_temp\":" + floatOrNull(input.externalTemp, 2);
  s += ",\"external_humidity\":" + floatOrNull(input.externalHumidity, 2);
  s += ",\"relay_state\":\"" + String(input.relayState) + "\"";
  s += ",\"relay_mode\":\"" + String(input.relayMode) + "\"";
  s += ",\"relay_mode_id\":" + String(static_cast<unsigned>(input.relayModeId));
  s += ",\"alarm_enabled\":" + String(input.alarmEnabled ? "true" : "false");
  s += ",\"alarm_active\":" + String(input.alarmActive ? "true" : "false");
  s += ",\"set_temp\":" + String(input.setTemp, 0);
  s += ",\"relay_on_delta\":" + String(input.relayOnDelta, 2);
  s += ",\"relay_off_delta\":" + String(input.relayOffDelta, 2);
  s += ",\"min_off_seconds\":" + String(input.minOffSeconds);
  s += "}";
  return s;
}

String buildSettingsJson(const SettingsJsonInput& input, bool includeOk) {
  String s = "{";
  if (includeOk) {
    s += "\"ok\":true,";
  }
  s += "\"temp_unit\":\"" + String(input.tempUnit) + "\"";
  s += ",\"relay_mode\":\"" + String(input.relayMode) + "\"";
  s += ",\"oled_layout\":\"" + String(input.oledLayout) + "\"";
  s += ",\"set_temp\":" + String(input.setTemp, 0);
  s += ",\"settings_auth_enabled\":" + String(input.settingsAuthEnabled ? "true" : "false");
  s += ",\"buttons_enabled\":" + String(input.buttonsEnabled ? "true" : "false");
  s += ",\"alarm_enabled\":" + String(input.alarmEnabled ? "true" : "false");
  s += ",\"alarm_low\":" + String(input.alarmLow, 0);
  s += ",\"alarm_high\":" + String(input.alarmHigh, 0);
  s += ",\"relay_on_delta\":" + String(input.relayOnDelta, 2);
  s += ",\"relay_off_delta\":" + String(input.relayOffDelta, 2);
  s += ",\"internal_temp_offset_f\":" + floatOrNull(input.internalTempOffsetF, 2);
  s += ",\"external_temp_offset_f\":" + floatOrNull(input.externalTempOffsetF, 2);
  s += ",\"min_off_seconds\":" + String(input.minOffSeconds);
  s += "}";
  return s;
}
