#pragma once

#include <Arduino.h>

struct FirmwareReleaseManifest {
  String version;
  String summary;
  String notesUrl;
  String firmwareUrl;
  String publishedAt;
  String checksumSha256;
  uint32_t firmwareSize = 0;
};

struct FirmwareUpdateState {
  String currentVersion;
  String manifestUrl;
  FirmwareReleaseManifest manifest;
  String downloadSha256;
  String status = "idle";
  String message;
  bool manifestLoaded = false;
  bool updateAvailable = false;
  bool otaReady = false;
  bool updateQueued = false;
  bool updateInProgress = false;
  bool rebootPending = false;
  bool checksumVerified = false;
  uint32_t lastCheckedMs = 0;
  uint32_t rebootAtMs = 0;
  uint32_t bytesWritten = 0;
  uint32_t contentLength = 0;
  uint8_t progressPercent = 0;
};

void initFirmwareUpdateState(FirmwareUpdateState& state,
                             const char* currentVersion,
                             const char* manifestUrl);

bool refreshFirmwareManifest(FirmwareUpdateState& state,
                             uint32_t nowMs,
                             bool networkAvailable,
                             bool forceRefresh);

bool queueFirmwareUpdate(FirmwareUpdateState& state, String& errorCode);

void processFirmwareUpdate(FirmwareUpdateState& state, uint32_t nowMs, bool networkAvailable);

String buildFirmwareStatusJson(const FirmwareUpdateState& state);