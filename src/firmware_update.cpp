#include "firmware_update.h"

#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClientSecure.h>

namespace {

static constexpr uint32_t FIRMWARE_REFRESH_CACHE_MS = 15UL * 60UL * 1000UL;
static constexpr uint32_t FIRMWARE_REBOOT_DELAY_MS = 1500UL;

String jsonEscape(const String& value) {
  String escaped;
  escaped.reserve(value.length() + 8);
  for (size_t i = 0; i < value.length(); ++i) {
    const char ch = value.charAt(i);
    switch (ch) {
      case '\\':
        escaped += "\\\\";
        break;
      case '"':
        escaped += "\\\"";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      default:
        escaped += ch;
        break;
    }
  }
  return escaped;
}

int findJsonValueStart(const String& json, const char* key) {
  const String needle = String("\"") + key + "\"";
  const int keyPos = json.indexOf(needle);
  if (keyPos < 0) {
    return -1;
  }

  int colonPos = json.indexOf(':', keyPos + needle.length());
  if (colonPos < 0) {
    return -1;
  }

  ++colonPos;
  while (colonPos < static_cast<int>(json.length()) && isspace(static_cast<unsigned char>(json.charAt(colonPos)))) {
    ++colonPos;
  }
  return colonPos;
}

bool extractJsonString(const String& json, const char* key, String& out) {
  const int valuePos = findJsonValueStart(json, key);
  if (valuePos < 0 || valuePos >= static_cast<int>(json.length()) || json.charAt(valuePos) != '"') {
    return false;
  }

  String value;
  bool escapeNext = false;
  for (int i = valuePos + 1; i < static_cast<int>(json.length()); ++i) {
    const char ch = json.charAt(i);
    if (escapeNext) {
      switch (ch) {
        case 'n':
          value += '\n';
          break;
        case 'r':
          value += '\r';
          break;
        case 't':
          value += '\t';
          break;
        default:
          value += ch;
          break;
      }
      escapeNext = false;
      continue;
    }

    if (ch == '\\') {
      escapeNext = true;
      continue;
    }
    if (ch == '"') {
      out = value;
      return true;
    }
    value += ch;
  }

  return false;
}

bool extractJsonUint32(const String& json, const char* key, uint32_t& out) {
  const int valuePos = findJsonValueStart(json, key);
  if (valuePos < 0) {
    return false;
  }

  String digits;
  for (int i = valuePos; i < static_cast<int>(json.length()); ++i) {
    const char ch = json.charAt(i);
    if (ch >= '0' && ch <= '9') {
      digits += ch;
      continue;
    }
    if (digits.length() > 0) {
      break;
    }
    if (!isspace(static_cast<unsigned char>(ch))) {
      return false;
    }
  }

  if (digits.length() == 0) {
    return false;
  }

  out = static_cast<uint32_t>(digits.toInt());
  return true;
}

void clearManifest(FirmwareUpdateState& state) {
  state.manifest = FirmwareReleaseManifest{};
  state.manifestLoaded = false;
  state.updateAvailable = false;
}

int nextVersionToken(const String& value, int& index) {
  while (index < static_cast<int>(value.length())) {
    const char ch = value.charAt(index);
    if ((ch >= '0' && ch <= '9') || ch == '.') {
      break;
    }
    ++index;
  }

  if (index >= static_cast<int>(value.length())) {
    return -1;
  }

  int result = 0;
  bool foundDigit = false;
  while (index < static_cast<int>(value.length())) {
    const char ch = value.charAt(index);
    if (ch >= '0' && ch <= '9') {
      result = (result * 10) + (ch - '0');
      foundDigit = true;
      ++index;
      continue;
    }
    if (ch == '.') {
      ++index;
      break;
    }
    break;
  }

  return foundDigit ? result : -1;
}

int compareVersions(const String& left, const String& right) {
  int leftIndex = 0;
  int rightIndex = 0;

  while (true) {
    const int leftToken = nextVersionToken(left, leftIndex);
    const int rightToken = nextVersionToken(right, rightIndex);

    if (leftToken < 0 && rightToken < 0) {
      return 0;
    }

    const int normalizedLeft = leftToken < 0 ? 0 : leftToken;
    const int normalizedRight = rightToken < 0 ? 0 : rightToken;
    if (normalizedLeft < normalizedRight) {
      return -1;
    }
    if (normalizedLeft > normalizedRight) {
      return 1;
    }
  }
}

bool fetchManifestPayload(const String& manifestUrl, String& payload, String& errorMessage) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  if (!http.begin(client, manifestUrl)) {
    errorMessage = "manifest_begin_failed";
    return false;
  }

