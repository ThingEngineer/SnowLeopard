#include "history_store.h"

namespace {

static constexpr uint32_t HISTORY_SAMPLE_MS = 10000;
static constexpr uint32_t HISTORY_SNAPSHOT_SAMPLE_MS = 3600000;
static constexpr uint32_t HISTORY_SNAPSHOT_PERSIST_MS = 300000;
static constexpr uint16_t HISTORY_LIVE_CAPACITY = 4320;
static constexpr uint16_t HISTORY_SNAPSHOT_CAPACITY = 72;
static constexpr uint16_t HISTORY_API_MAX_POINTS = 600;

static constexpr uint8_t HISTORY_FLAG_INTERNAL_VALID = 0x01;
static constexpr uint8_t HISTORY_FLAG_EXTERNAL_VALID = 0x02;
static constexpr uint8_t HISTORY_FLAG_RELAY_ON = 0x04;
static constexpr uint8_t HISTORY_FLAG_DISCONTINUITY = 0x08;

struct HistorySample {
  int16_t internalTempC10;
  int16_t externalTempC10;
  uint8_t flags;
} __attribute__((packed));

struct HistorySnapshotHeaderV1 {
  uint16_t version;
  uint16_t head;
  uint16_t count;
  uint16_t capacity;
  uint32_t periodMs;
} __attribute__((packed));

struct HistorySnapshotHeader {
  uint16_t version;
  uint16_t head;
  uint16_t count;
  uint16_t capacity;
  uint32_t periodMs;
  uint32_t bootSequence;
} __attribute__((packed));

struct HistorySnapshotBlobHeader {
  uint32_t magic;
  uint16_t version;
  uint16_t head;
  uint16_t count;
  uint16_t capacity;
  uint32_t periodMs;
  uint32_t bootSequence;
  uint32_t payloadCrc32;
} __attribute__((packed));

struct HistorySnapshotBlob {
  HistorySnapshotBlobHeader header;
  HistorySample samples[HISTORY_SNAPSHOT_CAPACITY];
} __attribute__((packed));

static constexpr uint16_t HISTORY_SNAPSHOT_VERSION_V1 = 1;
static constexpr uint16_t HISTORY_SNAPSHOT_VERSION = 2;
static constexpr uint16_t HISTORY_SNAPSHOT_BLOB_VERSION = 3;
static constexpr uint32_t HISTORY_SNAPSHOT_BLOB_MAGIC = 0x48535452UL;  // "HSTR"

static constexpr const char* HISTORY_SNAPSHOT_HDR_KEY = "hist_snap_hdr";
static constexpr const char* HISTORY_SNAPSHOT_BUF_KEY = "hist_snap_buf";
static constexpr const char* HISTORY_SNAPSHOT_BLOB_KEY = "hist_snap_blob";

HistorySample liveHistory[HISTORY_LIVE_CAPACITY] = {};
HistorySample snapshotHistory[HISTORY_SNAPSHOT_CAPACITY] = {};
uint16_t liveHistoryHead = 0;
uint16_t liveHistoryCount = 0;
uint16_t snapshotHistoryHead = 0;
uint16_t snapshotHistoryCount = 0;
uint32_t lastHistorySampleMs = 0;
uint32_t lastHistorySnapshotSampleMs = 0;
uint32_t lastHistorySnapshotPersistMs = 0;
bool historySnapshotDirty = false;
uint32_t historyBootSequence = 0;
bool historySnapshotNeedsDiscontinuityMarker = false;
bool historyLiveNeedsDiscontinuityMarker = false;

uint32_t crc32(const uint8_t* data, size_t len) {
  uint32_t crc = 0xFFFFFFFFUL;
  for (size_t i = 0; i < len; i++) {
    crc ^= static_cast<uint32_t>(data[i]);
    for (uint8_t bit = 0; bit < 8; bit++) {
      const bool lsbSet = (crc & 1UL) != 0;
      crc >>= 1;
      if (lsbSet) {
        crc ^= 0xEDB88320UL;
      }
    }
  }
  return ~crc;
}

int16_t encodeTempC10(float valueC) {
  if (!isfinite(valueC)) {
    return 0;
  }
  float scaled = valueC * 10.0f;
  if (scaled > 32767.0f) {
    scaled = 32767.0f;
  }
  if (scaled < -32768.0f) {
    scaled = -32768.0f;
  }
  return static_cast<int16_t>(scaled);
}

float decodeTempC10(int16_t valueC10) {
  return static_cast<float>(valueC10) / 10.0f;
}

void pushHistorySample(HistorySample* buffer,
                       uint16_t capacity,
                       uint16_t& head,
                       uint16_t& count,
                       const HistorySample& sample) {
  buffer[head] = sample;
  head = static_cast<uint16_t>((head + 1U) % capacity);
  if (count < capacity) {
    count++;
  }
}

HistorySample historySampleByNewestOffset(const HistorySample* buffer,
                                          uint16_t capacity,
                                          uint16_t head,
                                          uint16_t count,
                                          uint16_t newestOffset) {
  HistorySample empty = {0, 0, 0};
  if (count == 0 || newestOffset >= count) {
    return empty;
  }

  const uint32_t newestIndex = (head + capacity - 1U) % capacity;
  const uint32_t index = (newestIndex + capacity - newestOffset) % capacity;
  return buffer[index];
}

bool historySampleAtAgeSeconds(const HistorySample* buffer,
                               uint16_t capacity,
                               uint16_t head,
                               uint16_t count,
                               uint32_t periodSeconds,
                               uint32_t ageSeconds,
                               HistorySample& outSample) {
  if (count == 0 || periodSeconds == 0) {
    return false;
  }

  const uint32_t maxAgeSeconds = static_cast<uint32_t>(count - 1U) * periodSeconds;
  if (ageSeconds > maxAgeSeconds) {
    return false;
  }

  const uint32_t newestOffset = (ageSeconds + (periodSeconds / 2U)) / periodSeconds;
  if (newestOffset >= count) {
    return false;
  }

  outSample = historySampleByNewestOffset(buffer, capacity, head, count, static_cast<uint16_t>(newestOffset));
  return true;
}

String historyValueOrNull(bool valid, float valueDisplay) {
  if (!valid || !isfinite(valueDisplay)) {
    return "null";
  }
  return String(valueDisplay, 2);
}

}  // namespace

