#include "button_input.h"

namespace buttoninput {

bool consumePressOrRepeat(uint8_t pin,
                          bool& stable,
                          bool& lastRead,
                          uint32_t& debounceStart,
                          uint32_t& nextRepeatMs,
                          uint32_t nowMs,
                          uint32_t debounceMs,
                          uint32_t holdRepeatMs) {
  const bool readNow = digitalRead(pin);

  if (readNow != lastRead) {
    lastRead = readNow;
    debounceStart = nowMs;
  }

  if ((nowMs - debounceStart) >= debounceMs && readNow != stable) {
    stable = readNow;
    if (stable == LOW) {
      nextRepeatMs = nowMs + holdRepeatMs;
      return true;
    }
    nextRepeatMs = 0;
  }

  if (stable == LOW && nextRepeatMs != 0 && static_cast<int32_t>(nowMs - nextRepeatMs) >= 0) {
    nextRepeatMs = nowMs + holdRepeatMs;
    return true;
  }

  return false;
}

}  // namespace buttoninput
