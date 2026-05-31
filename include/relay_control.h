#pragma once

#include <stdint.h>

namespace relaycontrol {

enum class RelayAction {
  None,
  TurnOnManual,
  TurnOffManual,
  TurnOffSensorInvalid,
  TurnOffLowerThreshold,
  TurnOnUpperThreshold,
};

struct RelayControlInputs {
  bool relayOn;
  bool internalValid;
  float internalTempC;
  float setTempC;
  float relayOnDeltaC;
  float relayOffDeltaC;
  uint32_t nowMs;
  uint32_t relayOffSinceMs;
  uint32_t relayLockoutMs;
  bool manualOn;
  bool manualOff;
};

RelayAction evaluateRelayAction(const RelayControlInputs& inputs);

}  // namespace relaycontrol