namespace historystore {

void begin(uint32_t bootSequence) {
  historyBootSequence = bootSequence;
}

void primeTimers(uint32_t nowMs) {
  lastHistorySampleMs = nowMs - HISTORY_SAMPLE_MS;
  lastHistorySnapshotSampleMs = nowMs - HISTORY_SNAPSHOT_SAMPLE_MS;
  lastHistorySnapshotPersistMs = nowMs;
}

void loadSnapshot(Preferences& preferences) {
  snapshotHistoryHead = 0;
  snapshotHistoryCount = 0;
  historySnapshotNeedsDiscontinuityMarker = false;
  historyLiveNeedsDiscontinuityMarker = false;

  const size_t blobLen = preferences.isKey(HISTORY_SNAPSHOT_BLOB_KEY)
                         ? preferences.getBytesLength(HISTORY_SNAPSHOT_BLOB_KEY)
                         : 0U;
  if (blobLen == sizeof(HistorySnapshotBlob)) {
    HistorySnapshotBlob* blob = static_cast<HistorySnapshotBlob*>(malloc(sizeof(HistorySnapshotBlob)));
    if (blob == nullptr) {
      Serial.println("History snapshot load skipped: insufficient heap for blob");
    } else {
      memset(blob, 0, sizeof(HistorySnapshotBlob));
      if (preferences.getBytes(HISTORY_SNAPSHOT_BLOB_KEY, blob, sizeof(HistorySnapshotBlob)) == sizeof(HistorySnapshotBlob)) {
        const HistorySnapshotBlobHeader& header = blob->header;
        if (header.magic == HISTORY_SNAPSHOT_BLOB_MAGIC && header.version == HISTORY_SNAPSHOT_BLOB_VERSION &&
            header.capacity == HISTORY_SNAPSHOT_CAPACITY && header.periodMs == HISTORY_SNAPSHOT_SAMPLE_MS &&
            header.head < HISTORY_SNAPSHOT_CAPACITY && header.count <= HISTORY_SNAPSHOT_CAPACITY) {
          const uint32_t expectedCrc = crc32(reinterpret_cast<const uint8_t*>(blob->samples), sizeof(blob->samples));
          if (expectedCrc == header.payloadCrc32) {
            memcpy(snapshotHistory, blob->samples, sizeof(snapshotHistory));
            snapshotHistoryHead = header.head;
            snapshotHistoryCount = header.count;
            historySnapshotNeedsDiscontinuityMarker =
              (snapshotHistoryCount > 0 && header.bootSequence != historyBootSequence);
            historyLiveNeedsDiscontinuityMarker = historySnapshotNeedsDiscontinuityMarker;
            free(blob);
            return;
          }
          Serial.println("History snapshot blob CRC mismatch; ignoring persisted blob");
        }
      }
      free(blob);
    }
  }

  const size_t bufLen = preferences.isKey(HISTORY_SNAPSHOT_BUF_KEY)
                        ? preferences.getBytesLength(HISTORY_SNAPSHOT_BUF_KEY)
                        : 0U;
  if (bufLen != sizeof(snapshotHistory)) {
    return;
  }

  const size_t hdrLen = preferences.isKey(HISTORY_SNAPSHOT_HDR_KEY)
                        ? preferences.getBytesLength(HISTORY_SNAPSHOT_HDR_KEY)
                        : 0U;
  if (hdrLen == sizeof(HistorySnapshotHeader)) {
    HistorySnapshotHeader header{};
    if (preferences.getBytes(HISTORY_SNAPSHOT_HDR_KEY, &header, sizeof(header)) != sizeof(header)) {
      return;
    }

    if (header.version != HISTORY_SNAPSHOT_VERSION || header.capacity != HISTORY_SNAPSHOT_CAPACITY ||
        header.periodMs != HISTORY_SNAPSHOT_SAMPLE_MS || header.head >= HISTORY_SNAPSHOT_CAPACITY ||
        header.count > HISTORY_SNAPSHOT_CAPACITY) {
      return;
    }

    if (preferences.getBytes(HISTORY_SNAPSHOT_BUF_KEY, snapshotHistory, sizeof(snapshotHistory)) != sizeof(snapshotHistory)) {
      return;
    }

    snapshotHistoryHead = header.head;
    snapshotHistoryCount = header.count;
    historySnapshotNeedsDiscontinuityMarker = (snapshotHistoryCount > 0 && header.bootSequence != historyBootSequence);
    historyLiveNeedsDiscontinuityMarker = historySnapshotNeedsDiscontinuityMarker;
    return;
  }

  if (hdrLen != sizeof(HistorySnapshotHeaderV1)) {
    return;
  }

  HistorySnapshotHeaderV1 legacyHeader{};
  if (preferences.getBytes(HISTORY_SNAPSHOT_HDR_KEY, &legacyHeader, sizeof(legacyHeader)) != sizeof(legacyHeader)) {
    return;
  }

  if (legacyHeader.version != HISTORY_SNAPSHOT_VERSION_V1 || legacyHeader.capacity != HISTORY_SNAPSHOT_CAPACITY ||
      legacyHeader.periodMs != HISTORY_SNAPSHOT_SAMPLE_MS || legacyHeader.head >= HISTORY_SNAPSHOT_CAPACITY ||
      legacyHeader.count > HISTORY_SNAPSHOT_CAPACITY) {
    return;
  }

  if (preferences.getBytes(HISTORY_SNAPSHOT_BUF_KEY, snapshotHistory, sizeof(snapshotHistory)) != sizeof(snapshotHistory)) {
    return;
  }

  snapshotHistoryHead = legacyHeader.head;
  snapshotHistoryCount = legacyHeader.count;
  historySnapshotNeedsDiscontinuityMarker = (snapshotHistoryCount > 0);
  historyLiveNeedsDiscontinuityMarker = historySnapshotNeedsDiscontinuityMarker;
}

void persistSnapshot(Preferences& preferences, bool force) {
  const uint32_t nowMs = millis();
  if (!historySnapshotDirty) {
    return;
  }

  if (!force && (nowMs - lastHistorySnapshotPersistMs) < HISTORY_SNAPSHOT_PERSIST_MS) {
    return;
  }

  HistorySnapshotBlob* blob = static_cast<HistorySnapshotBlob*>(malloc(sizeof(HistorySnapshotBlob)));
  if (blob == nullptr) {
    Serial.println("History snapshot persist skipped: insufficient heap for blob");
    return;
  }

  memset(blob, 0, sizeof(HistorySnapshotBlob));
  blob->header.magic = HISTORY_SNAPSHOT_BLOB_MAGIC;
  blob->header.version = HISTORY_SNAPSHOT_BLOB_VERSION;
  blob->header.head = snapshotHistoryHead;
  blob->header.count = snapshotHistoryCount;
  blob->header.capacity = HISTORY_SNAPSHOT_CAPACITY;
  blob->header.periodMs = HISTORY_SNAPSHOT_SAMPLE_MS;
  blob->header.bootSequence = historyBootSequence;
  memcpy(blob->samples, snapshotHistory, sizeof(snapshotHistory));
  blob->header.payloadCrc32 = crc32(reinterpret_cast<const uint8_t*>(blob->samples), sizeof(blob->samples));

  size_t bytesWritten = preferences.putBytes(HISTORY_SNAPSHOT_BLOB_KEY, blob, sizeof(HistorySnapshotBlob));
  if (bytesWritten != sizeof(HistorySnapshotBlob)) {
    if (preferences.isKey(HISTORY_SNAPSHOT_HDR_KEY)) {
      preferences.remove(HISTORY_SNAPSHOT_HDR_KEY);
    }
    if (preferences.isKey(HISTORY_SNAPSHOT_BUF_KEY)) {
      preferences.remove(HISTORY_SNAPSHOT_BUF_KEY);
    }
    bytesWritten = preferences.putBytes(HISTORY_SNAPSHOT_BLOB_KEY, blob, sizeof(HistorySnapshotBlob));
  }

  if (bytesWritten != sizeof(HistorySnapshotBlob)) {
    Serial.printf("History snapshot persist failed (%u/%u bytes)\n",
                  static_cast<unsigned>(bytesWritten),
                  static_cast<unsigned>(sizeof(HistorySnapshotBlob)));
    // Throttle repeated persist attempts after a failure to avoid serial spam.
    lastHistorySnapshotPersistMs = nowMs;
    free(blob);
    return;
  }

  if (preferences.isKey(HISTORY_SNAPSHOT_HDR_KEY)) {
    preferences.remove(HISTORY_SNAPSHOT_HDR_KEY);
  }
  if (preferences.isKey(HISTORY_SNAPSHOT_BUF_KEY)) {
    preferences.remove(HISTORY_SNAPSHOT_BUF_KEY);
  }

  free(blob);
  historySnapshotDirty = false;
  lastHistorySnapshotPersistMs = nowMs;
}

void recordSample(uint32_t nowMs,
                  float internalTempC,
                  float externalTempC,
                  bool internalValid,
                  bool externalValid,
                  bool relayOn) {
  if ((nowMs - lastHistorySampleMs) >= HISTORY_SAMPLE_MS) {
    lastHistorySampleMs = nowMs;
    HistorySample sample{};
    sample.internalTempC10 = encodeTempC10(internalTempC);
    sample.externalTempC10 = encodeTempC10(externalTempC);
    sample.flags = 0;
    if (internalValid && isfinite(internalTempC)) {
      sample.flags |= HISTORY_FLAG_INTERNAL_VALID;
    }
    if (externalValid && isfinite(externalTempC)) {
      sample.flags |= HISTORY_FLAG_EXTERNAL_VALID;
    }
    if (relayOn) {
      sample.flags |= HISTORY_FLAG_RELAY_ON;
    }
    if (historyLiveNeedsDiscontinuityMarker) {
      sample.flags |= HISTORY_FLAG_DISCONTINUITY;
      historyLiveNeedsDiscontinuityMarker = false;
    }
    pushHistorySample(liveHistory, HISTORY_LIVE_CAPACITY, liveHistoryHead, liveHistoryCount, sample);
  }

  if ((nowMs - lastHistorySnapshotSampleMs) >= HISTORY_SNAPSHOT_SAMPLE_MS) {
    lastHistorySnapshotSampleMs = nowMs;
    HistorySample sample{};
    sample.internalTempC10 = encodeTempC10(internalTempC);
    sample.externalTempC10 = encodeTempC10(externalTempC);
    sample.flags = 0;
    if (internalValid && isfinite(internalTempC)) {
      sample.flags |= HISTORY_FLAG_INTERNAL_VALID;
    }
    if (externalValid && isfinite(externalTempC)) {
      sample.flags |= HISTORY_FLAG_EXTERNAL_VALID;
    }
    if (relayOn) {
      sample.flags |= HISTORY_FLAG_RELAY_ON;
    }
    if (historySnapshotNeedsDiscontinuityMarker) {
      sample.flags |= HISTORY_FLAG_DISCONTINUITY;
      historySnapshotNeedsDiscontinuityMarker = false;
    }
    pushHistorySample(snapshotHistory, HISTORY_SNAPSHOT_CAPACITY, snapshotHistoryHead, snapshotHistoryCount, sample);
    historySnapshotDirty = true;
  }
}

String buildBlendedHistoryJson(uint32_t windowSeconds,
                               uint32_t requestedStepSeconds,
                               bool includeExternal,
                               const char* tempUnit,
                               float (*tempToDisplayFn)(float)) {
  const uint32_t livePeriodSeconds = HISTORY_SAMPLE_MS / 1000U;
  const uint32_t snapshotPeriodSeconds = HISTORY_SNAPSHOT_SAMPLE_MS / 1000U;
  const uint32_t liveRetentionSeconds = (liveHistoryCount > 0) ? static_cast<uint32_t>(liveHistoryCount - 1U) * livePeriodSeconds : 0U;
  const uint32_t snapshotRetentionSeconds =
    (snapshotHistoryCount > 0) ? static_cast<uint32_t>(snapshotHistoryCount - 1U) * snapshotPeriodSeconds : 0U;
  const uint32_t totalRetentionSeconds = max(liveRetentionSeconds, snapshotRetentionSeconds);
  const uint32_t boundedWindowSeconds = min(windowSeconds, totalRetentionSeconds);

  if (totalRetentionSeconds == 0 || boundedWindowSeconds == 0) {
    return "{\"ok\":true,\"source\":\"blended\",\"temp_unit\":\"" + String(tempUnit) +
           "\",\"period_s\":0,\"requested_window_s\":" + String(windowSeconds) +
           "\",\"effective_step_s\":0,\"window_s\":0,\"retention_s\":0,\"age_s\":[],\"internal\":[],\"relay\":[],\"external\":[],\"discontinuity\":[],\"discontinuity_age_s\":[],\"discontinuity_count\":0}";
  }

  uint32_t stepSeconds = requestedStepSeconds;
  if (stepSeconds == 0) {
    stepSeconds = livePeriodSeconds;
  }
  if (stepSeconds < livePeriodSeconds) {
    stepSeconds = livePeriodSeconds;
  }
  while (((boundedWindowSeconds / stepSeconds) + 1U) > HISTORY_API_MAX_POINTS) {
    stepSeconds += livePeriodSeconds;
  }

  String s = "{";
  s += "\"ok\":true";
  s += ",\"source\":\"blended\"";
  s += ",\"temp_unit\":\"" + String(tempUnit) + "\"";
  s += ",\"period_s\":0";
  s += ",\"requested_window_s\":" + String(windowSeconds);
  s += ",\"effective_step_s\":" + String(stepSeconds);
  s += ",\"window_s\":" + String(boundedWindowSeconds);
  s += ",\"retention_s\":" + String(totalRetentionSeconds);

  s += ",\"age_s\":[";
  bool first = true;
  for (int32_t age = static_cast<int32_t>(boundedWindowSeconds); age >= 0; age -= static_cast<int32_t>(stepSeconds)) {
    if (!first) {
      s += ",";
    }
    first = false;
    s += String(static_cast<uint32_t>(age));
  }
  s += "]";

  auto emitSeries = [&](const char* name, bool relaySeries, bool externalSeries) {
    s += ",\"" + String(name) + "\":";
    if (externalSeries && !includeExternal) {
      s += "null";
      return;
    }

    s += "[";
    bool localFirst = true;
    for (int32_t age = static_cast<int32_t>(boundedWindowSeconds); age >= 0; age -= static_cast<int32_t>(stepSeconds)) {
      if (!localFirst) {
        s += ",";
      }
      localFirst = false;

      HistorySample sample{};
      bool found = false;
      const uint32_t ageU = static_cast<uint32_t>(age);
      if (ageU <= liveRetentionSeconds) {
        found = historySampleAtAgeSeconds(
          liveHistory, HISTORY_LIVE_CAPACITY, liveHistoryHead, liveHistoryCount, livePeriodSeconds, ageU, sample);
      }
      if (!found) {
        found = historySampleAtAgeSeconds(snapshotHistory,
                                          HISTORY_SNAPSHOT_CAPACITY,
                                          snapshotHistoryHead,
                                          snapshotHistoryCount,
                                          snapshotPeriodSeconds,
                                          ageU,
                                          sample);
      }

      if (!found) {
        s += relaySeries ? "0" : "null";
        continue;
      }

      if (relaySeries) {
        s += ((sample.flags & HISTORY_FLAG_RELAY_ON) ? "1" : "0");
      } else if (externalSeries) {
        const bool valid = (sample.flags & HISTORY_FLAG_EXTERNAL_VALID) != 0;
        s += historyValueOrNull(valid, tempToDisplayFn(decodeTempC10(sample.externalTempC10)));
      } else {
        const bool valid = (sample.flags & HISTORY_FLAG_INTERNAL_VALID) != 0;
        s += historyValueOrNull(valid, tempToDisplayFn(decodeTempC10(sample.internalTempC10)));
      }
    }
    s += "]";
  };

  emitSeries("internal", false, false);
  emitSeries("relay", true, false);
  emitSeries("external", false, true);

  uint32_t discontinuityAges[64] = {};
  size_t discontinuityAgeCount = 0;
  auto addDiscontinuityAge = [&](uint32_t ageSeconds) {
    if (ageSeconds > boundedWindowSeconds || discontinuityAgeCount >= 64) {
      return;
    }

    const uint32_t dedupeWindow = max(livePeriodSeconds, snapshotPeriodSeconds);
    for (size_t i = 0; i < discontinuityAgeCount; i++) {
      const uint32_t existing = discontinuityAges[i];
      const uint32_t diff = (existing > ageSeconds) ? (existing - ageSeconds) : (ageSeconds - existing);
      if (diff <= dedupeWindow) {
        return;
      }
    }

    discontinuityAges[discontinuityAgeCount++] = ageSeconds;
  };

  for (uint16_t offset = 0; offset < liveHistoryCount; offset++) {
    const HistorySample sample = historySampleByNewestOffset(liveHistory, HISTORY_LIVE_CAPACITY, liveHistoryHead, liveHistoryCount, offset);
    if ((sample.flags & HISTORY_FLAG_DISCONTINUITY) != 0) {
      addDiscontinuityAge(static_cast<uint32_t>(offset) * livePeriodSeconds);
    }
  }
  for (uint16_t offset = 0; offset < snapshotHistoryCount; offset++) {
    const HistorySample sample =
      historySampleByNewestOffset(snapshotHistory, HISTORY_SNAPSHOT_CAPACITY, snapshotHistoryHead, snapshotHistoryCount, offset);
    if ((sample.flags & HISTORY_FLAG_DISCONTINUITY) != 0) {
      addDiscontinuityAge(static_cast<uint32_t>(offset) * snapshotPeriodSeconds);
    }
  }

  for (size_t i = 0; i < discontinuityAgeCount; i++) {
    for (size_t j = i + 1; j < discontinuityAgeCount; j++) {
      if (discontinuityAges[j] > discontinuityAges[i]) {
        const uint32_t tmp = discontinuityAges[i];
        discontinuityAges[i] = discontinuityAges[j];
        discontinuityAges[j] = tmp;
      }
    }
  }

  s += ",\"discontinuity\":[";
  bool discFirst = true;
  for (int32_t age = static_cast<int32_t>(boundedWindowSeconds); age >= 0; age -= static_cast<int32_t>(stepSeconds)) {
    if (!discFirst) {
      s += ",";
    }
    discFirst = false;

    HistorySample sample{};
    bool found = false;
    const uint32_t ageU = static_cast<uint32_t>(age);
    if (ageU <= liveRetentionSeconds) {
      found = historySampleAtAgeSeconds(
        liveHistory, HISTORY_LIVE_CAPACITY, liveHistoryHead, liveHistoryCount, livePeriodSeconds, ageU, sample);
    }
    if (!found) {
      found = historySampleAtAgeSeconds(snapshotHistory,
                                        HISTORY_SNAPSHOT_CAPACITY,
                                        snapshotHistoryHead,
                                        snapshotHistoryCount,
                                        snapshotPeriodSeconds,
                                        ageU,
                                        sample);
    }
    s += (found && ((sample.flags & HISTORY_FLAG_DISCONTINUITY) != 0) ? "1" : "0");
  }
  s += "]";

  s += ",\"discontinuity_age_s\":[";
  for (size_t i = 0; i < discontinuityAgeCount; i++) {
    if (i > 0) {
      s += ",";
    }
    s += String(discontinuityAges[i]);
  }
  s += "]";
  s += ",\"discontinuity_count\":" + String(discontinuityAgeCount);

  s += "}";
  return s;
}

}  // namespace historystore
