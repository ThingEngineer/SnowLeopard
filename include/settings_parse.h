#pragma once

#include <Arduino.h>

namespace settingsparse {

static constexpr uint8_t RELAY_MODE_AUTO = 0;
static constexpr uint8_t RELAY_MODE_MANUAL_ON = 1;
static constexpr uint8_t RELAY_MODE_MANUAL_OFF = 2;

static constexpr uint8_t OLED_LAYOUT_STANDARD = 0;
static constexpr uint8_t OLED_LAYOUT_SETPOINT_ONLY = 1;
static constexpr uint8_t OLED_LAYOUT_SETPOINT_INTERNAL = 2;
static constexpr uint8_t OLED_LAYOUT_SETPOINT_INTERNAL_RELAY = 3;
static constexpr uint8_t OLED_LAYOUT_INTERNAL_EXTERNAL = 4;

const char* relayModeText(uint8_t relayMode);
const char* relayModeApiValue(uint8_t relayMode);
bool parseRelayModeValue(const String& rawValue, uint8_t& outMode);

const char* oledLayoutApiValue(uint8_t oledLayoutMode);
bool parseOledLayoutValue(const String& rawValue, uint8_t& outMode);

}  // namespace settingsparse
