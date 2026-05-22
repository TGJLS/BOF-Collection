---
phase: 28-ci-cd-tests
plan: "02"
subsystem: testing
tags: [ci-cd, github-actions, tasks-yaml, adaptix-testing, ps-bof]

# Dependency graph
requires:
  - phase: 28-01-testing-kit-capture
    provides: capture field + variable substitution feature in adaptix-testing runner
  - phase: 26-ps-axs-process-browser
    provides: ps.axs command wiring and bof-collection.axs with ps.axs load
provides:
  - 8 PS-BOF task entries in tasks.yaml covering CI-01 through CI-06
  - PS-BOF deploy block in test.yaml (mkdir, copy _bin/*.o, ps.axs, bof-collection.axs)
  - PS-BOF x64 object count verification (workspace and container, expect 6)
  - adaptix-testing reinstall from git before integration tests
affects: [ci-cd, ps-bof, testing]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "PS-BOF section in tasks.yaml following FS-BOF comment-header convention (em-dash separators)"
    - "PID capture: spawn notepad.exe, capture PID, chain to grep/suspend/resume/kill sequence"
    - "expected_regex with (?s) flag for multi-section output assertion (ps grep)"
    - "not_expected: error pattern for suspend/resume/kill success verification"
    - "uv tool install --reinstall before integration tests to pick up new features without container rebuild"

key-files:
  created: []
  modified:
    - .github/ci/tasks.yaml
    - .github/workflows/test.yaml

key-decisions:
  - "ps-grep uses expected_regex with escaped brackets (\\[Token\\]) and (?s) flag to verify all four section headers appear in order in a single assertion"
  - "uv reinstall placed after build verification echo and before server startup, per D-04 (just make it work approach)"
  - "bof-collection.axs copied to container because container image pre-dates Phase 26 ps.axs load addition"

patterns-established:
  - "PID capture chain: ps-run captures PID, ps-grep/suspend/resume/kill use {{pid}} substitution"
  - "PS-BOF deploy mirrors FS-BOF pattern: mkdir -p target dir, cp _bin/*.o, cp .axs, cp root axs"

requirements-completed: [CI-01, CI-02, CI-03, CI-04, CI-05, CI-06, CI-07]

# Metrics
duration: 10min
completed: 2026-05-22
---

# Phase 28 Plan 02: BOF-Collection CI Integration Summary

**8 PS-BOF test entries added to tasks.yaml with PID capture chain, plus PS-BOF deploy block and adaptix-testing reinstall in test.yaml covering CI-01 through CI-07**

## Performance

- **Duration:** ~10 min
- **Started:** 2026-05-22T11:20:00Z
- **Completed:** 2026-05-22T11:30:37Z
- **Tasks:** 2
- **Files modified:** 2

## Accomplishments

- Added 8 PS-BOF task entries to `.github/ci/tasks.yaml` under a `# ── PS-BOF ──` header: ps-list x2 (CI-01), ps-run-spawn-notepad with PID capture fixture (CI-02/04/05/06), ps-run-pipe-whoami (CI-03), ps-grep with four-section regex (CI-04), ps-suspend/resume/kill with not_expected error (CI-05, CI-06, CI-02)
- Added PS-BOF copy block to `.github/workflows/test.yaml` Docker bash block: mkdir -p container path, cp _bin/*.o, ps.axs, and updated bof-collection.axs
- Added PS-BOF x64 object count verification (6 objects, checked in workspace and container)
- Added `uv tool install --reinstall` from git before integration tests to pick up capture feature without container rebuild

## Task Commits

1. **Task 1: Add PS-BOF task entries to tasks.yaml** - `ae7c1a2` (feat)
2. **Task 2: Add PS-BOF deployment block to test.yaml** - `42617db` (feat)

## Files Created/Modified

- `.github/ci/tasks.yaml` - appended PS-BOF section with 8 task entries covering all CI requirements
- `.github/workflows/test.yaml` - added PS-BOF deploy block (4 bash lines), PS-BOF x64 count checks (4 bash lines), and uv reinstall (1 bash line)

## Decisions Made

- ps-grep uses `expected_regex: "(?s)\\[Token\\].*\\[Modules\\].*\\[Cmdline\\].*\\[Threads\\]"` — single assertion verifying all four section headers appear in order, per PLAN.md Task 1 entry 5 (overrides RESEARCH Pattern 2 which showed only `expected: "[Token]"`)
- bof-collection.axs copied from workspace to container because the container image pre-dates Phase 26's `ax.script_load(path + "PS-BOF/ps.axs")` addition (D-08)
- uv reinstall placed between build verification echo and server startup — after build checks complete, before server is running (D-04)

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- PS-BOF CI integration complete — tasks.yaml and test.yaml updated
- Requires Testing-Kit capture feature (plan 28-01) to be deployed to the container via the uv reinstall step
- All CI-01 through CI-07 requirements addressed; ready for end-to-end CI validation run

---
*Phase: 28-ci-cd-tests*
*Completed: 2026-05-22*

## Self-Check: PASSED

- `.github/ci/tasks.yaml` — FOUND (modified, 8 PS-BOF entries appended)
- `.github/workflows/test.yaml` — FOUND (modified, deploy block + reinstall added)
- Commit `ae7c1a2` — FOUND (Task 1: tasks.yaml)
- Commit `42617db` — FOUND (Task 2: test.yaml)
