#pragma once

#include <Arduino.h>
#include <DNSServer.h>

struct NetworkWorkflowConfig {
  const char* apSsid = "";
  byte dnsPort = 53;
  uint8_t apChannel = 1;
  bool apHidden = false;
  uint8_t apMaxClients = 4;
  uint32_t staConnectTimeoutMs = 20000;
  uint8_t staConnectAttempts = 2;
};

struct NetworkWorkflowState {
  bool& provisioningMode;
  String& staSsid;
  String& staPass;
  DNSServer& dnsServer;
};

bool connectStaOnce(NetworkWorkflowState state,
                    const NetworkWorkflowConfig& config,
                    const String& ssid,
                    const String& password);

bool connectStaWithRetry(NetworkWorkflowState state,
                         const NetworkWorkflowConfig& config,
                         const char* reason);

void startProvisioningAp(NetworkWorkflowState state,
                         const NetworkWorkflowConfig& config,
                         const char* reason);
