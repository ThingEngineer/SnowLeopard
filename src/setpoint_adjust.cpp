#include "setpoint_adjust.h"

#include "button_input.h"

namespace setpointadjust {

void handleSetpointButtons(uint32_t nowMs,
                           SetpointButtonState buttonState,
                           SetpointAdjustState adjustState,
                           const SetpointAdjustConfig& config,
                           void (*normalizeSetpointToWholeDisplayUnit)(),
                           void (*saveSettings)(),
                           void (*applyControl)(uint32_t)) {
  bool changed = false;
  if (buttoninput::consumePressOrRepeat(config.pinBtnUp,
                                        buttonState.upStable,
                                        buttonState.upLastRead,
                                        buttonState.upDebounceStartMs,
                                        buttonState.upNextRepeatMs,
                                        nowMs,
                                        config.buttonDebounceMs,
                                        config.buttonHoldRepeatMs)) {
    adjustState.setTempC += adjustState.tempUnitF ? (5.0f / 9.0f) : config.targetStepC;
    changed = true;
  }
  if (buttoninput::consumePressOrRepeat(config.pinBtnDown,
                                        buttonState.downStable,
                                        buttonState.downLastRead,
                                        buttonState.downDebounceStartMs,
                                        buttonState.downNextRepeatMs,
                                        nowMs,
                                        config.buttonDebounceMs,
                                        config.buttonHoldRepeatMs)) {
    adjustState.setTempC -= adjustState.tempUnitF ? (5.0f / 9.0f) : config.targetStepC;
    changed = true;
  }

  if (!changed) {
    return;
  }

  normalizeSetpointToWholeDisplayUnit();
  saveSettings();
  applyControl(nowMs);
  adjustState.setpointFocusUntilMs = nowMs + config.setpointFocusDisplayMs;
}

}  // namespace setpointadjust
