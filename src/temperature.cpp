#include "temperature.h"

#include <math.h>

namespace tempconv {

float displayTempC(float celsius, bool unitF) {
  if (isnan(celsius)) {
    return NAN;
  }

  if (!unitF) {
    return celsius;
  }

  return (celsius * 9.0f / 5.0f) + 32.0f;
}

float displayTempToC(float value, bool unitF) {
  if (unitF) {
    return (value - 32.0f) * 5.0f / 9.0f;
  }

  return value;
}

float tempFromDisplay(float value, bool unitF) {
  return unitF ? ((value - 32.0f) * 5.0f / 9.0f) : value;
}

float tempToDisplay(float value, bool unitF) {
  return unitF ? ((value * 9.0f / 5.0f) + 32.0f) : value;
}

float deltaToDisplay(float valueC, bool unitF) {
  return unitF ? (valueC * 9.0f / 5.0f) : valueC;
}

float deltaFromDisplay(float valueDisplay, bool unitF) {
  return unitF ? (valueDisplay * 5.0f / 9.0f) : valueDisplay;
}

float clampFloatRange(float value, float minValue, float maxValue) {
  if (value < minValue) {
    return minValue;
  }

  if (value > maxValue) {
    return maxValue;
  }

  return value;
}

uint32_t clampUInt32Range(uint32_t value, uint32_t minValue, uint32_t maxValue) {
  if (value < minValue) {
    return minValue;
  }

  if (value > maxValue) {
    return maxValue;
  }

  return value;
}

}  // namespace tempconv