  http.addHeader("Accept", "application/json");
  const int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    errorMessage = String("manifest_http_") + httpCode;
    http.end();
    return false;
  }

  payload = http.getString();
  http.end();

  if (payload.length() == 0) {
    errorMessage = "manifest_empty";
    return false;
  }

  return true;
}

bool parseManifestPayload(const String& payload, FirmwareReleaseManifest& manifest) {
  FirmwareReleaseManifest parsed;
  if (!extractJsonString(payload, "version", parsed.version)) {
    return false;
  }

  extractJsonString(payload, "summary", parsed.summary);
  extractJsonString(payload, "notes_url", parsed.notesUrl);
  extractJsonString(payload, "firmware_url", parsed.firmwareUrl);
  extractJsonString(payload, "published_at", parsed.publishedAt);
  extractJsonString(payload, "sha256", parsed.checksumSha256);
  extractJsonUint32(payload, "size", parsed.firmwareSize);

  manifest = parsed;
  return true;
}

void setStatus(FirmwareUpdateState& state, const String& status, const String& message) {
  state.status = status;
  state.message = message;
}

bool performFirmwareUpdate(FirmwareUpdateState& state) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  if (!http.begin(client, state.manifest.firmwareUrl)) {
    setStatus(state, "error", "Unable to open firmware download.");
    return false;
  }

  const int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    setStatus(state, "error", String("Firmware download failed (HTTP ") + httpCode + ").");
    http.end();
    return false;
  }

  const int reportedLength = http.getSize();
  state.contentLength = reportedLength > 0 ? static_cast<uint32_t>(reportedLength) : state.manifest.firmwareSize;
  WiFiClient* stream = http.getStreamPtr();
  if (stream == nullptr) {
    setStatus(state, "error", "Firmware download stream unavailable.");
    http.end();
    return false;
  }

  setStatus(state, "downloading", "Downloading and applying firmware...");

  const size_t updateTarget = state.contentLength > 0 ? state.contentLength : UPDATE_SIZE_UNKNOWN;
  if (!Update.begin(updateTarget, U_FLASH)) {
    setStatus(state, "error", "Not enough OTA space for the firmware image.");
    http.end();
    return false;
  }

  const size_t written = Update.writeStream(*stream);
  state.bytesWritten = static_cast<uint32_t>(written);
  http.end();

  if (state.contentLength > 0 && written != state.contentLength) {
    Update.abort();
    setStatus(state, "error", "Firmware download was incomplete.");
    return false;
  }

  if (!Update.end()) {
    setStatus(state, "error", "Firmware apply failed during finalization.");
    return false;
  }

  if (!Update.isFinished()) {
    setStatus(state, "error", "Firmware image did not finish writing.");
    return false;
  }

  state.rebootPending = true;
  setStatus(state, "reboot_pending", "Update installed. Rebooting...");
  return true;
}

}  // namespace

void initFirmwareUpdateState(FirmwareUpdateState& state,
                             const char* currentVersion,
                             const char* manifestUrl) {
  state.currentVersion = currentVersion != nullptr ? String(currentVersion) : String("0.0.0");
  state.manifestUrl = manifestUrl != nullptr ? String(manifestUrl) : String();
  state.status = "idle";
  state.message = "Ready to check for firmware updates.";
}

