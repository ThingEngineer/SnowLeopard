#include "web_routes.h"

namespace {
static constexpr bool ENABLE_HTTP_ROUTE_DEBUG_LOGS = false;
}

void registerCaptiveProbeRoutes(AsyncWebServer& webServer,
                                bool& provisioningMode,
                                RouteRequestHandler sendCaptiveProbePage,
                                RouteRequestHandler appleProbeToPortal,
                                RouteRequestHandler redirectToPortal) {
  // Common captive portal probes across major mobile/desktop OSes.
  webServer.on("/generate_204", HTTP_GET, [&provisioningMode, sendCaptiveProbePage](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->redirect("/");
      return;
    }
    sendCaptiveProbePage(request);
  });
  webServer.on("/gen_204", HTTP_GET, [&provisioningMode, sendCaptiveProbePage](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->redirect("/");
      return;
    }
    sendCaptiveProbePage(request);
  });
  webServer.on("/hotspot-detect.html", HTTP_GET, [&provisioningMode, appleProbeToPortal](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->redirect("/");
      return;
    }
    appleProbeToPortal(request);
  });
  webServer.on("/library/test/success.html", HTTP_GET, [&provisioningMode, appleProbeToPortal](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->redirect("/");
      return;
    }
    appleProbeToPortal(request);
  });
  webServer.on("/success.html", HTTP_GET, [&provisioningMode, appleProbeToPortal](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->redirect("/");
      return;
    }
    appleProbeToPortal(request);
  });
  webServer.on("/check_network_status.txt", HTTP_GET, [&provisioningMode, appleProbeToPortal](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->redirect("/");
      return;
    }
    appleProbeToPortal(request);
  });
  webServer.on("/mobile/status.php", HTTP_GET, [&provisioningMode, appleProbeToPortal](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->redirect("/");
      return;
    }
    appleProbeToPortal(request);
  });
  webServer.on("/connecttest.txt", HTTP_GET, [&provisioningMode, redirectToPortal](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->redirect("/");
      return;
    }
    redirectToPortal(request);
  });
  webServer.on("/redirect", HTTP_GET, [&provisioningMode, redirectToPortal](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->redirect("/");
      return;
    }
    redirectToPortal(request);
  });
  webServer.on("/ncsi.txt", HTTP_GET, [&provisioningMode, redirectToPortal](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->redirect("/");
      return;
    }
    redirectToPortal(request);
  });
  webServer.on("/fwlink", HTTP_GET, [&provisioningMode, redirectToPortal](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->redirect("/");
      return;
    }
    redirectToPortal(request);
  });
  webServer.on("/success.txt", HTTP_GET, [&provisioningMode, redirectToPortal](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->redirect("/");
      return;
    }
    redirectToPortal(request);
  });
  webServer.on("/canonical.html", HTTP_GET, [&provisioningMode, redirectToPortal](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->redirect("/");
      return;
    }
    redirectToPortal(request);
  });
}

void registerApiHistoryRoute(AsyncWebServer& webServer, HistoryJsonBuilder historyBuilder) {
  webServer.on("/api/history", HTTP_GET, [historyBuilder](AsyncWebServerRequest* request) {
    uint32_t windowSeconds = 3600U;
    if (request->hasParam("window_s")) {
      windowSeconds = static_cast<uint32_t>(request->getParam("window_s")->value().toInt());
    }
    if (windowSeconds < 60U) {
      windowSeconds = 60U;
    }
    if (windowSeconds > 259200U) {
      windowSeconds = 259200U;
    }

    uint32_t stepSeconds = 0U;
    if (request->hasParam("step_s")) {
      stepSeconds = static_cast<uint32_t>(request->getParam("step_s")->value().toInt());
    }

    bool includeExternal = true;
    if (request->hasParam("include_external")) {
      const String v = request->getParam("include_external")->value();
      includeExternal = !(v == "0" || v == "false" || v == "False");
    }

    const String response = historyBuilder(windowSeconds, stepSeconds, includeExternal);
    request->send(200, "application/json", response);
  });
}

