#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

struct SettingsRequestDefaults {
  bool settingsAuthEnabled = false;
  bool buttonsEnabled = true;
  bool alarmEnabled = true;
  float alarmLowDisplay = 0.0f;
  float alarmHighDisplay = 0.0f;
  float relayOnDeltaDisplay = 0.0f;
  float relayOffDeltaDisplay = 0.0f;
  float internalTempOffsetF = 0.0f;
  float externalTempOffsetF = 0.0f;
  uint32_t minOffSeconds = 0;
  bool currentlyPasswordEnabled = false;
};

struct ParsedSettingsRequest {
  String unit;
  String mode;
  String oledLayoutValue;
  float setDisplay = 0.0f;

  bool nextSettingsAuthEnabled = false;
  String nextSettingsPassword;
  String nextSettingsPasswordConfirm;

  bool nextButtonsEnabled = true;
  bool nextAlarmEnabled = true;
  float alarmLowDisplay = 0.0f;
  float alarmHighDisplay = 0.0f;
  float onDeltaDisplay = 0.0f;
  float offDeltaDisplay = 0.0f;
  float internalTempOffsetF = 0.0f;
  float externalTempOffsetF = 0.0f;
  uint32_t minOffSeconds = 0;
};

bool parseSettingsPostRequest(AsyncWebServerRequest* request,
                              const SettingsRequestDefaults& defaults,
                              ParsedSettingsRequest& out,
                              String& errorCode);
