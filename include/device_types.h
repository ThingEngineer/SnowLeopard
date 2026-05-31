#pragma once

#include <stdint.h>

enum ActiveI2cBus : uint8_t {
  I2C_BUS_OLED = 0,
  I2C_BUS_INTERNAL = 1,
};

enum TempUnit : uint8_t {
  TEMP_UNIT_C = 0,
  TEMP_UNIT_F = 1,
};

enum RelayMode : uint8_t {
  RELAY_MODE_AUTO = 0,
  RELAY_MODE_MANUAL_ON = 1,
  RELAY_MODE_MANUAL_OFF = 2,
};

enum OledLayoutMode : uint8_t {
  OLED_LAYOUT_STANDARD = 0,
  OLED_LAYOUT_SETPOINT_ONLY = 1,
  OLED_LAYOUT_SETPOINT_INTERNAL = 2,
  OLED_LAYOUT_SETPOINT_INTERNAL_RELAY = 3,
  OLED_LAYOUT_INTERNAL_EXTERNAL = 4,
};
