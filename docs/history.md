# History Feature

## Overview

SnowLeopard now includes a history system and web UI page for temperature and relay state trends.

Implemented components:

- Live high-resolution history in RAM (10-second samples)
- Reboot-survivable coarse history snapshots in NVS (60-minute samples)
- Phase 2 blended timeline API (snapshot for older ages + live for recent ages)
- HTTP API endpoint: `GET /api/history`
- Web page: `GET /history`
- Navigation links from status/settings pages to history

The history page shows:

- Internal temperature trend (always)
- External temperature trend (toggle)
- Relay ON state as chart background shading
- Reboot/discontinuity markers as vertical guide lines when present
- Range buttons: 5m, 15m, 30m, 1h, 3h, 6h, 12h, 24h

## How It Works

### 1) Sampling Layers

Two parallel histories are maintained:

1. Live history (RAM)

- Sample period: 10 seconds
- Capacity: 4320 samples
- Retention: ~12 hours
- Purpose: high-resolution recent trend

2. Snapshot history (NVS-backed ring)

- Sample period: 60 minutes
- Capacity: 48 samples
- Retention: ~48 hours stored internally
- Purpose: reboot-survivable trend history

### 2) Persistence Strategy

- Snapshot samples are added every 60 minutes.
- Snapshot ring is persisted to NVS every 5 minutes (dirty-write batching).
- A forced persist is performed before reboot paths triggered by provisioning save.
- On startup, snapshot ring metadata + buffer are loaded from NVS if valid.

This avoids writing every 10-second sample to flash while still retaining useful history across reboot.

### 3) API Behavior

`GET /api/history`

Query parameters:

- `window_s` (optional): requested window in seconds
- `step_s` (optional): requested plotting step in seconds
- `include_external` (optional): `1/0` (default `1`)

Response fields:

- `ok`
- `source`: `blended`
- `temp_unit`
- `period_s`
- `requested_window_s`
- `effective_step_s`
- `window_s`
- `retention_s`
- `age_s`: age-from-now array (seconds)
- `internal`: internal temperature series (display unit)
- `relay`: relay state series (0/1)
- `external`: external temperature series or `null`
- `discontinuity`: series (0/1), where `1` marks a reboot/session boundary in persisted summary history
- `discontinuity_age_s`: explicit age positions (seconds) for restart/session boundary markers
- `discontinuity_count`: number of restart/session boundaries represented in the response

Point count is bounded server-side (max 600) by auto-adjusting effective step.
The route handler clamps requested windows to 60 seconds minimum and 24 hours maximum, so retained coarse history older than 24 hours is kept internally but not directly exposed by the current API/UI.

### 3.1) Phase 2 Blended Timeline

The API now uses a blended source model and returns `source: "blended"`.

Selection behavior per plotted point:

- Use live history when the point age falls within live retention.
- Fall back to snapshot history for older ages.
- If neither source has data for an age, return `null` (temperature series) or `0` (relay series).

This produces a single continuous response timeline while preserving high detail for recent data.

Reboot/session semantics (option 2):

- Persisted summary history now stores a boot/session sequence marker.
- On startup, if persisted data came from a prior boot/session, the first newly persisted sample is marked as a discontinuity.
- API consumers can use the `discontinuity` array to display or handle history gaps explicitly.

### 4) UI Notes

The history page uses a lightweight canvas renderer (no external chart dependency):

- Grid + axes
- Internal line in cyan
- External line in orange
- Relay ON shading
- Optional vertical discontinuity markers
- Marker rendering clusters very-close boundaries so visual noise is reduced at coarse zoom levels
- Metadata line showing source/window/step/points and capped-request context
- Auto-refresh every 15 seconds while page is open

## Current Limits

- Immediately after reboot, retained span may be short until fresh live/snapshot samples accumulate.
- `period_s` is `0` for blended responses because data can come from mixed 10s and 60s sources.
- History absolute wall-clock timestamps are not currently emitted; API uses age-from-now.
- Reboot-survivable resolution is intentionally coarse (60 minutes).
- Requested windows larger than current retention are capped server-side and reported in metadata (`requested_window_s` vs `window_s`).

## Storage and Resource Considerations

Approximate history storage:

- Live buffer: 4320 \* 5 bytes ~= 21.1 KB
- Snapshot buffer: 48 \* 5 bytes ~= 0.24 KB
- Total history buffer RAM ~= 21.4 KB (+ small metadata/state)

NVS snapshot payload persisted:

- Full snapshot ring blob + small header
- Written every 5 minutes when new data exists

## Testing Performed

- Build and flash succeeded.
- `/history` page loads in browser.
- `/api/history` returns valid JSON payload with expected metadata and arrays.
- History range controls and external toggle trigger fetch/render updates.

## Next Possible Phases

1. Wall-clock timestamps

- Add optional epoch timestamps for easier correlation with external events.

2. CSV export

- Add `GET /api/history.csv` for manual analysis.

3. Persistent history partition

- Add a dedicated history partition for longer retention and more efficient append/write patterns.

4. Event annotations

- Mark relay transitions and mode changes directly on chart.

5. History settings page section

- Let user tune retention, persistence interval, and default window.
