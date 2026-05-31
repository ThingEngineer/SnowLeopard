#pragma once

#include <Arduino.h>

struct PasswordResetGestureState {
  bool upStable = HIGH;
  bool upLastRead = HIGH;
  uint32_t upDebounceStartMs = 0;
  bool downStable = HIGH;
  bool downLastRead = HIGH;
  uint32_t downDebounceStartMs = 0;
  bool armed = false;
  uint32_t deadlineMs = 0;
  uint8_t downCount = 0;
};

void initPasswordResetGestureState(PasswordResetGestureState& state,
                                   bool initialUpLevel,
                                   bool initialDownLevel,
                                   uint32_t nowMs);

bool handlePasswordResetGesture(PasswordResetGestureState& state,
                                uint32_t nowMs,
                                uint8_t upPin,
                                uint8_t downPin,
                                uint32_t debounceMs,
                                uint32_t windowMs,
                                bool& triggered);
