#include "sensor_io.h"

#include <math.h>

namespace sensorio {

bool initAht(Adafruit_AHTX0& sensor,
             SensorEndpointState state,
             const SensorInitConfig& config,
             SelectI2cBusFn selectI2cBus,
             ScanI2cBusFn scanI2cBus) {
  if (!selectI2cBus(config.bus)) {
    state.valid = false;
    state.tempC = NAN;
    state.humidity = NAN;
    Serial.println(config.busInitFailedMsg);
    return false;
  }

  scanI2cBus(config.scanLabel);

  if (sensor.begin(&Wire, 0x38)) {
    state.valid = true;
    Serial.println(config.okMsg);
    return true;
  }

  state.valid = false;
  state.tempC = NAN;
  state.humidity = NAN;
  Serial.println(config.notFoundMsg);
  return false;
}

bool readAht(Adafruit_AHTX0& sensor,
             uint8_t bus,
             float& tempC,
             float& humidity,
             SelectI2cBusFn selectI2cBus) {
  if (!selectI2cBus(bus)) {
    return false;
  }

  sensors_event_t humidityEvent;
  sensors_event_t tempEvent;
  sensor.getEvent(&humidityEvent, &tempEvent);

  if (isnan(tempEvent.temperature) || isnan(humidityEvent.relative_humidity)) {
    return false;
  }

  tempC = tempEvent.temperature;
  humidity = humidityEvent.relative_humidity;
  return true;
}

}  // namespace sensorio
