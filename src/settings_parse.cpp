#include "settings_parse.h"

namespace settingsparse {

const char* relayModeText(uint8_t relayMode) {
  switch (relayMode) {
    case RELAY_MODE_MANUAL_ON:
      return "MAN ON";
    case RELAY_MODE_MANUAL_OFF:
      return "MAN OFF";
    default:
      return "AUTO";
  }
}

const char* relayModeApiValue(uint8_t relayMode) {
  switch (relayMode) {
    case RELAY_MODE_MANUAL_ON:
      return "manual_on";
    case RELAY_MODE_MANUAL_OFF:
      return "manual_off";
    default:
      return "auto";
  }
}

bool parseRelayModeValue(const String& rawValue, uint8_t& outMode) {
  String value = rawValue;
  value.trim();
  value.toLowerCase();

  if (value == "manual_on" || value == "man on" || value == "manual locked on") {
    outMode = RELAY_MODE_MANUAL_ON;
    return true;
  }
  if (value == "manual_off" || value == "man off" || value == "manual locked off") {
    outMode = RELAY_MODE_MANUAL_OFF;
    return true;
  }
  if (value == "auto") {
    outMode = RELAY_MODE_AUTO;
    return true;
  }

  return false;
}

const char* oledLayoutApiValue(uint8_t oledLayoutMode) {
  switch (oledLayoutMode) {
    case OLED_LAYOUT_SETPOINT_ONLY:
      return "setpoint_only";
    case OLED_LAYOUT_SETPOINT_INTERNAL:
      return "setpoint_internal";
    case OLED_LAYOUT_SETPOINT_INTERNAL_RELAY:
      return "setpoint_internal_relay";
    case OLED_LAYOUT_INTERNAL_EXTERNAL:
      return "internal_external";
    case OLED_LAYOUT_STANDARD:
    default:
      return "standard";
  }
}

bool parseOledLayoutValue(const String& rawValue, uint8_t& outMode) {
  String value = rawValue;
  value.trim();
  value.toLowerCase();

  if (value == "standard") {
    outMode = OLED_LAYOUT_STANDARD;
    return true;
  }
  if (value == "setpoint_only") {
    outMode = OLED_LAYOUT_SETPOINT_ONLY;
    return true;
  }
  if (value == "setpoint_internal") {
    outMode = OLED_LAYOUT_SETPOINT_INTERNAL;
    return true;
  }
  if (value == "setpoint_internal_relay") {
    outMode = OLED_LAYOUT_SETPOINT_INTERNAL_RELAY;
    return true;
  }
  if (value == "internal_external") {
    outMode = OLED_LAYOUT_INTERNAL_EXTERNAL;
    return true;
  }

  return false;
}

}  // namespace settingsparse