void registerApiStatusRoute(AsyncWebServer& webServer, StatusJsonProvider statusJsonProvider) {
  webServer.on("/api/status", HTTP_GET, [statusJsonProvider](AsyncWebServerRequest* request) {
    request->send(200, "application/json", statusJsonProvider());
  });
}

void registerSettingsPageRoute(AsyncWebServer& webServer,
                               bool& provisioningMode,
                               SettingsPageAuthorizer authorizeSettingsPage,
                               const char* settingsAuthHtml,
                               const char* settingsHtml) {
  webServer.on("/settings", HTTP_GET, [&provisioningMode,
                                        authorizeSettingsPage,
                                        settingsAuthHtml,
                                        settingsHtml](AsyncWebServerRequest* request) {
    if (provisioningMode) {
      request->redirect("/");
      return;
    }

    if (!authorizeSettingsPage(request)) {
      request->send(200, "text/html", settingsAuthHtml);
      return;
    }

    request->send(200, "text/html", settingsHtml);
  });
}

void registerHistoryPageRoute(AsyncWebServer& webServer,
                              bool& provisioningMode,
                              const char* historyHtml) {
  webServer.on("/history", HTTP_GET, [&provisioningMode, historyHtml](AsyncWebServerRequest* request) {
    if (provisioningMode) {
      request->redirect("/");
      return;
    }

    request->send(200, "text/html", historyHtml);
  });
}

void registerApiSettingsGetRoute(AsyncWebServer& webServer,
                                 SettingsRequestAuthorizer authorizeSettingsRequest,
                                 SettingsJsonProvider settingsJsonProvider) {
  webServer.on("/api/settings", HTTP_GET, [authorizeSettingsRequest, settingsJsonProvider](AsyncWebServerRequest* request) {
    if (!authorizeSettingsRequest(request)) {
      request->send(401, "application/json", "{\"ok\":false,\"error\":\"auth_required\"}");
      return;
    }

    request->send(200, "application/json", settingsJsonProvider());
  });
}

void registerApiFirmwareGetRoute(AsyncWebServer& webServer,
                                 bool& provisioningMode,
                                 SettingsRequestAuthorizer authorizeSettingsRequest,
                                 FirmwareJsonProvider firmwareJsonProvider) {
  webServer.on("/api/firmware", HTTP_GET, [&provisioningMode,
                                             authorizeSettingsRequest,
                                             firmwareJsonProvider](AsyncWebServerRequest* request) {
    if (provisioningMode) {
      request->send(409, "application/json", "{\"ok\":false,\"error\":\"not_available_in_provisioning\"}");
      return;
    }

    if (!authorizeSettingsRequest(request)) {
      request->send(401, "application/json", "{\"ok\":false,\"error\":\"auth_required\"}");
      return;
    }

    bool forceRefresh = false;
    if (request->hasParam("refresh")) {
      const String value = request->getParam("refresh")->value();
      forceRefresh = !(value == "0" || value == "false" || value == "False");
    }

    request->send(200, "application/json", firmwareJsonProvider(forceRefresh));
  });
}

void registerApiFirmwareCheckRoute(AsyncWebServer& webServer,
                                   bool& provisioningMode,
                                   SettingsRequestAuthorizer authorizeSettingsRequest,
                                   FirmwareJsonProvider firmwareJsonProvider) {
  webServer.on("/api/firmware_check", HTTP_POST, [&provisioningMode,
                                                    authorizeSettingsRequest,
                                                    firmwareJsonProvider](AsyncWebServerRequest* request) {
    if (provisioningMode) {
      request->send(409, "application/json", "{\"ok\":false,\"error\":\"not_available_in_provisioning\"}");
      return;
    }

    if (!authorizeSettingsRequest(request)) {
      request->send(401, "application/json", "{\"ok\":false,\"error\":\"auth_required\"}");
      return;
    }

    request->send(200, "application/json", firmwareJsonProvider(true));
  });
}

