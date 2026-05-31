#include "network_workflow.h"

#include <WiFi.h>

namespace {

bool hasStoredStaCredentials(const String& staSsid) {
  return staSsid.length() > 0;
}

}  // namespace

bool connectStaOnce(NetworkWorkflowState state,
                    const NetworkWorkflowConfig& config,
                    const String& ssid,
                    const String& password) {
  WiFi.persistent(false);
  WiFi.mode(WIFI_MODE_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid.c_str(), password.c_str());

  const uint32_t startMs = millis();
  while ((millis() - startMs) < config.staConnectTimeoutMs) {
    if (WiFi.status() == WL_CONNECTED) {
      state.provisioningMode = false;
      Serial.print("STA connected, IP: ");
      Serial.println(WiFi.localIP());
      return true;
    }
    delay(250);
  }

  WiFi.disconnect(true);
  return false;
}

bool connectStaWithRetry(NetworkWorkflowState state,
                         const NetworkWorkflowConfig& config,
                         const char* reason) {
  if (!hasStoredStaCredentials(state.staSsid)) {
    return false;
  }

  state.dnsServer.stop();
  for (uint8_t attempt = 1; attempt <= config.staConnectAttempts; attempt++) {
    Serial.printf("STA connect attempt %u/%u (%s)\n", attempt, config.staConnectAttempts, reason);
    if (connectStaOnce(state, config, state.staSsid, state.staPass)) {
      return true;
    }
  }

  Serial.println("STA connection failed after retries");
  return false;
}

void startProvisioningAp(NetworkWorkflowState state,
                         const NetworkWorkflowConfig& config,
                         const char* reason) {
  Serial.printf("Entering provisioning AP mode (%s)\n", reason);
  WiFi.disconnect(true);
  WiFi.persistent(false);
  WiFi.mode(WIFI_MODE_AP);
  WiFi.setSleep(false);
  delay(100);

  bool apOk = WiFi.softAP(config.apSsid, nullptr, config.apChannel, config.apHidden, config.apMaxClients);

  if (!apOk) {
    Serial.println("WARN: SoftAP start failed; retrying with minimal defaults");
    WiFi.softAPdisconnect(true);
    delay(150);
    apOk = WiFi.softAP(config.apSsid);
  }
  if (!apOk) {
    Serial.println("ERROR: Failed to start SoftAP after retry");
  }

  WiFi.softAPsetHostname("SnowLeopard");
  const IPAddress apIp = WiFi.softAPIP();
  state.dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  state.dnsServer.start(config.dnsPort, "*", apIp);

  state.provisioningMode = true;
  Serial.print("AP SSID: ");
  Serial.println(config.apSsid);
  Serial.print("AP Security: ");
  Serial.println("OPEN");
  Serial.print("AP IP: ");
  Serial.println(apIp);
}
