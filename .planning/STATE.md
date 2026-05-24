---
gsd_state_version: 1.0
milestone: v1.6
milestone_name: TK-BOF
status: completed
stopped_at: Phase 32 context gathered
last_updated: "2026-05-24T10:45:46.108Z"
last_activity: 2026-05-23 -- Phase 30 marked complete
progress:
  total_phases: 5
  completed_phases: 3
  total_plans: 5
  completed_plans: 5
  percent: 100
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-05-23)

**Core value:** Operators can perform common filesystem and process-control operations directly through BOFs without dropping to cmd.exe or PowerShell — minimizing detection surface
**Current focus:** Phase 30 — core-token-bofs

## Current Position

Phase: 30 — COMPLETE
Plan: 1 of 2
Status: Phase 30 complete
Last activity: 2026-05-23 -- Phase 30 marked complete

```
Progress: Phase 0/5 complete
[                              ] 0%
```

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
- v1.5: BeaconPkgBytes/BeaconPkgInt32 (Kharon-specific) removed from ps list; replaced with standard BeaconPrintf text table output; `_include/adaptix.h` deleted — Process Browser binary format deferred to Phase 26
- v1.6: TK-BOF uses ADVAPI32$/NTDLL$ dynamic resolution (same pattern as PS-BOF); syscall-based token APIs not viable in standard BOF context
- v1.6: tk.axs registers all 6 subcommands beacon-only (same pattern as ps.axs)
- v1.6: tk steal and tk make both support --no-apply to skip immediate impersonation; handle is always printed for later use with tk use

### Blockers/Concerns

None.

## Session Continuity

Last session: 2026-05-24T10:45:46.102Z
Stopped at: Phase 32 context gathered
Resume: Run `/gsd:plan-phase 29` to plan TK-BOF Setup
