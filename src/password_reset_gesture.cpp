#include "password_reset_gesture.h"

namespace {

bool consumeDebouncedPress(uint8_t pin,
                           bool& stable,
                           bool& lastRead,
                           uint32_t& debounceStart,
                           uint32_t nowMs,
                           uint32_t debounceMs) {
  const bool readNow = digitalRead(pin);

  if (readNow != lastRead) {
    lastRead = readNow;
    debounceStart = nowMs;
  }

  if ((nowMs - debounceStart) >= debounceMs && readNow != stable) {
    stable = readNow;
    return stable == LOW;
  }

  return false;
}

}  // namespace

void initPasswordResetGestureState(PasswordResetGestureState& state,
                                   bool initialUpLevel,
                                   bool initialDownLevel,
                                   uint32_t nowMs) {
  state.upStable = initialUpLevel;
  state.upLastRead = initialUpLevel;
  state.upDebounceStartMs = nowMs;
  state.downStable = initialDownLevel;
  state.downLastRead = initialDownLevel;
  state.downDebounceStartMs = nowMs;
  state.armed = false;
  state.deadlineMs = 0;
  state.downCount = 0;
}

bool handlePasswordResetGesture(PasswordResetGestureState& state,
                                uint32_t nowMs,
                                uint8_t upPin,
                                uint8_t downPin,
                                uint32_t debounceMs,
                                uint32_t windowMs,
                                bool& triggered) {
  triggered = false;
  consumeDebouncedPress(upPin, state.upStable, state.upLastRead, state.upDebounceStartMs, nowMs, debounceMs);
  const bool downPressed = consumeDebouncedPress(
    downPin, state.downStable, state.downLastRead, state.downDebounceStartMs, nowMs, debounceMs);

  if (state.upStable == LOW && state.downStable == LOW) {
    if (!state.armed) {
      state.armed = true;
      state.deadlineMs = nowMs + windowMs;
      state.downCount = 0;
    }
  } else if (state.armed) {
    state.armed = false;
    state.deadlineMs = 0;
    state.downCount = 0;
  }

  if (!state.armed) {
    return false;
  }

  if (nowMs > state.deadlineMs) {
    state.armed = false;
    state.deadlineMs = 0;
    state.downCount = 0;
    return false;
  }

  if (downPressed && state.upStable == LOW && state.downStable == LOW) {
    state.downCount++;
    if (state.downCount >= 2) {
      state.armed = false;
      state.deadlineMs = 0;
      state.downCount = 0;
      triggered = true;
      return true;
    }
  }

  return true;
}
