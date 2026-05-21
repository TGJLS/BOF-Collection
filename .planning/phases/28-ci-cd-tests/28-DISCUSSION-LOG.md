# Phase 28: CI/CD Tests - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-05-21
**Phase:** 28-ci-cd-tests
**Areas discussed:** PID capture strategy, Testing-Kit extension design, test sequencing, CI workflow deployment, plan structure

---

## PID Capture Strategy

| Option | Description | Selected |
|--------|-------------|----------|
| System PID + preamble notepad | Use PID 4 for ps grep; preamble launches notepad; allowed_to_fail for kill | |
| Spawn via ps run + independent tests with allowed_to_fail | Test each command separately; kill uses dummy PID | |
| Extend Testing-Kit with output capture | Add capture/substitution feature to Testing-Kit to enable PID chaining | ✓ |

**User's choice:** Extend Testing-Kit — "Could we maybe add functionality to the Testing-Kit to make this possible? it is at ~/github/Testing-Kit"
**Notes:** User is the Testing-Kit author; adding capture support is feasible and preferable to workarounds.

---

## Testing-Kit Capture Syntax

| Option | Description | Selected |
|--------|-------------|----------|
| `capture: {pid: "regex"}` + `{{pid}}` | Named dict, supports multiple captures per task | ✓ (implied) |
| `capture_regex` + `capture_var` flat fields | Single capture per task, simpler schema | |

**User's choice:** Dict syntax (supports multiple captures per task). Also requested README update for Testing-Kit.
**Notes:** User clarified "I obviously meant updating the README in Testing-Kit" — not BOF-Collection README.

---

## Test Sequencing

| Option | Description | Selected |
|--------|-------------|----------|
| Spawn once, reuse PID for all 5 dependent tests | ps run → capture pid → ps grep → ps suspend → ps resume → ps kill | ✓ |
| Spawn separate processes per test group | One spawn per test, more isolated | |

**User's choice:** Spawn once, reuse PID across all dependent tests.
**Notes:** Order: ps grep → ps suspend → ps resume → ps kill. ps run --pipe whoami tested separately.

---

## CI Workflow Deployment

| Option | Description | Selected |
|--------|-------------|----------|
| Copy PS-BOF _bin/*.o + ps.axs + workspace bof-collection.axs | Mirror FS-BOF pattern; add mkdir -p; overwrite stale bof-collection.axs | ✓ |
| Copy entire PS-BOF directory from workspace | Simpler rsync/cp -r; auto-catches missing subdirs | |

**User's choice:** Granular copy — objects, ps.axs, and bof-collection.axs.
**Notes:** User explicitly said "I completely need to rework this docker approach — for now let's just get it to work. I'll clean up later."

---

## Plan Structure

| Option | Description | Selected |
|--------|-------------|----------|
| Two plans (28-01 Testing-Kit, 28-02 BOF-Collection CI) | Clean separation; different repos; Wave 1 parallel | ✓ |
| One combined plan | Simpler tracking | |

**User's choice:** Two plans in Wave 1 (parallel).

---

## Claude's Discretion

- tasks.yaml section comment header (`# ── PS-BOF ──`)
- ps run spawn task assertion (`expected_regex: "Process started: PID \\d+"`)
- ps suspend/resume/kill assertion style (no expected vs. `not_expected: "error"`)
- ps grep output assertions (which section labels to check)
- uv reinstall placement in test.yaml bash block

## Deferred Ideas

- Docker CI approach rework — user acknowledged it needs a full cleanup; deferred to a future phase.
