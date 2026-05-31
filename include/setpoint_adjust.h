#pragma once

#include <stdint.h>

namespace setpointadjust {

struct SetpointButtonState {
  bool& upStable;
  bool& upLastRead;
  uint32_t& upDebounceStartMs;
  uint32_t& upNextRepeatMs;
  bool& downStable;
  bool& downLastRead;
  uint32_t& downDebounceStartMs;
  uint32_t& downNextRepeatMs;
};

struct SetpointAdjustState {
  bool tempUnitF;
  float& setTempC;
  uint32_t& setpointFocusUntilMs;
};

struct SetpointAdjustConfig {
  uint8_t pinBtnUp;
  uint8_t pinBtnDown;
  float targetStepC;
  uint32_t buttonDebounceMs;
  uint32_t buttonHoldRepeatMs;
  uint32_t setpointFocusDisplayMs;
};

void handleSetpointButtons(uint32_t nowMs,
                           SetpointButtonState buttonState,
                           SetpointAdjustState adjustState,
                           const SetpointAdjustConfig& config,
                           void (*normalizeSetpointToWholeDisplayUnit)(),
                           void (*saveSettings)(),
                           void (*applyControl)(uint32_t));

}  // namespace setpointadjust
