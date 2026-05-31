#pragma once

#include <stdint.h>

namespace i2cbus {

constexpr uint8_t BUS_OLED = 0;
constexpr uint8_t BUS_INTERNAL = 1;

bool selectI2cBus(uint8_t requestedBus,
                  uint8_t& activeBus,
                  bool& i2cInitialized,
                  uint8_t pinOledSda,
                  uint8_t pinOledScl,
                  uint8_t pinHwSda,
                  uint8_t pinHwScl);

void scanI2cBus(const char* label);

}  // namespace i2cbus
