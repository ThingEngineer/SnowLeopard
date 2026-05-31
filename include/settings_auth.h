#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

String generateSettingsAuthToken();

bool settingsAuthSatisfied(bool settingsPasswordEnabled,
                           const String& settingsAuthToken,
                           AsyncWebServerRequest* request,
                           const char* cookieName);

void addSettingsAuthCookie(bool settingsPasswordEnabled,
                           const String& settingsAuthToken,
                           AsyncWebServerResponse* response,
                           const char* cookieName,
                           uint32_t cookieTtlSec);

void refreshSettingsAuthToken(bool settingsPasswordEnabled, String& settingsAuthToken);

bool authenticateSettingsPassword(bool settingsPasswordEnabled,
                                  const String& settingsPassword,
                                  const String& providedPassword);
