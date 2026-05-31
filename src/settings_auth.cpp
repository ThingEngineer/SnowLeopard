#include "settings_auth.h"

#include <esp_system.h>

namespace {

bool cookieContainsValue(const String& cookieHeader, const char* key, const String& expectedValue) {
  const String needle = String(key) + "=";
  int start = cookieHeader.indexOf(needle);
  while (start >= 0) {
    start += needle.length();
    int end = cookieHeader.indexOf(';', start);
    String value = (end >= 0) ? cookieHeader.substring(start, end) : cookieHeader.substring(start);
    value.trim();
    if (value == expectedValue) {
      return true;
    }
    start = cookieHeader.indexOf(needle, end >= 0 ? end : start);
  }
  return false;
}

}  // namespace

String generateSettingsAuthToken() {
  char token[17];
  snprintf(token,
           sizeof(token),
           "%08lx%08lx",
           static_cast<unsigned long>(esp_random()),
           static_cast<unsigned long>(esp_random()));
  return String(token);
}

bool settingsAuthSatisfied(bool settingsPasswordEnabled,
                           const String& settingsAuthToken,
                           AsyncWebServerRequest* request,
                           const char* cookieName) {
  if (!settingsPasswordEnabled) {
    return true;
  }

  if (!request->hasHeader("Cookie") || settingsAuthToken.length() == 0) {
    return false;
  }

  return cookieContainsValue(request->getHeader("Cookie")->value(), cookieName, settingsAuthToken);
}

void addSettingsAuthCookie(bool settingsPasswordEnabled,
                           const String& settingsAuthToken,
                           AsyncWebServerResponse* response,
                           const char* cookieName,
                           uint32_t cookieTtlSec) {
  if (!settingsPasswordEnabled || settingsAuthToken.length() == 0) {
    response->addHeader("Set-Cookie", String(cookieName) + "=; Path=/; Max-Age=0; SameSite=Lax");
    return;
  }

  response->addHeader("Set-Cookie",
                      String(cookieName) + "=" + settingsAuthToken +
                        "; Path=/; Max-Age=" + String(cookieTtlSec) + "; SameSite=Lax");
}

void refreshSettingsAuthToken(bool settingsPasswordEnabled, String& settingsAuthToken) {
  settingsAuthToken = settingsPasswordEnabled ? generateSettingsAuthToken() : String();
}

bool authenticateSettingsPassword(bool settingsPasswordEnabled,
                                  const String& settingsPassword,
                                  const String& providedPassword) {
  return settingsPasswordEnabled && settingsPassword.length() > 0 && providedPassword == settingsPassword;
}
