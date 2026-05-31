#pragma once

#include <Arduino.h>
#include <Preferences.h>

namespace historystore {

void begin(uint32_t bootSequence);
void primeTimers(uint32_t nowMs);
void loadSnapshot(Preferences& preferences);
void persistSnapshot(Preferences& preferences, bool force);

void recordSample(uint32_t nowMs,
                  float internalTempC,
                  float externalTempC,
                  bool internalValid,
                  bool externalValid,
                  bool relayOn);

String buildBlendedHistoryJson(uint32_t windowSeconds,
                               uint32_t requestedStepSeconds,
                               bool includeExternal,
                               const char* tempUnit,
                               float (*tempToDisplayFn)(float));

}  // namespace historystore
