#include "network_helpers.h"

#include <WiFi.h>

namespace {

bool isNumericIpHost(const String& host) {
  for (size_t i = 0; i < host.length(); i++) {
    const int c = host.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return host.length() > 0;
}

String normalizedHostNoPort(const String& rawHost) {
  String host = rawHost;
  host.trim();
  host.toLowerCase();

  const int colonIndex = host.indexOf(':');
  if (colonIndex > 0) {
    host = host.substring(0, colonIndex);
  }
  return host;
}

}  // namespace

String currentDeviceIpText() {
  IPAddress ip = WiFi.localIP();
  if (WiFi.getMode() == WIFI_MODE_AP || ip == IPAddress(0, 0, 0, 0)) {
    ip = WiFi.softAPIP();
  }
  return ip.toString();
}

bool shouldCaptiveRedirect(AsyncWebServerRequest* request) {
  if (!request->hasHeader("Host")) {
    return false;
  }

  const String host = normalizedHostNoPort(request->getHeader("Host")->value());
  if (host.length() == 0) {
    return false;
  }

  if (!isNumericIpHost(host) && host.indexOf("192.168.4.1") < 0 && host.indexOf("snowleopard_config") < 0 &&
      host.indexOf("snowleopard") < 0) {
    return true;
  }

  return false;
}

void loadStaCredentials(Preferences& preferences, String& staSsid, String& staPass) {
  staSsid = preferences.isKey("sta_ssid") ? preferences.getString("sta_ssid", "") : String();
  staPass = preferences.isKey("sta_pass") ? preferences.getString("sta_pass", "") : String();
}

void saveStaCredentials(Preferences& preferences,
                        String& staSsid,
                        String& staPass,
                        const String& ssid,
                        const String& password) {
  preferences.putString("sta_ssid", ssid);
  preferences.putString("sta_pass", password);
  staSsid = ssid;
  staPass = password;
}

bool hasStoredStaCredentials(const String& staSsid) {
  return staSsid.length() > 0;
}
