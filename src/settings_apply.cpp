#include "settings_apply.h"

#include "settings_auth.h"

void applySettingsUpdate(const SettingsApplyParams& params,
                         SettingsApplyState state,
                         float (*clampTempC)(float),
                         float (*clampRelayDeltaC)(float),
                         uint32_t (*clampRelayLockoutMs)(uint32_t),
                         void (*normalizeSetpointToWholeDisplayUnit)(),
                         void (*normalizeAlarmThresholdsToWholeDisplayUnit)(),
                         void (*clampSetpointToBounds)(),
                         void (*saveSettings)(),
                         void (*applyControl)(uint32_t)) {
  state.tempUnitF = params.nextUnitF;
  state.setTempC = clampTempC(params.nextSetC);
  normalizeSetpointToWholeDisplayUnit();
  state.alarmLowTempC = clampTempC(params.nextAlarmLowC);
  state.alarmHighTempC = clampTempC(params.nextAlarmHighC);
  normalizeAlarmThresholdsToWholeDisplayUnit();
  state.tempAlarmEnabled = params.nextAlarmEnabled;
  state.buttonsEnabled = params.nextButtonsEnabled;

  if (params.nextSettingsAuthEnabled) {
    state.settingsPasswordEnabled = true;
    if (params.passwordFieldsProvided) {
      state.settingsPassword = params.nextSettingsPassword;
    }
  } else {
    state.settingsPasswordEnabled = false;
    state.settingsPassword = String();
  }
  refreshSettingsAuthToken(state.settingsPasswordEnabled, state.settingsAuthToken);

  state.relayOnDeltaC = clampRelayDeltaC(params.nextOnDeltaC);
  state.relayOffDeltaC = clampRelayDeltaC(params.nextOffDeltaC);
  state.internalTempOffsetF = params.nextInternalTempOffsetF;
  state.externalTempOffsetF = params.nextExternalTempOffsetF;
  state.relayLockoutMs = clampRelayLockoutMs(params.minOffSeconds * 1000U);
  clampSetpointToBounds();
  saveSettings();
  applyControl(params.nowMs);
}
