---
phase: 26
plan: "01"
subsystem: axs-scripting
tags: [axs, ps-bof, command-registration, adaptix]
dependency_graph:
  requires: []
  provides: [PS-BOF/ps.axs, bof-collection.axs ps.axs load]
  affects: [bof-collection.axs]
tech_stack:
  added: []
  patterns: [ax.create_command, setPreHook, ax.bof_pack, ax.execute_alias, addSubCommands, ax.register_commands_group]
key_files:
  created:
    - PS-BOF/ps.axs
  modified:
    - bof-collection.axs
decisions:
  - D-01: ps.axs does NOT register on_processbrowser_list or add_session_browser — already satisfied by beacon agent ax_config.axs
  - D-03: ps list zero-arg prehook — execute bof with no bof_params interpolation
  - D-04: ps kill conditional pack — int32 or int32,int32 based on exit_code presence
  - D-05: ps run method auto-detection from token/logon flags
  - D-06: ps grep packs single int32 pid
  - D-07: ps suspend and resume pack single int32 pid
  - D-08: group registered on beacon only, not gopher/kharon
  - D-09: ax.script_load for PS-BOF/ps.axs added to bof-collection.axs
  - bof_pack uses "int32" type token per CONTEXT.md decisions (not "int" from Extension-Kit)
metrics:
  duration: "2m"
  completed_date: "2026-05-20"
  tasks_completed: 2
  files_created: 1
  files_modified: 1
---

# Phase 26 Plan 01: ps.axs and bof-collection.axs Wire-up Summary

**One-liner:** Adaptix axs script registering all 6 PS-BOF subcommands under a beacon-only `ps` parent command, wired into bof-collection.axs via ax.script_load.

## Tasks Completed

| Task | Name | Commit | Files |
|------|------|--------|-------|
| T01 | Create PS-BOF/ps.axs | 04afe1d | PS-BOF/ps.axs (created, 87 lines) |
| T02 | Add ax.script_load to bof-collection.axs | d9f20a7 | bof-collection.axs (+1 line) |

## What Was Built

`PS-BOF/ps.axs` registers 6 PS-BOF subcommands as a `ps` parent command for beacon sessions:

- **ps list** — zero-arg prehook; dispatches `execute bof "<path>/list.<arch>.o"` with no trailing args
- **ps kill** — packs `int32` (pid only) or `int32,int32` (pid + exit_code) depending on whether exit_code was provided
- **ps run** — method auto-detected from flag presence (token→2, logon credentials→1, default→0); packs 9 args: `int32,wstr,int32,int32,int32,wstr,wstr,wstr,int32` matching run.c parse order
- **ps grep** — packs `int32` pid
- **ps suspend** — packs `int32` pid
- **ps resume** — packs `int32` pid

The group is registered on `["beacon"]` only per D-08. No Process Browser wiring per D-01 (already handled by beacon agent's ax_config.axs at lines 11-16 and 107-110).

`bof-collection.axs` receives one additional line: `ax.script_load(path + "PS-BOF/ps.axs");` after the Exit-BOF load.

## PB-02 / PB-03 Requirement Satisfaction

PB-02 (Process Browser menu action) and PB-03 (on_processbrowser_list event handler) are satisfied by the beacon agent's existing `ax_config.axs` (lines 11-16 register `menu.add_session_browser`; lines 107-110 register `event.on_processbrowser_list` calling native `"ps list"`). Per D-01, ps.axs does NOT duplicate these registrations. This is an explicit scope reduction decision, not an oversight.

## Deviations from Plan

None — plan executed exactly as written.

## Known Stubs

None. ps.axs wires real BOF .o files from `PS-BOF/_bin/`. All 12 compiled targets (6 commands × 2 archs) already exist in `PS-BOF/_bin/` from previous phases.

## Threat Flags

No new security-relevant surface. ps.axs operates entirely within an established beacon session. The arg injection risk for `--command` in ps run is intentional operator capability, not a vulnerability.

## Self-Check: PASSED

Files exist:
- PS-BOF/ps.axs: FOUND
- bof-collection.axs contains `PS-BOF/ps.axs`: FOUND

Commits exist:
- 04afe1d: FOUND (feat(26-01): create PS-BOF/ps.axs)
- d9f20a7: FOUND (feat(26-01): wire PS-BOF/ps.axs into bof-collection.axs)
