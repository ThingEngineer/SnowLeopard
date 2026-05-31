#pragma once

#include <Arduino.h>

namespace buttoninput {

bool consumePressOrRepeat(uint8_t pin,
                          bool& stable,
                          bool& lastRead,
                          uint32_t& debounceStart,
                          uint32_t& nextRepeatMs,
                          uint32_t nowMs,
                          uint32_t debounceMs,
                          uint32_t holdRepeatMs);

}  // namespace buttoninput