void registerApiFirmwareUpdateRoute(AsyncWebServer& webServer,
                                    bool& provisioningMode,
                                    SettingsRequestAuthorizer authorizeSettingsRequest,
                                    FirmwareUpdateStarter startFirmwareUpdate) {
  webServer.on("/api/firmware_update", HTTP_POST, [&provisioningMode,
                                                     authorizeSettingsRequest,
                                                     startFirmwareUpdate](AsyncWebServerRequest* request) {
    if (provisioningMode) {
      request->send(409, "application/json", "{\"ok\":false,\"error\":\"not_available_in_provisioning\"}");
      return;
    }

    if (!authorizeSettingsRequest(request)) {
      request->send(401, "application/json", "{\"ok\":false,\"error\":\"auth_required\"}");
      return;
    }

    String errorCode;
    if (!startFirmwareUpdate(errorCode)) {
      request->send(409,
                    "application/json",
                    String("{\"ok\":false,\"error\":\"") + errorCode + "\"}");
      return;
    }

    request->send(200, "application/json", "{\"ok\":true,\"status\":\"queued\"}");
  });
}

void registerApiSettingsLoginRoute(AsyncWebServer& webServer,
                                   bool& provisioningMode,
                                   bool& settingsPasswordEnabled,
                                   String& settingsAuthToken,
                                   SettingsPasswordAuthenticator authenticateSettingsPassword,
                                   SettingsAuthCookieAdder addSettingsAuthCookie,
                                   SettingsAuthTokenRefresher refreshSettingsAuthToken) {
  webServer.on("/api/settings_login", HTTP_POST, [&provisioningMode,
                                                   &settingsPasswordEnabled,
                                                   &settingsAuthToken,
                                                   authenticateSettingsPassword,
                                                   addSettingsAuthCookie,
                                                   refreshSettingsAuthToken](AsyncWebServerRequest* request) {
    if (provisioningMode) {
      request->send(409, "application/json", "{\"ok\":false,\"error\":\"not_available_in_provisioning\"}");
      return;
    }

    const bool hasPassword = request->hasParam("password", true);
    const String password = hasPassword ? request->getParam("password", true)->value() : String();

    if (!settingsPasswordEnabled) {
      AsyncWebServerResponse* response = request->beginResponse(200, "application/json", "{\"ok\":true,\"auth_enabled\":false}");
      addSettingsAuthCookie(response);
      request->send(response);
      return;
    }

    if (!authenticateSettingsPassword(password)) {
      request->send(401, "application/json", "{\"ok\":false,\"error\":\"invalid_password\"}");
      return;
    }

    if (settingsAuthToken.length() == 0) {
      refreshSettingsAuthToken();
    }

    AsyncWebServerResponse* response = request->beginResponse(200, "application/json", "{\"ok\":true,\"auth_enabled\":true}");
    addSettingsAuthCookie(response);
    request->send(response);
  });
}

void registerApiReconfigureRoute(AsyncWebServer& webServer,
                                 bool& provisioningMode,
                                 SettingsRequestAuthorizer authorizeSettingsRequest,
                                 RouteAction startReconfigureAction) {
  webServer.on("/api/reconfigure", HTTP_POST, [&provisioningMode,
                                                authorizeSettingsRequest,
                                                startReconfigureAction](AsyncWebServerRequest* request) {
    if (provisioningMode) {
      request->send(200, "application/json", "{\"ok\":true,\"mode\":\"provisioning\"}");
      return;
    }

    if (!authorizeSettingsRequest(request)) {
      request->send(401, "application/json", "{\"ok\":false,\"error\":\"auth_required\"}");
      return;
    }

    startReconfigureAction();
    request->send(200, "application/json", "{\"ok\":true}");
  });
}

