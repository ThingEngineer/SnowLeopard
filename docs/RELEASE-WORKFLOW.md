# SnowLeopard Release Workflow

This document is the operator checklist for cutting SnowLeopard firmware releases from this single repository.

## 1. What Controls OTA Availability

SnowLeopard only shows `Update available` when all of the following are true:

1. The running firmware version in `include/firmware_version.h` is lower than the version in `release-data/current.json`.
2. The device can reach the manifest URL declared in `include/firmware_version.h`.
3. The manifest points to a valid GitHub Release firmware binary.
4. The user opens `/settings`, then checks for updates from the Firmware Update section.

The device does not use a database. GitHub repo files and GitHub Releases are the source of truth.

## 1.1 GitHub Pages Prerequisite

Before the `Deploy Release Portal` workflow can succeed for the first time, enable GitHub Pages once in the repository settings.

In GitHub repository Settings > Pages:

1. Open the Pages settings.
2. Set the build source to GitHub Actions.
3. Save.

After that one-time setup, the workflow can use the default `GITHUB_TOKEN`.

Without that setup, `actions/configure-pages` fails with `Get Pages site failed` because the repository has no Pages site yet.

## 2. Files You Update For Every Release

- `include/firmware_version.h`
- `release-data/current.json`
- `release-data/releases/index.json`
- `release-data/releases/<version>.md`

## 3. Initial Release

Use this when publishing the first real GitHub release.

1. Choose the first real version number.
2. If your test devices are already running `0.1.0-dev`, use `0.1.1` or higher. The current firmware comparator ignores the `-dev` suffix and treats `0.1.0-dev` and `0.1.0` as equal.
3. Update `include/firmware_version.h` so `kSnowLeopardFirmwareVersion` matches that version.
4. Create `release-data/releases/<version>.md` with the release notes.
5. Add the new release entry at the top of `release-data/releases/index.json`.
6. Update `release-data/current.json`:
   - `version`
   - `summary`
   - `notes_url`
   - `firmware_url`
   - `published_at`
   - `sha256`
   - `size`
7. Commit and push the manifest, notes, and firmware version changes to `main`.
8. Create and push the matching git tag.
9. Wait for the `Build Firmware Release` GitHub Action to finish and publish the binary asset.
10. Copy the real GitHub Release binary URL, file size, and SHA-256 into `release-data/current.json` if you used placeholders before the release asset existed.
11. Push the final manifest update to `main`.
12. Wait for the `Deploy Release Portal` workflow to publish GitHub Pages.
13. On a device running an older version, open `/settings`, press `Check for update`, and confirm the new version appears.

## 4. Every Later Release

Use this for every firmware release after the first.

1. Choose the next version number.
2. Update `include/firmware_version.h` to the new version.
3. Add `release-data/releases/<version>.md`.
4. Insert the new release at the top of `release-data/releases/index.json`.
5. Update `release-data/current.json` to the new version and new release note URL.
6. Commit and push those changes to `main`.
7. Create and push the matching git tag `v<version>`.
8. After the firmware release workflow uploads the binary, confirm the artifact name and URL.
9. Update `release-data/current.json` with the final `firmware_url`, `sha256`, and `size` if needed.
10. Push the manifest correction to `main`.
11. Wait for GitHub Pages to redeploy.
12. On a device still running the older firmware, open `/settings`, press `Check for update`, and confirm that `Update now` becomes available.

## 5. Suggested Operator Sequence

If you want the smallest number of manual steps, use this order:

1. Prepare notes and manifest with the new version number.
2. Push to `main`.
3. Tag and push `v<version>`.
4. Let GitHub build and publish the binary.
5. Add the final checksum, size, and release asset URL to `release-data/current.json`.
6. Push the manifest update.

## 6. Rollback Procedure

If a release must be pulled:

1. Edit `release-data/current.json` so it points back to the previous good version.
2. If needed, move the previous good release entry to the top of `release-data/releases/index.json`.
3. Push the manifest change to `main`.
4. GitHub Pages redeploys automatically.
5. Devices checking for updates will stop advertising the pulled release as current.

## 7. Verification Checklist

Before and after a release:

1. `pio run --environment esp32-c3-devkitm-1`
2. `cd portal && npm run build`
3. Confirm `release-data/current.json` and `include/firmware_version.h` are intentionally aligned or intentionally different depending on whether you are testing update availability.
4. Confirm the GitHub Release asset URL in `release-data/current.json` resolves.
5. Confirm `/settings` on a test device shows the expected installed version and latest version.
