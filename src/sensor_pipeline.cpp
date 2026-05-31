#include "sensor_pipeline.h"

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
                              RecordHistoryFn recordHistorySample) {
  if ((nowMs - lastSensorMs) < sensorPeriodMs) {
    return;
  }
  lastSensorMs = nowMs;

  if (!state.internalValid && (nowMs - state.lastInternalRetryMs) >= config.sensorRetryMs) {
    state.lastInternalRetryMs = nowMs;
    initInternalAht();
  }

  if (!state.externalValid && (nowMs - state.lastExternalRetryMs) >= config.sensorRetryMs) {
    state.lastExternalRetryMs = nowMs;
    initExternalAht();
  }

  if (state.internalValid) {
    if (!readAht(ahtInternal, i2cBusInternal, state.internalTempC, state.internalHumidity)) {
      state.internalValid = false;
      state.internalTempC = NAN;
      state.internalHumidity = NAN;
      Serial.println("Internal AHT30 read failed; will retry init");
    } else {
      state.haveSensorReading = true;
    }
  }

  if (state.externalValid) {
    if (!readAht(ahtExternal, i2cBusOled, state.externalTempC, state.externalHumidity)) {
      state.externalValid = false;
      state.externalTempC = NAN;
      state.externalHumidity = NAN;
      Serial.println("External AHT30 read failed; will retry init");
    } else {
      state.haveSensorReading = true;
    }
  }

  applyControl(nowMs);
  recordHistorySample(nowMs);
}
