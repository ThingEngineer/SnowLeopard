#include "firmware_update.h"

#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClientSecure.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <mbedtls/sha256.h>

void otaWorkerTask(void* context);

namespace {

static constexpr uint32_t FIRMWARE_REFRESH_CACHE_MS = 15UL * 60UL * 1000UL;
static constexpr uint32_t FIRMWARE_REBOOT_DELAY_MS = 1500UL;
static constexpr uint32_t FIRMWARE_STREAM_IDLE_TIMEOUT_MS = 5000UL;
static constexpr size_t FIRMWARE_DOWNLOAD_BUFFER_SIZE = 4096;

static const char kGitHubTrustedRootsPem[] = R"CERT(
-----BEGIN CERTIFICATE-----
MIICOjCCAcGgAwIBAgIQQvLM2htpN0RfFf51KBC49DAKBggqhkjOPQQDAzBfMQsw
CQYDVQQGEwJHQjEYMBYGA1UEChMPU2VjdGlnbyBMaW1pdGVkMTYwNAYDVQQDEy1T
ZWN0aWdvIFB1YmxpYyBTZXJ2ZXIgQXV0aGVudGljYXRpb24gUm9vdCBFNDYwHhcN
MjEwMzIyMDAwMDAwWhcNNDYwMzIxMjM1OTU5WjBfMQswCQYDVQQGEwJHQjEYMBYG
A1UEChMPU2VjdGlnbyBMaW1pdGVkMTYwNAYDVQQDEy1TZWN0aWdvIFB1YmxpYyBT
ZXJ2ZXIgQXV0aGVudGljYXRpb24gUm9vdCBFNDYwdjAQBgcqhkjOPQIBBgUrgQQA
IgNiAAR2+pmpbiDt+dd34wc7qNs9Xzjoq1WmVk/WSOrsfy2qw7LFeeyZYX8QeccC
WvkEN/U0NSt3zn8gj1KjAIns1aeibVvjS5KToID1AZTc8GgHHs3u/iVStSBDHBv+
6xnOQ6OjQjBAMB0GA1UdDgQWBBTRItpMWfFLXyY4qp3W7usNw/upYTAOBgNVHQ8B
Af8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAKBggqhkjOPQQDAwNnADBkAjAn7qRa
qCG76UeXlImldCBteU/IvZNeWBj7LRoAasm4PdCkT0RHlAFWovgzJQxC36oCMB3q
4S6ILuH5px0CMk7yn2xVdOOurvulGu7t0vzCAxHrRVxgED1cf5kDW21USAGKcw==
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)CERT";

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
  state.otaReady = false;
}

String normalizeLower(const String& value) {
  String normalized = value;
  normalized.trim();
  normalized.toLowerCase();
  return normalized;
}

String extractUrlHost(const String& url) {
  String normalized = normalizeLower(url);
  if (!normalized.startsWith("https://")) {
    return String();
  }

  int hostStart = 8;
  int hostEnd = normalized.indexOf('/', hostStart);
  if (hostEnd < 0) {
    hostEnd = normalized.length();
  }

  const int portPos = normalized.indexOf(':', hostStart);
  if (portPos >= 0 && portPos < hostEnd) {
    hostEnd = portPos;
  }

  if (hostEnd <= hostStart) {
    return String();
  }

  return normalized.substring(hostStart, hostEnd);
}

bool isTrustedGitHubHost(const String& host) {
  return host == "github.com" || host.endsWith(".github.io") || host.endsWith(".githubusercontent.com");
}

bool configureTrustedTlsClient(WiFiClientSecure& client, const String& url, String& errorMessage) {
  const String host = extractUrlHost(url);
  if (host.length() == 0) {
    errorMessage = "https_required";
    return false;
  }

  if (!isTrustedGitHubHost(host)) {
    errorMessage = "untrusted_host";
    return false;
  }

  client.setCACert(kGitHubTrustedRootsPem);
  return true;
}

