---
name: split-logical-commits
description: "Split a mixed working tree into clean, logical commits. Use when many files changed across firmware, docs, workflows, and release metadata and you want concise scoped commit messages."
argument-hint: "Optional grouping hints, for example firmware+portal+docs"
user-invocable: true
disable-model-invocation: false
---

# Split Logical Commits

Create a clean commit history from a broad set of local modifications.

## When To Use

- `git status` shows many unrelated files.
- You want each commit to represent one coherent change.
- You need reviewable, low-risk history before pushing.

## Inputs

- Optional desired commit groups (for example: firmware, portal/workflows, docs, release metadata).

## Procedure

1. Inspect working tree:
   - `git status --short`
   - `git diff --stat`
2. Propose file groups with one purpose each.
3. Validate each group diff before staging:
   - `git diff -- <files in group>`
4. Commit groups one at a time:
   - `git add <group files>`
   - `git commit -m "<concise scoped message>"`
5. After each commit:
   - verify remaining changes still make sense
6. Final verification:
   - `git log --oneline -N`
   - `git status --short`

## Commit Message Rules

- Use short prefixes like `feat:`, `fix:`, `ci:`, `docs:`, `chore:`.
- Keep message specific to the staged group.
- Avoid catch-all messages like `update files`.

## Decision Rules

- If a file mixes concerns, split by hunks with `git add -p`.
- If a generated file is required for reproducibility (for example lockfiles), keep it with the functional change that introduced it.
- Keep workflow changes separate from product behavior when possible.

## Completion Criteria

- Each commit has a single purpose.
- Working tree is clean after intended commits.
- Recent history is understandable without opening every diff.

## References

- [AGENTS.md](../../../AGENTS.md)
- [docs/RELEASE-WORKFLOW.md](../../../docs/RELEASE-WORKFLOW.md)
- [.github/workflows/pages.yml](../../../.github/workflows/pages.yml)
- [.github/workflows/firmware-release.yml](../../../.github/workflows/firmware-release.yml)
