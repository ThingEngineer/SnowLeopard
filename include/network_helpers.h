#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

String currentDeviceIpText();

bool shouldCaptiveRedirect(AsyncWebServerRequest* request);

void loadStaCredentials(Preferences& preferences, String& staSsid, String& staPass);
void saveStaCredentials(Preferences& preferences,
                        String& staSsid,
                        String& staPass,
                        const String& ssid,
                        const String& password);
bool hasStoredStaCredentials(const String& staSsid);
