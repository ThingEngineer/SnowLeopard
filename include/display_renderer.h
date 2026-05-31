#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

struct DisplayRenderInput {
  explicit DisplayRenderInput(U8G2& display) : u8g2(display) {}

  U8G2& u8g2;
  uint32_t nowMs = 0;

  bool provisioningMode = false;
  uint32_t staInfoDisplayUntilMs = 0;
  uint32_t oledNoticeUntilMs = 0;
  const String* oledNoticeText = nullptr;
  bool haveSensorReading = false;
  uint32_t setpointFocusUntilMs = 0;
  bool tempAlarmBeepActive = false;
  uint32_t tempAlarmTestUntilMs = 0;

  float setTempC = NAN;
  float internalTempC = NAN;
  float externalTempC = NAN;
  bool relayOn = false;

  int oledLayoutMode = 0;
  int oledXOffset = 0;
  int oledYOffset = 0;

  const char* apSsid = "";
  const char* relayStateText = "";
  const char* (*tempUnitTextFn)() = nullptr;
  float (*tempToDisplayFn)(float) = nullptr;
};

void renderDisplay(DisplayRenderInput input);
