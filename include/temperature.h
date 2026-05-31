#pragma once

#include <stdint.h>

namespace tempconv {

float displayTempC(float celsius, bool unitF);
float displayTempToC(float value, bool unitF);
float tempFromDisplay(float value, bool unitF);
float tempToDisplay(float value, bool unitF);
float deltaToDisplay(float valueC, bool unitF);
float deltaFromDisplay(float valueDisplay, bool unitF);
float clampFloatRange(float value, float minValue, float maxValue);
uint32_t clampUInt32Range(uint32_t value, uint32_t minValue, uint32_t maxValue);

}  // namespace tempconv