bool isHexDigit(char ch) {
  return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

bool isValidSha256(const String& value) {
  if (value.length() != 64) {
    return false;
  }

  for (size_t i = 0; i < value.length(); ++i) {
    if (!isHexDigit(value.charAt(i))) {
      return false;
    }
  }
  return true;
}

String toHexSha256(const unsigned char digest[32]) {
  static const char kHex[] = "0123456789abcdef";
  String hex;
  hex.reserve(64);
  for (size_t i = 0; i < 32; ++i) {
    const unsigned char value = digest[i];
    hex += kHex[(value >> 4) & 0x0F];
    hex += kHex[value & 0x0F];
  }
  return hex;
}

bool manifestCanRunOta(const FirmwareReleaseManifest& manifest, String& reason) {
  if (manifest.firmwareUrl.length() == 0) {
    reason = "Firmware download URL is missing from the manifest.";
    return false;
  }
  if (extractUrlHost(manifest.firmwareUrl).length() == 0) {
    reason = "Firmware download URL must use HTTPS.";
    return false;
  }
  if (!isTrustedGitHubHost(extractUrlHost(manifest.firmwareUrl))) {
    reason = "Firmware download host is not in the GitHub allowlist.";
    return false;
  }
  if (!isValidSha256(manifest.checksumSha256)) {
    reason = "Manifest is missing a valid SHA-256 checksum.";
    return false;
  }
  if (manifest.firmwareSize == 0) {
    reason = "Manifest is missing the firmware file size.";
    return false;
  }

  reason = String();
  return true;
}

void logOta(const String& message) {
  Serial.print("[OTA] ");
  Serial.println(message);
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
  if (!configureTrustedTlsClient(client, manifestUrl, errorMessage)) {
    return false;
  }

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
  logOta("Starting OTA workflow.");

  WiFiClientSecure client;
  String tlsError;
  if (!configureTrustedTlsClient(client, state.manifest.firmwareUrl, tlsError)) {
    logOta(String("TLS configuration rejected: ") + tlsError);
    setStatus(state, "error", String("Firmware download rejected (") + tlsError + ").");
    return false;
  }

  HTTPClient http;
  http.setTimeout(1000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  logOta(String("Opening firmware URL: ") + state.manifest.firmwareUrl);
  if (!http.begin(client, state.manifest.firmwareUrl)) {
    logOta("HTTP begin failed.");
    setStatus(state, "error", "Unable to open firmware download.");
    return false;
  }

  setStatus(state, "downloading", "Starting firmware download request...");
  const int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    logOta(String("HTTP GET failed with code ") + httpCode);
    setStatus(state, "error", String("Firmware download failed (HTTP ") + httpCode + ").");
    http.end();
    return false;
  }
  logOta("HTTP GET accepted. Streaming firmware payload.");

  const int reportedLength = http.getSize();
  state.contentLength = reportedLength > 0 ? static_cast<uint32_t>(reportedLength) : state.manifest.firmwareSize;
  WiFiClient* stream = http.getStreamPtr();
  if (stream == nullptr) {
    setStatus(state, "error", "Firmware download stream unavailable.");
    http.end();
    return false;
  }

  state.checksumVerified = false;
  state.downloadSha256 = String();
  state.progressPercent = 0;
  setStatus(state, "downloading", "Downloading firmware...");
  uint8_t lastLoggedPercent = 0;

  const size_t updateTarget = state.contentLength > 0 ? state.contentLength : UPDATE_SIZE_UNKNOWN;
  if (!Update.begin(updateTarget, U_FLASH)) {
    setStatus(state, "error", "Not enough OTA space for the firmware image.");
    http.end();
    return false;
  }

  mbedtls_sha256_context sha256;
  mbedtls_sha256_init(&sha256);
  if (mbedtls_sha256_starts_ret(&sha256, 0) != 0) {
    Update.abort();
    http.end();
    setStatus(state, "error", "Unable to initialize SHA-256 verification.");
    return false;
  }

  uint8_t buffer[FIRMWARE_DOWNLOAD_BUFFER_SIZE];
  uint32_t lastDataMs = millis();
  while (http.connected() || stream->available() > 0) {
    const size_t available = stream->available();
    if (available == 0) {
      if (state.contentLength > 0 && state.bytesWritten >= state.contentLength) {
        break;
      }
      if ((millis() - lastDataMs) > FIRMWARE_STREAM_IDLE_TIMEOUT_MS) {
        logOta("Download stream idle timeout reached.");
        break;
      }
      delay(1);
      continue;
    }

    const size_t toRead = available > sizeof(buffer) ? sizeof(buffer) : available;
    const int readCount = stream->read(reinterpret_cast<uint8_t*>(buffer), toRead);
    if (readCount <= 0) {
      continue;
    }

    lastDataMs = millis();

    if (Update.write(buffer, static_cast<size_t>(readCount)) != static_cast<size_t>(readCount)) {
      mbedtls_sha256_free(&sha256);
      Update.abort();
      http.end();
      setStatus(state, "error", "Firmware write failed during OTA.");
      return false;
    }

    if (mbedtls_sha256_update_ret(&sha256, buffer, static_cast<size_t>(readCount)) != 0) {
      mbedtls_sha256_free(&sha256);
      Update.abort();
      http.end();
      setStatus(state, "error", "SHA-256 verification failed during download.");
      return false;
    }

    state.bytesWritten += static_cast<uint32_t>(readCount);
    if (state.contentLength > 0) {
      const uint32_t rawPercent = (state.bytesWritten * 100UL) / state.contentLength;
      state.progressPercent = static_cast<uint8_t>(rawPercent > 100UL ? 100UL : rawPercent);
      setStatus(state, "downloading", "Downloading firmware...");
      if (state.progressPercent >= static_cast<uint8_t>(lastLoggedPercent + 10) || state.progressPercent == 100) {
        lastLoggedPercent = state.progressPercent;
        logOta(String("Download progress: ") + state.progressPercent + "%.");
      }
    } else {
      setStatus(state, "downloading", "Downloading firmware...");
    }

    // Yield each chunk so async networking tasks can run during long OTA downloads.
    delay(1);
  }

  unsigned char digest[32];
  if (mbedtls_sha256_finish_ret(&sha256, digest) != 0) {
    logOta("SHA-256 finalize failed.");
    mbedtls_sha256_free(&sha256);
    Update.abort();
    http.end();
    setStatus(state, "error", "Unable to finalize SHA-256 verification.");
    return false;
  }
  mbedtls_sha256_free(&sha256);
  state.downloadSha256 = toHexSha256(digest);
  http.end();

  if (state.contentLength > 0 && state.bytesWritten != state.contentLength) {
    logOta("Download size mismatch against HTTP content-length.");
    Update.abort();
    setStatus(state, "error", "Firmware download was incomplete.");
    return false;
  }

  if (state.manifest.firmwareSize > 0 && state.bytesWritten != state.manifest.firmwareSize) {
    logOta("Download size mismatch against manifest size.");
    Update.abort();
    setStatus(state, "error", "Downloaded firmware size does not match the manifest.");
    return false;
  }

  const String expectedSha256 = normalizeLower(state.manifest.checksumSha256);
  if (state.downloadSha256 != expectedSha256) {
    logOta("SHA-256 mismatch.");
    Update.abort();
    setStatus(state, "error", "Downloaded firmware checksum does not match the manifest.");
    return false;
  }

  state.checksumVerified = true;
  state.progressPercent = 100;
  setStatus(state, "verifying", "Download complete. Verifying and installing firmware...");

  if (!Update.end()) {
    logOta("Update.end() failed.");
    setStatus(state, "error", "Firmware apply failed during finalization.");
    return false;
  }

  if (!Update.isFinished()) {
    logOta("Update image not marked finished.");
    setStatus(state, "error", "Firmware image did not finish writing.");
    return false;
  }

  logOta("OTA image written successfully; scheduling reboot.");
  state.rebootPending = true;
  setStatus(state, "reboot_pending", "Firmware installed. Rebooting device...");
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
  state.progressPercent = 0;
}

bool refreshFirmwareManifest(FirmwareUpdateState& state,
                             uint32_t nowMs,
                             bool networkAvailable,
                             bool forceRefresh) {
  if (state.updateQueued || state.updateInProgress || state.rebootPending) {
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
  state.downloadSha256 = String();
  state.checksumVerified = false;
  state.bytesWritten = 0;
  state.contentLength = 0;
  state.progressPercent = 0;

  const bool newerVersionAvailable = compareVersions(state.currentVersion, state.manifest.version) < 0;
  String otaReason;
  state.otaReady = newerVersionAvailable && manifestCanRunOta(state.manifest, otaReason);
  state.updateAvailable = newerVersionAvailable;
  if (newerVersionAvailable && state.otaReady) {
    setStatus(state, "update_available", String("Version ") + state.manifest.version + " is available.");
  } else if (newerVersionAvailable) {
    setStatus(state, "update_available", String("Version ") + state.manifest.version + " is available, but OTA is blocked: " + otaReason);
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
  if (!state.otaReady) {
    errorCode = "manifest_incomplete";
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
    // Keep the update queued and retry automatically when STA reconnects.
    setStatus(state, "queued", "Waiting for Wi-Fi before OTA starts...");
    return;
  }

  state.updateQueued = false;
  state.updateInProgress = true;
  state.checksumVerified = false;
  state.downloadSha256 = String();
  state.bytesWritten = 0;
  state.contentLength = 0;
  state.progressPercent = 0;
  setStatus(state, "queued", "Starting OTA worker...");
  logOta("Queue accepted. Launching OTA worker task.");

  BaseType_t workerCreated = xTaskCreate(::otaWorkerTask,
                                         "ota_worker",
                                         12288,
                                         &state,
                                         1,
                                         nullptr);
  if (workerCreated != pdPASS) {
    state.updateInProgress = false;
    setStatus(state, "error", "Unable to start OTA worker task.");
    logOta("Failed to create OTA worker task.");
  }
}

void otaWorkerTask(void* context) {
  FirmwareUpdateState* state = static_cast<FirmwareUpdateState*>(context);
  if (state == nullptr) {
    vTaskDelete(nullptr);
    return;
  }

  const bool ok = performFirmwareUpdate(*state);
  state->updateInProgress = false;
  if (ok) {
    state->rebootAtMs = millis() + FIRMWARE_REBOOT_DELAY_MS;
    logOta("OTA worker completed successfully.");
  } else {
    logOta("OTA worker finished with failure.");
  }

  vTaskDelete(nullptr);
}

String buildFirmwareStatusJson(const FirmwareUpdateState& state) {
  const String latestVersion = state.manifest.version.length() > 0 ? state.manifest.version : state.currentVersion;
  String s = "{";
  s += "\"ok\":true";
  s += ",\"current_version\":\"" + jsonEscape(state.currentVersion) + "\"";
  s += ",\"latest_version\":\"" + jsonEscape(latestVersion) + "\"";
  s += ",\"update_available\":" + String(state.updateAvailable ? "true" : "false");
  s += ",\"ota_ready\":" + String(state.otaReady ? "true" : "false");
  s += ",\"manifest_loaded\":" + String(state.manifestLoaded ? "true" : "false");
  s += ",\"status\":\"" + jsonEscape(state.status) + "\"";
  s += ",\"message\":\"" + jsonEscape(state.message) + "\"";
  s += ",\"summary\":\"" + jsonEscape(state.manifest.summary) + "\"";
  s += ",\"notes_url\":\"" + jsonEscape(state.manifest.notesUrl) + "\"";
  s += ",\"firmware_url\":\"" + jsonEscape(state.manifest.firmwareUrl) + "\"";
  s += ",\"published_at\":\"" + jsonEscape(state.manifest.publishedAt) + "\"";
  s += ",\"sha256\":\"" + jsonEscape(state.manifest.checksumSha256) + "\"";
  s += ",\"download_sha256\":\"" + jsonEscape(state.downloadSha256) + "\"";
  s += ",\"checksum_verified\":" + String(state.checksumVerified ? "true" : "false");
  s += ",\"firmware_size\":" + String(state.manifest.firmwareSize);
  s += ",\"bytes_written\":" + String(state.bytesWritten);
  s += ",\"content_length\":" + String(state.contentLength);
  s += ",\"progress_percent\":" + String(state.progressPercent);
  s += ",\"checked_at_ms\":" + String(state.lastCheckedMs);
  s += "}";
  return s;
}