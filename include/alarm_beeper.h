#pragma once

#include <stdint.h>

namespace alarmbeeper {

struct AlarmBeeperState {
  bool tempAlarmEnabled;
  bool internalValid;
  float internalTempC;
  float alarmLowTempC;
  float alarmHighTempC;
  bool tempAlarmBeepActive;
  uint8_t tempAlarmBeepStep;
  uint32_t tempAlarmNextStepMs;
  uint32_t tempAlarmTestUntilMs;
};

struct AlarmBeeperConfig {
  uint16_t toneHighHz;
  uint16_t toneLowHz;
  uint8_t ledcChannel;
  uint8_t ledcDuty;
  uint32_t stepMs;
  uint32_t pauseMs;
};

bool internalTempOutsideAlarmThresholds(const AlarmBeeperState& state);
void updateTemperatureAlarmBeeper(uint32_t nowMs,
                                  AlarmBeeperState& state,
                                  const AlarmBeeperConfig& config);

}  // namespace alarmbeeper
