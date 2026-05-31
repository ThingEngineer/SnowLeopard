#pragma once

#include <Arduino.h>
#include <Adafruit_AHTX0.h>

namespace sensorio {

struct SensorEndpointState {
  bool& valid;
  float& tempC;
  float& humidity;
};

struct SensorInitConfig {
  uint8_t bus;
  const char* scanLabel;
  const char* busInitFailedMsg;
  const char* okMsg;
  const char* notFoundMsg;
};

using SelectI2cBusFn = bool (*)(uint8_t bus);
using ScanI2cBusFn = void (*)(const char* label);

bool initAht(Adafruit_AHTX0& sensor,
             SensorEndpointState state,
             const SensorInitConfig& config,
             SelectI2cBusFn selectI2cBus,
             ScanI2cBusFn scanI2cBus);

bool readAht(Adafruit_AHTX0& sensor,
             uint8_t bus,
             float& tempC,
             float& humidity,
             SelectI2cBusFn selectI2cBus);

}  // namespace sensorio
