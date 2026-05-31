#pragma once

#include <Arduino.h>
#include <Adafruit_AHTX0.h>

struct SensorPipelineState {
  bool& internalValid;
  bool& externalValid;
  float& internalTempC;
  float& internalHumidity;
  float& externalTempC;
  float& externalHumidity;
  bool& haveSensorReading;
  uint32_t& lastInternalRetryMs;
  uint32_t& lastExternalRetryMs;
};

struct SensorPipelineConfig {
  uint32_t sensorRetryMs;
  float internalTempOffsetC;
  float externalTempOffsetC;
};

using SensorInitFn = bool (*)();
using SensorReadFn = bool (*)(Adafruit_AHTX0&, uint8_t, float&, float&);
using ApplyControlFn = void (*)(uint32_t nowMs);
using RecordHistoryFn = void (*)(uint32_t nowMs);

void readSensorsControlAndLog(uint32_t nowMs,
                              uint32_t& lastSensorMs,
                              uint32_t sensorPeriodMs,
                              SensorPipelineState state,
                              const SensorPipelineConfig& config,
                              Adafruit_AHTX0& ahtInternal,
                              Adafruit_AHTX0& ahtExternal,
                              uint8_t i2cBusInternal,
                              uint8_t i2cBusOled,
                              SensorInitFn initInternalAht,
                              SensorInitFn initExternalAht,
                              SensorReadFn readAht,
                              ApplyControlFn applyControl,
                              RecordHistoryFn recordHistorySample);
