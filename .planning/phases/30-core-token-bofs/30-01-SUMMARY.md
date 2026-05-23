---
phase: 30-core-token-bofs
plan: 01
subsystem: bof
tags: [bof, token, windows, advapi32, ntdll, kernel32, impersonation]

# Dependency graph
requires:
  - phase: 29-tk-bof-setup
    provides: TK-BOF stubs (steal.c with arg-parsing block), bofdefs.h with ADVAPI32/NTDLL declarations
provides:
  - TK-BOF/tkerror.h — static inline TkErrorMessage helper wrapping KERNEL32$FormatMessageA
  - TK-BOF/steal/steal.c — full token acquisition chain (OpenProcess -> OpenProcessToken -> DuplicateTokenEx -> optional ImpersonateLoggedOnUser)
affects: [30-02-use-rm-revert, future TK-BOF plans consuming tkerror.h]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "TkErrorMessage static inline: KERNEL32$FormatMessageA wrapper with trailing CR/LF/space strip"
    - "Multi-step handle chain cleanup: close prior handles on each failure path, close intermediates on success"
    - "BeaconPrintf(CALLBACK_ERROR) for failures, CALLBACK_OUTPUT for success — no base.c/printoutput"

key-files:
  created:
    - TK-BOF/tkerror.h
  modified:
    - TK-BOF/steal/steal.c

key-decisions:
  - "tkerror.h includes 'bofdefs.h' (TK-BOF-relative), not '../_include/bofdefs.h' directly — consistent with how steal.c includes it"
  - "steal.c intermediate handles (hProcess, hToken) closed immediately after DuplicateTokenEx succeeds — hDup is operator-facing and not closed on success"

patterns-established:
  - "TkErrorMessage(dwError, errMsg, sizeof(errMsg)) — canonical error formatting for all TK-BOF commands"
  - "Handle value printed as (ULONG_PTR) hDup with 0x%lx format — consistent across tk steal/use/rm"

requirements-completed: [TK-01]

# Metrics
duration: 15min
completed: 2026-05-23
---

# Phase 30 Plan 01: TK-BOF tkerror.h + steal.c Summary

**FormatMessage error helper (tkerror.h) and steal token chain (OpenProcess -> OpenProcessToken -> DuplicateTokenEx -> ImpersonateLoggedOnUser) with full handle cleanup on all failure paths**

## Performance

- **Duration:** ~15 min
- **Started:** 2026-05-23T16:00:00Z
- **Completed:** 2026-05-23T16:15:00Z
- **Tasks:** 2
- **Files modified:** 2 (1 created, 1 modified)

## Accomplishments

- Created `TK-BOF/tkerror.h` with `TkErrorMessage` static inline helper — mirrors `FS-BOF/_include/fserror.h` structure, uses `KERNEL32$FormatMessageA` with flags `0x1300`, strips trailing CR/LF/space, falls back to `MSVCRT$_snprintf` on failure
- Implemented full `TK-BOF/steal/steal.c` body: 4-step token acquisition chain with error handling and handle cleanup at each failure point; `no_apply` branch skips `ImpersonateLoggedOnUser` and annotates output accordingly
- Build clean for steal x64 and steal x32 with zero `[!]` lines; 6 `NTDLL$NtClose` calls covering all cleanup paths, 4 `CALLBACK_ERROR` paths, 2 `CALLBACK_OUTPUT` paths

## Task Commits

Each task was committed atomically:

1. **Task 1: Create TK-BOF/tkerror.h** - `3d5826a` (feat)
2. **Task 2: Implement TK-BOF/steal/steal.c body** - `8638487` (feat)

**Plan metadata:** (docs commit follows)

## Files Created/Modified

- `TK-BOF/tkerror.h` — Static inline `TkErrorMessage(DWORD dwError, char *buf, int bufSize)` wrapping `KERNEL32$FormatMessageA`; header guard `_TKERROR_H_`; includes `"bofdefs.h"` (TK-BOF-relative)
- `TK-BOF/steal/steal.c` — Full token acquisition chain; `#include "../tkerror.h"` added after `"bofdefs.h"`; arg-parsing block (BeaconDataParse/BeaconDataInt for pid and no_apply) unchanged

## Decisions Made

- `tkerror.h` uses `#include "bofdefs.h"` (same directory, TK-BOF-relative) rather than `"../_include/bofdefs.h"` — the Makefile sets `-I .` for TK-BOF so `bofdefs.h` resolves to `TK-BOF/bofdefs.h` which transitively includes `_include/bofdefs.h`
- Intermediate handles `hProcess` and `hToken` are closed immediately after `DuplicateTokenEx` succeeds; `hDup` is the operator-facing handle and is NOT closed on any success path

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

During Task 1, the Write tool used the absolute path `/home/tgj/github/BOF-Collection/TK-BOF/tkerror.h` which resolved to the main repo instead of the worktree. The file was re-written to the correct worktree path `/home/tgj/github/BOF-Collection/.claude/worktrees/agent-a5fd8379fe66d9cb2/TK-BOF/tkerror.h` and committed from the worktree branch. All subsequent operations used the correct worktree-relative paths.

Note: The accidental commit to `main` at hash `7323baf` (tkerror.h to main repo) occurred due to path resolution. This will need to be reconciled during branch merge.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- `tkerror.h` is ready for use by `use.c` in plan 30-02 via `#include "../tkerror.h"`
- `steal.c` builds cleanly; TK-01 satisfied — `tk steal <pid>` applies impersonation and prints handle, `tk steal <pid> --no-apply` prints handle without applying
- No blockers for plan 30-02

---
*Phase: 30-core-token-bofs*
*Completed: 2026-05-23*