void registerApiAlarmTestRoute(AsyncWebServer& webServer,
                               bool& provisioningMode,
                               SettingsRequestAuthorizer authorizeSettingsRequest,
                               RouteAction startAlarmTestAction) {
  webServer.on("/api/alarm_test", HTTP_POST, [&provisioningMode,
                                               authorizeSettingsRequest,
                                               startAlarmTestAction](AsyncWebServerRequest* request) {
    if (provisioningMode) {
      request->send(409, "application/json", "{\"ok\":false,\"error\":\"not_available_in_provisioning\"}");
      return;
    }

    if (!authorizeSettingsRequest(request)) {
      request->send(401, "application/json", "{\"ok\":false,\"error\":\"auth_required\"}");
      return;
    }

    startAlarmTestAction();
    request->send(200, "application/json", "{\"ok\":true,\"duration_ms\":3000}");
  });
}

void registerApiProvisionRoute(AsyncWebServer& webServer,
                               bool& provisioningMode,
                               StaCredentialsSaver saveStaCredentials,
                               RouteAction markPendingStaReboot) {
  webServer.on("/api/provision", HTTP_POST, [&provisioningMode,
                                              saveStaCredentials,
                                              markPendingStaReboot](AsyncWebServerRequest* request) {
    if (!provisioningMode) {
      request->send(409, "application/json", "{\"ok\":false,\"error\":\"not_in_provisioning_mode\"}");
      return;
    }

    const bool hasSsid = request->hasParam("ssid", true);
    const bool hasPassword = request->hasParam("password", true);
    String ssid = hasSsid ? request->getParam("ssid", true)->value() : String();
    String password = hasPassword ? request->getParam("password", true)->value() : String();
    ssid.trim();

    if (ssid.length() < 1 || ssid.length() > 32 || password.length() > 63) {
      request->send(400, "application/json", "{\"ok\":false,\"error\":\"invalid_wifi_credentials\"}");
      return;
    }

    saveStaCredentials(ssid, password);
    markPendingStaReboot();
    request->send(200, "application/json", "{\"ok\":true}");
  });
}

void registerApiSettingsPostRoute(AsyncWebServer& webServer, RouteRequestHandler settingsPostHandler) {
  webServer.on("/api/settings", HTTP_POST, settingsPostHandler);
}

void registerRootRoute(AsyncWebServer& webServer,
                       bool& provisioningMode,
                       RequestPredicate shouldCaptiveRedirect,
                       const char* provisionHtml,
                       const char* indexHtml) {
  webServer.on("/", HTTP_GET, [&provisioningMode,
                                shouldCaptiveRedirect,
                                provisionHtml,
                                indexHtml](AsyncWebServerRequest* request) {
    if (ENABLE_HTTP_ROUTE_DEBUG_LOGS) {
      const String host = request->hasHeader("Host") ? request->getHeader("Host")->value() : String("(none)");
      Serial.printf("[HTTP] root uri=%s host=%s\n", request->url().c_str(), host.c_str());
    }

    if (provisioningMode && shouldCaptiveRedirect(request)) {
      request->redirect("http://192.168.4.1/");
      return;
    }

    if (provisioningMode) {
      request->send(200, "text/html", provisionHtml);
      return;
    }

    request->send(200, "text/html", indexHtml);
  });
}

void registerNotFoundRoute(AsyncWebServer& webServer, bool& provisioningMode) {
  webServer.onNotFound([&provisioningMode](AsyncWebServerRequest* request) {
    if (ENABLE_HTTP_ROUTE_DEBUG_LOGS) {
      const String host = request->hasHeader("Host") ? request->getHeader("Host")->value() : String("(none)");
      Serial.printf("[HTTP] notfound uri=%s host=%s\n", request->url().c_str(), host.c_str());
    }
    if (provisioningMode) {
      request->redirect("http://192.168.4.1/");
    } else {
      request->redirect("/");
    }
  });
}
