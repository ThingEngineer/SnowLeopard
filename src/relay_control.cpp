#include "relay_control.h"

#include <math.h>

namespace relaycontrol {

RelayAction evaluateRelayAction(const RelayControlInputs& inputs) {
  if (inputs.manualOn) {
    return inputs.relayOn ? RelayAction::None : RelayAction::TurnOnManual;
  }

  if (inputs.manualOff) {
    return inputs.relayOn ? RelayAction::TurnOffManual : RelayAction::None;
  }

  if (!inputs.internalValid || isnan(inputs.internalTempC)) {
    return inputs.relayOn ? RelayAction::TurnOffSensorInvalid : RelayAction::None;
  }

  if (inputs.relayOn) {
    if (inputs.internalTempC <= (inputs.setTempC - inputs.relayOffDeltaC)) {
      return RelayAction::TurnOffLowerThreshold;
    }
    return RelayAction::None;
  }

  const bool lockout = (inputs.nowMs - inputs.relayOffSinceMs) < inputs.relayLockoutMs;
  if (inputs.internalTempC >= (inputs.setTempC + inputs.relayOnDeltaC) && !lockout) {
    return RelayAction::TurnOnUpperThreshold;
  }

  return RelayAction::None;
}

}  // namespace relaycontrol
