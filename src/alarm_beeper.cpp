#include "alarm_beeper.h"

#include <Arduino.h>

namespace alarmbeeper {

namespace {

void setPiezoTone(const AlarmBeeperConfig& config, uint16_t frequencyHz) {
  if (frequencyHz == 0) {
    ledcWriteTone(config.ledcChannel, 0);
    ledcWrite(config.ledcChannel, 0);
    return;
  }

  ledcWriteTone(config.ledcChannel, frequencyHz);
  ledcWrite(config.ledcChannel, config.ledcDuty);
}

}  // namespace

bool internalTempOutsideAlarmThresholds(const AlarmBeeperState& state) {
  if (!state.tempAlarmEnabled) {
    return false;
  }

  if (!state.internalValid || !isfinite(state.internalTempC)) {
    return false;
  }

  return state.internalTempC < state.alarmLowTempC || state.internalTempC > state.alarmHighTempC;
}

void updateTemperatureAlarmBeeper(uint32_t nowMs,
                                  AlarmBeeperState& state,
                                  const AlarmBeeperConfig& config) {
  const bool alarmNowActive = internalTempOutsideAlarmThresholds(state) ||
                              (state.tempAlarmTestUntilMs != 0 && nowMs <= state.tempAlarmTestUntilMs);
  if (!alarmNowActive) {
    if (state.tempAlarmBeepActive) {
      state.tempAlarmBeepActive = false;
      state.tempAlarmBeepStep = 0;
      state.tempAlarmNextStepMs = 0;
      setPiezoTone(config, 0);
    }
    return;
  }

  if (!state.tempAlarmBeepActive) {
    state.tempAlarmBeepActive = true;
    state.tempAlarmBeepStep = 0;
    state.tempAlarmNextStepMs = 0;
  }

  if (state.tempAlarmNextStepMs != 0 && static_cast<int32_t>(nowMs - state.tempAlarmNextStepMs) < 0) {
    return;
  }

  switch (state.tempAlarmBeepStep) {
    case 0:
      setPiezoTone(config, config.toneHighHz);
      state.tempAlarmNextStepMs = nowMs + config.stepMs;
      break;
    case 1:
      setPiezoTone(config, config.toneLowHz);
      state.tempAlarmNextStepMs = nowMs + config.stepMs;
      break;
    case 2:
      setPiezoTone(config, config.toneHighHz);
      state.tempAlarmNextStepMs = nowMs + config.stepMs;
      break;
    case 3:
      setPiezoTone(config, config.toneLowHz);
      state.tempAlarmNextStepMs = nowMs + config.stepMs;
      break;
    default:
      setPiezoTone(config, 0);
      state.tempAlarmNextStepMs = nowMs + config.pauseMs;
      break;
  }

  state.tempAlarmBeepStep = (state.tempAlarmBeepStep + 1U) % 5U;
}

}  // namespace alarmbeeper
