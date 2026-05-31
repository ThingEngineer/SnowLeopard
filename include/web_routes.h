#pragma once

#include <ESPAsyncWebServer.h>

typedef void (*RouteRequestHandler)(AsyncWebServerRequest* request);
typedef String (*HistoryJsonBuilder)(uint32_t windowSeconds, uint32_t stepSeconds, bool includeExternal);
typedef String (*StatusJsonProvider)();
typedef bool (*SettingsRequestAuthorizer)(AsyncWebServerRequest* request);
typedef bool (*SettingsPageAuthorizer)(AsyncWebServerRequest* request);
typedef bool (*RequestPredicate)(AsyncWebServerRequest* request);
typedef String (*SettingsJsonProvider)();
typedef bool (*SettingsPasswordAuthenticator)(const String& password);
typedef void (*SettingsAuthCookieAdder)(AsyncWebServerResponse* response);
typedef void (*SettingsAuthTokenRefresher)();
typedef void (*RouteAction)();
typedef void (*StaCredentialsSaver)(const String& ssid, const String& password);

void registerCaptiveProbeRoutes(AsyncWebServer& webServer,
                                bool& provisioningMode,
                                RouteRequestHandler sendCaptiveProbePage,
                                RouteRequestHandler appleProbeToPortal,
                                RouteRequestHandler redirectToPortal);

void registerApiHistoryRoute(AsyncWebServer& webServer, HistoryJsonBuilder historyBuilder);

void registerApiStatusRoute(AsyncWebServer& webServer, StatusJsonProvider statusJsonProvider);

void registerSettingsPageRoute(AsyncWebServer& webServer,
                               bool& provisioningMode,
                               SettingsPageAuthorizer authorizeSettingsPage,
                               const char* settingsAuthHtml,
                               const char* settingsHtml);

void registerHistoryPageRoute(AsyncWebServer& webServer,
                              bool& provisioningMode,
                              const char* historyHtml);

void registerApiSettingsGetRoute(AsyncWebServer& webServer,
                                 SettingsRequestAuthorizer authorizeSettingsRequest,
                                 SettingsJsonProvider settingsJsonProvider);

void registerApiSettingsLoginRoute(AsyncWebServer& webServer,
                                   bool& provisioningMode,
                                   bool& settingsPasswordEnabled,
                                   String& settingsAuthToken,
                                   SettingsPasswordAuthenticator authenticateSettingsPassword,
                                   SettingsAuthCookieAdder addSettingsAuthCookie,
                                   SettingsAuthTokenRefresher refreshSettingsAuthToken);

void registerApiReconfigureRoute(AsyncWebServer& webServer,
                                 bool& provisioningMode,
                                 SettingsRequestAuthorizer authorizeSettingsRequest,
                                 RouteAction startReconfigureAction);

void registerApiAlarmTestRoute(AsyncWebServer& webServer,
                               bool& provisioningMode,
                               SettingsRequestAuthorizer authorizeSettingsRequest,
                               RouteAction startAlarmTestAction);

void registerApiProvisionRoute(AsyncWebServer& webServer,
                               bool& provisioningMode,
                               StaCredentialsSaver saveStaCredentials,
                               RouteAction markPendingStaReboot);

void registerApiSettingsPostRoute(AsyncWebServer& webServer, RouteRequestHandler settingsPostHandler);

void registerRootRoute(AsyncWebServer& webServer,
                       bool& provisioningMode,
                       RequestPredicate shouldCaptiveRedirect,
                       const char* provisionHtml,
                       const char* indexHtml);

void registerNotFoundRoute(AsyncWebServer& webServer, bool& provisioningMode);
