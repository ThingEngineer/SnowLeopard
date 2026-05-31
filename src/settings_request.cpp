#include "settings_request.h"

namespace {

bool isFalseLike(const String& value) {
  return value == "0" || value == "false" || value == "False" || value == "off";
}

bool readBoolParam(AsyncWebServerRequest* request, const char* key, bool fallback) {
  if (!request->hasParam(key, true)) {
    return fallback;
  }
  return !isFalseLike(request->getParam(key, true)->value());
}

String readStringParam(AsyncWebServerRequest* request, const char* key, const String& fallback) {
  if (!request->hasParam(key, true)) {
    return fallback;
  }
  return request->getParam(key, true)->value();
}

float readFloatParam(AsyncWebServerRequest* request, const char* key, float fallback) {
  if (!request->hasParam(key, true)) {
    return fallback;
  }
  return request->getParam(key, true)->value().toFloat();
}

uint32_t readUIntParam(AsyncWebServerRequest* request, const char* key, uint32_t fallback) {
  if (!request->hasParam(key, true)) {
    return fallback;
  }
  return static_cast<uint32_t>(request->getParam(key, true)->value().toInt());
}

}  // namespace

bool parseSettingsPostRequest(AsyncWebServerRequest* request,
                              const SettingsRequestDefaults& defaults,
                              ParsedSettingsRequest& out,
                              String& errorCode) {
  const bool hasUnit = request->hasParam("temp_unit", true);
  const bool hasMode = request->hasParam("relay_mode", true);
  const bool hasSet = request->hasParam("set_temp", true);
  if (!hasUnit || !hasMode || !hasSet) {
    errorCode = "missing_settings";
    return false;
  }

  out.unit = request->getParam("temp_unit", true)->value();
  out.mode = request->getParam("relay_mode", true)->value();
  out.mode.trim();
  out.mode.toLowerCase();
  out.oledLayoutValue = readStringParam(request, "oled_layout", String("standard"));
  out.setDisplay = request->getParam("set_temp", true)->value().toFloat();

  out.nextSettingsAuthEnabled = readBoolParam(request, "settings_auth_enabled", defaults.settingsAuthEnabled);
  out.nextSettingsPassword = readStringParam(request, "settings_auth_password", String());
  out.nextSettingsPasswordConfirm = readStringParam(request, "settings_auth_password_confirm", String());

  out.nextAlarmEnabled = readBoolParam(request, "alarm_enabled", defaults.alarmEnabled);
  out.alarmLowDisplay = readFloatParam(request, "alarm_low", defaults.alarmLowDisplay);
  out.alarmHighDisplay = readFloatParam(request, "alarm_high", defaults.alarmHighDisplay);
  out.onDeltaDisplay = readFloatParam(request, "relay_on_delta", defaults.relayOnDeltaDisplay);
  out.offDeltaDisplay = readFloatParam(request, "relay_off_delta", defaults.relayOffDeltaDisplay);
  out.internalTempOffsetF = readFloatParam(request, "internal_temp_offset_f", defaults.internalTempOffsetF);
  out.externalTempOffsetF = readFloatParam(request, "external_temp_offset_f", defaults.externalTempOffsetF);
  out.minOffSeconds = readUIntParam(request, "min_off_seconds", defaults.minOffSeconds);

  if (!(out.unit == "C" || out.unit == "F")) {
    errorCode = "invalid_unit";
    return false;
  }

  const bool passwordFieldsProvided =
    out.nextSettingsPassword.length() > 0 || out.nextSettingsPasswordConfirm.length() > 0;
  if (out.nextSettingsAuthEnabled) {
    if (passwordFieldsProvided && out.nextSettingsPassword != out.nextSettingsPasswordConfirm) {
      errorCode = "password_mismatch";
      return false;
    }

    if (!defaults.currentlyPasswordEnabled && !passwordFieldsProvided) {
      errorCode = "password_required";
      return false;
    }
  }

  errorCode = String();
  return true;
}
