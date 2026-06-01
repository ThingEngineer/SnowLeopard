---
name: publish-ota-release
description: "Publish a SnowLeopard OTA release end-to-end. Use when pushing release prep commits, creating and pushing a v* tag, verifying GitHub Actions workflows, and finalizing release-data/current.json sha256 and size from the published firmware asset."
argument-hint: "Target version, for example 0.1.1"
user-invocable: true
disable-model-invocation: false
---

# Publish OTA Release

Run the full SnowLeopard release flow with GitHub Releases and GitHub Pages as the source of truth.

## When To Use

- You are ready to publish a firmware version.
- `release-data` and firmware version constants are already prepared.
- You need to finalize manifest `sha256` and `size` after release assets are published.

## Inputs

- Target version in `X.Y.Z` format.
- Optional release date (defaults to current date in `release-data/current.json`).

## Procedure

1. Preflight checks:
   - Confirm clean git tree.
   - Confirm release metadata/version files are updated for the target version.
2. Push release prep commits to the branch that will feed the release:
   - For the current develop-first flow, push `develop` before tagging.
3. Create and push annotated tag:
   - `git tag -a v<version> -m "SnowLeopard v<version>"`
   - `git push origin v<version>`
4. Verify the tag points to the intended release commit:
   - Use `git rev-parse v<version>^{}` to compare the peeled tag commit, not `git rev-parse v<version>` which returns the annotated tag object id.
5. Verify GitHub Actions status:
   - `Build Firmware Release` for tag must succeed.
   - `Deploy Release Portal` for `main` should be green.
6. Wait until binary asset URL resolves:
   - `https://github.com/ThingEngineer/SnowLeopard/releases/download/v<version>/snowleopard-v<version>.bin`
7. Download artifact and compute metadata:
   - SHA-256 using `shasum -a 256`
   - size in bytes using `wc -c`
8. Update manifest:
   - Write `sha256` and `size` in [release-data/current.json](../../../release-data/current.json)
9. Finalize commit:
   - `git add release-data/current.json`
   - `git commit -m "chore: finalize v<version> manifest checksum"`
   - Push the same branch used for release prep (`develop` in the current flow), then PR to `main`.
10. Smoke check:

- Confirm portal release page exists.
- Confirm `/settings` can check updates and sees expected latest version.

## Decision Rules

- If `actions/configure-pages` fails with `Get Pages site failed`, enable Pages once in repo settings with source set to GitHub Actions, then rerun.
- If release asset URLs return `404`, do not update checksum/size yet; wait for release workflow completion.
- If a tag already exists, do not recreate it; verify the peeled tag commit (`git rev-parse v<version>^{}`) points to the intended commit and rerun workflow if needed.
- There is no required manual step between `git push origin v<version>` and the publish phase beyond waiting for the release asset to exist; publish should perform that wait/finalize behavior.

## Completion Criteria

- Tag `v<version>` exists remotely.
- Release binary asset exists at expected URL.
- `release-data/current.json` has non-empty `sha256` and non-zero `size` matching published binary.
- Final checksum commit is pushed to the release branch and ready to merge.

## References

- [docs/RELEASE-WORKFLOW.md](../../../docs/RELEASE-WORKFLOW.md)
- [release-data/current.json](../../../release-data/current.json)
- [release-data/releases/index.json](../../../release-data/releases/index.json)
- [include/firmware_version.h](../../../include/firmware_version.h)
- [.github/workflows/firmware-release.yml](../../../.github/workflows/firmware-release.yml)
- [.github/workflows/pages.yml](../../../.github/workflows/pages.yml)