bool refreshFirmwareManifest(FirmwareUpdateState& state,
                             uint32_t nowMs,
                             bool networkAvailable,
                             bool forceRefresh) {
  if (state.updateInProgress || state.rebootPending) {
    return false;
  }

  if (!forceRefresh && state.manifestLoaded && (nowMs - state.lastCheckedMs) < FIRMWARE_REFRESH_CACHE_MS) {
    return false;
  }

  if (!networkAvailable) {
    if (forceRefresh || !state.manifestLoaded) {
      clearManifest(state);
      setStatus(state, "error", "Wi-Fi connection required to check for updates.");
    }
    return false;
  }

  setStatus(state, "checking", "Checking for firmware updates...");
  String payload;
  String errorMessage;
  if (!fetchManifestPayload(state.manifestUrl, payload, errorMessage)) {
    clearManifest(state);
    state.lastCheckedMs = nowMs;
    setStatus(state, "error", String("Update manifest unavailable (") + errorMessage + ").");
    return false;
  }

  FirmwareReleaseManifest manifest;
  if (!parseManifestPayload(payload, manifest)) {
    clearManifest(state);
    state.lastCheckedMs = nowMs;
    setStatus(state, "error", "Update manifest is invalid.");
    return false;
  }

  state.manifest = manifest;
  state.manifestLoaded = true;
  state.lastCheckedMs = nowMs;
  state.updateAvailable = compareVersions(state.currentVersion, state.manifest.version) < 0;
  if (state.updateAvailable) {
    setStatus(state, "update_available", String("Version ") + state.manifest.version + " is available.");
  } else {
    setStatus(state, "up_to_date", "This device is already up to date.");
  }

  return true;
}

bool queueFirmwareUpdate(FirmwareUpdateState& state, String& errorCode) {
  if (state.updateInProgress || state.rebootPending || state.updateQueued) {
    errorCode = "update_in_progress";
    return false;
  }
  if (!state.manifestLoaded) {
    errorCode = "manifest_unavailable";
    return false;
  }
  if (!state.updateAvailable) {
    errorCode = "no_update_available";
    return false;
  }
  if (state.manifest.firmwareUrl.length() == 0) {
    errorCode = "firmware_url_missing";
    return false;
  }

  state.updateQueued = true;
  setStatus(state, "queued", "Preparing firmware update...");
  errorCode = String();
  return true;
}

void processFirmwareUpdate(FirmwareUpdateState& state, uint32_t nowMs, bool networkAvailable) {
  if (state.rebootPending) {
    if (state.rebootAtMs == 0) {
      state.rebootAtMs = nowMs + FIRMWARE_REBOOT_DELAY_MS;
    }
    if ((nowMs - state.rebootAtMs) < 0x80000000UL && nowMs >= state.rebootAtMs) {
      delay(150);
      ESP.restart();
    }
    return;
  }

  if (!state.updateQueued || state.updateInProgress) {
    return;
  }

  if (!networkAvailable) {
    state.updateQueued = false;
    setStatus(state, "error", "Wi-Fi connection lost before OTA started.");
    return;
  }

  state.updateQueued = false;
  state.updateInProgress = true;
  state.bytesWritten = 0;
  state.contentLength = 0;
  const bool ok = performFirmwareUpdate(state);
  state.updateInProgress = false;
  if (ok) {
    state.rebootAtMs = nowMs + FIRMWARE_REBOOT_DELAY_MS;
  }
}

String buildFirmwareStatusJson(const FirmwareUpdateState& state) {
  const String latestVersion = state.manifest.version.length() > 0 ? state.manifest.version : state.currentVersion;
  String s = "{";
  s += "\"ok\":true";
  s += ",\"current_version\":\"" + jsonEscape(state.currentVersion) + "\"";
  s += ",\"latest_version\":\"" + jsonEscape(latestVersion) + "\"";
  s += ",\"update_available\":" + String(state.updateAvailable ? "true" : "false");
  s += ",\"manifest_loaded\":" + String(state.manifestLoaded ? "true" : "false");
  s += ",\"status\":\"" + jsonEscape(state.status) + "\"";
  s += ",\"message\":\"" + jsonEscape(state.message) + "\"";
  s += ",\"summary\":\"" + jsonEscape(state.manifest.summary) + "\"";
  s += ",\"notes_url\":\"" + jsonEscape(state.manifest.notesUrl) + "\"";
  s += ",\"firmware_url\":\"" + jsonEscape(state.manifest.firmwareUrl) + "\"";
  s += ",\"published_at\":\"" + jsonEscape(state.manifest.publishedAt) + "\"";
  s += ",\"sha256\":\"" + jsonEscape(state.manifest.checksumSha256) + "\"";
  s += ",\"firmware_size\":" + String(state.manifest.firmwareSize);
  s += ",\"bytes_written\":" + String(state.bytesWritten);
  s += ",\"content_length\":" + String(state.contentLength);
  s += ",\"checked_at_ms\":" + String(state.lastCheckedMs);
  s += "}";
  return s;
}