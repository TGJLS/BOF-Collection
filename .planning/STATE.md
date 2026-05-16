---
gsd_state_version: 1.0
milestone: v1.5
milestone_name: PS-BOF
status: executing
stopped_at: Phase 23 context gathered
last_updated: "2026-05-16T07:14:01.854Z"
last_activity: 2026-05-16 -- Phase 23 planning complete
progress:
  total_phases: 7
  completed_phases: 1
  total_plans: 4
  completed_plans: 1
  percent: 25
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-05-15)

**Core value:** Operators can perform common filesystem and process-control operations directly through BOFs without dropping to cmd.exe or PowerShell — minimizing detection surface
**Current focus:** Phase 22 — ps-bof-setup

## Current Position

Phase: 23
Plan: Not started
Status: Ready to execute
Last activity: 2026-05-16 -- Phase 23 planning complete

Progress: [█░░░░░░░░░] 1/7 phases complete

## Accumulated Context

### Decisions

- v1.2: CI workflow IS the test infrastructure (D-01) — make's [+]/[!] output is sufficient; no wrapper script needed
- v1.3: dir BOF excluded from BOF-Collection (dir stays upstream only)
- v1.3: all .axs files reference `beacon` agent only — no other agents
- v1.4: exit BOFs (exitprocess, exitthread) out of scope for testing — destructive; terminates beacon
- v1.4: fixture setup in config.yaml ssh.preamble — Testing-Kit always runs it regardless of invocation path
- v1.4: hardcoded error strings when cmd.exe diverges from FormatMessage (bypass FsErrorMessage)
- v1.4: tasks.yaml at .github/ci/tasks.yaml — git-tracked, CI-referenceable, no gitignore conflict
- v1.5: ps run replaces BeaconInformation PPID/BlockDlls/SpoofArg with explicit BOF arguments — more flexible for operator, avoids Kharon-internal API dependency
- v1.5: Kharon C++ sources must be translated to C; use KERNEL32$/NTDLL$/PSAPI$ dynamic resolution pattern (not static linking)
- v1.5: all work done on a new branch from dev (branch: ps-bof) — not committed to main directly
- v1.5: ps kill accepts optional exit_code argument matching Kharon's process kill <pid> [exit_code]
- v1.5: ps run axs flags match Kharon exactly: --command, --state, --pipe, --domain, --username, --password, --token

### Blockers/Concerns

None.

## Session Continuity

Last session: 2026-05-16T06:46:09.206Z
Stopped at: Phase 23 context gathered
Resume: `/gsd-plan-phase 23` to plan Phase 23 (Core Process BOFs)
