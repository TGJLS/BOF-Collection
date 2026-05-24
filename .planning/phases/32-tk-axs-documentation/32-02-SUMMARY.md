---
phase: 32-tk-axs-documentation
plan: 02
subsystem: config/script
tags: [adaptix, axs, tk-bof, token-management, beacon-object-file]

requires:
  - phase: 32-01
    provides: origin/main merge (PS-BOF line in bof-collection.axs), LogonUserW conversion in make.c

provides:
  - TK-BOF/tk.axs: Adaptix command registrations for all 6 tk subcommands (steal, use, make, rm, revert, privget) under parent `tk` command, beacon-only
  - bof-collection.axs: now loads TK-BOF/tk.axs alongside FS-BOF, Exit-BOF, PS-BOF

affects: [33-tk-ci-testing]

tech-stack:
  added: []
  patterns:
    - "tk.axs: nested subcommand pattern matching exit.axs; addArgBool key includes -- prefix in parsed_json"
    - "bof_pack: wstr,wstr,wstr,int,int for make (5-arg multi-type pack)"
    - "beacon-only registration: [\"beacon\"], [\"windows\"], []"

key-files:
  created:
    - TK-BOF/tk.axs
  modified:
    - bof-collection.axs

key-decisions:
  - "beacon-only registration per D-02: [\"beacon\"], [\"windows\"], [] — not gopher or kharon"
  - "addArgBool key naming: parsed_json[\"--no-apply\"] includes dashes (Pitfall 1 from RESEARCH.md)"
  - "bof_pack format int not int32/int64: Adaptix axs rejects int32/int64 per D-03"

requirements-completed: [TK-08]

duration: 15min
completed: 2026-05-24
---

# Phase 32 Plan 02: tk.axs + bof-collection.axs wiring Summary

**tk.axs registering all 6 TK-BOF subcommands beacon-only with correct bof_pack format strings matching each C file's BeaconDataParse arg order**

## Performance

- **Duration:** ~15 min
- **Started:** 2026-05-24T20:00:00Z
- **Completed:** 2026-05-24T20:15:00Z
- **Tasks:** 2
- **Files modified:** 2

## Accomplishments

- Created TK-BOF/tk.axs (68 lines) registering steal, use, make, rm, revert, privget under parent `tk` command
- bof-collection.axs now loads all 4 category scripts in order: FS-BOF, Exit-BOF, PS-BOF, TK-BOF
- Closes TK-08: operators can type `tk steal <pid>`, `tk make --username u --password p`, etc. in Adaptix

## Task Commits

Each task was committed atomically:

1. **Task 1: Create TK-BOF/tk.axs with all 6 subcommands and group registration** - `faba094` (feat)
2. **Task 2: Add TK-BOF script_load line to root bof-collection.axs** - `ffc58ce` (feat)

**Plan metadata:** (see below)

## Files Created/Modified

- `TK-BOF/tk.axs` - Adaptix axs script: 6 subcommand definitions, parent `tk` command, group registration beacon-only. steal packs int,int; use/rm pack int; make packs wstr,wstr,wstr,int,int; revert/privget pack no args.
- `bof-collection.axs` - Added `ax.script_load(path + "TK-BOF/tk.axs");` after PS-BOF line; now 4 script_load calls

## Decisions Made

None beyond plan specification - all decisions (D-02, D-03, D-04, Pitfall 1) were pre-resolved in CONTEXT.md and RESEARCH.md before execution.

## Deviations from Plan

None - plan executed exactly as written.

The automated verify in the plan uses `grep -cF 'ax.create_command'` (without parenthesis) which matches 8 occurrences because `ax.create_commands_group` contains the substring. The actual acceptance criterion ("exactly 7 `ax.create_command(` calls") is met: `grep -cF 'ax.create_command(' TK-BOF/tk.axs` returns 7. This is a minor plan verify script issue, not a code issue.

## Issues Encountered

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- TK-BOF tk commands are fully wired in Adaptix: operators can load bof-collection.axs and use all tk subcommands against a beacon on a Windows target
- Phase 33 (CI/CD) can now add tasks.yaml test entries and test.yaml deploy block for TK-BOF - the axs wiring is in place

---

## Self-Check

### Files exist

- `TK-BOF/tk.axs` - FOUND
- `bof-collection.axs` (modified) - FOUND

### Commits exist

- `faba094` - feat(32-02): create TK-BOF/tk.axs registering all 6 subcommands beacon-only
- `ffc58ce` - feat(32-02): add TK-BOF/tk.axs script_load to bof-collection.axs

### Key assertions

- `grep -F 'name: "TK-BOF"' TK-BOF/tk.axs` - PASS
- `grep -cF 'ax.create_command(' TK-BOF/tk.axs` = 7 - PASS
- `ax.register_commands_group(group_tk, ["beacon"], ["windows"], [])` - PASS
- No `"gopher"` or `"kharon"` in tk.axs - PASS
- `ax.bof_pack("int,int", [pid, no_apply])` - PASS
- `ax.bof_pack("wstr,wstr,wstr,int,int", [username, password, domain, no_apply, logon_type])` - PASS
- 2 occurrences of `ax.bof_pack("int", [token_handle])` - PASS
- `parsed_json["--no-apply"]` (key includes dashes) - PASS
- All 6 _bin/ paths present - PASS
- No `int32` or `int64` - PASS
- bof-collection.axs: exactly 4 script_load calls in FS/Exit/PS/TK order - PASS

## Self-Check: PASSED

*Phase: 32-tk-axs-documentation*
*Completed: 2026-05-24*
