#include "i2c_bus.h"

#include <Arduino.h>
#include <Wire.h>

namespace i2cbus {

bool selectI2cBus(uint8_t requestedBus,
                  uint8_t& activeBus,
                  bool& i2cInitialized,
                  uint8_t pinOledSda,
                  uint8_t pinOledScl,
                  uint8_t pinHwSda,
                  uint8_t pinHwScl) {
  if (i2cInitialized && activeBus == requestedBus) {
    return true;
  }

  uint8_t sda = pinOledSda;
  uint8_t scl = pinOledScl;

  if (requestedBus == BUS_INTERNAL) {
    sda = pinHwSda;
    scl = pinHwScl;
  }

  if (i2cInitialized) {
    Wire.end();
  }

  if (!Wire.begin(sda, scl, 100000U)) {
    Serial.printf("I2C bus switch failed: SDA=%u SCL=%u\n", sda, scl);
    return false;
  }

  activeBus = requestedBus;
  i2cInitialized = true;
  return true;
}

void scanI2cBus(const char* label) {
  uint8_t foundCount = 0;
  Serial.printf("I2C scan (%s):", label);
  for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf(" 0x%02X", addr);
      foundCount++;
    }
  }

  if (foundCount == 0) {
    Serial.print(" none");
  }
  Serial.println();
}

}  // namespace i2cbus
