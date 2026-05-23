---
phase: 30-core-token-bofs
plan: 02
subsystem: bof
tags: [bof, token, windows, advapi32, ntdll, impersonation, ntstatus]

# Dependency graph
requires:
  - phase: 30-01
    provides: TK-BOF/tkerror.h (TkErrorMessage helper), use/rm/revert stubs with arg-parsing blocks
provides:
  - TK-BOF/use/use.c — full use body: ImpersonateLoggedOnUser + TkErrorMessage error path
  - TK-BOF/rm/rm.c — full rm body: NtClose + raw NTSTATUS hex error path
  - TK-BOF/revert/revert.c — full revert body: RevertToSelf success-only
affects: [30-03-make-privget, downstream TK-BOF plans]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "use.c: ADVAPI32$ImpersonateLoggedOnUser + TkErrorMessage via KERNEL32$GetLastError — Win32 error path"
    - "rm.c: NTDLL$NtClose + raw NTSTATUS hex via (ULONG) status cast and 0x%lx — no FormatMessage"
    - "revert.c: ADVAPI32$RevertToSelf() return discarded — no error branch per D-07 and CLAUDE.md simplicity"
    - "Handle printed as (ULONG_PTR) token_handle with 0x%lx — consistent across use/rm/steal"

key-files:
  created: []
  modified:
    - TK-BOF/use/use.c
    - TK-BOF/rm/rm.c
    - TK-BOF/revert/revert.c

key-decisions:
  - "rm.c uses status < 0 literal check (not NT_SUCCESS macro) — macro may not be available; matches PATTERNS.md exactly"
  - "rm.c does NOT include tkerror.h — NtClose returns NTSTATUS, not Win32 error; FormatMessage yields misleading text for NTSTATUS values"
  - "revert.c discards RevertToSelf return value — D-07 specifies success output only; no error branch per CLAUDE.md simplicity rule"
  - "use.c does NOT close token_handle on failure or success — operator owns handle lifecycle (rm/revert are separate)"

requirements-completed: [TK-02, TK-04, TK-05]

# Metrics
duration: 10min
completed: 2026-05-23
---

# Phase 30 Plan 02: TK-BOF use/rm/revert Summary

**ImpersonateLoggedOnUser by handle (use.c), NtClose with raw NTSTATUS hex (rm.c), and RevertToSelf success-only (revert.c) — three single-API-call BOF bodies completing TK-02, TK-04, TK-05**

## Performance

- **Duration:** ~10 min
- **Started:** 2026-05-23
- **Completed:** 2026-05-23
- **Tasks:** 3
- **Files modified:** 3

## Accomplishments

- Implemented `TK-BOF/use/use.c` body: `ADVAPI32$ImpersonateLoggedOnUser(token_handle)` with `TkErrorMessage` error path and `"[+] Impersonating handle 0x%lx"` success output (D-03/D-04); builds use x64 + use x32
- Implemented `TK-BOF/rm/rm.c` body: `NTDLL$NtClose(token_handle)` with `status < 0` check and raw `"[-] rm: NtClose failed: 0x%lx"` NTSTATUS hex error output, `"[+] Handle 0x%lx closed."` success (D-06); builds rm x64 + rm x32
- Implemented `TK-BOF/revert/revert.c` body: `ADVAPI32$RevertToSelf()` discard-return, emit `"[+] Reverted to process token."` (D-07); no error branch, no BeaconDataParse; builds revert x64 + revert x32

## Build Results

```
[+] use x64
[+] use x32
[+] rm x64
[+] rm x32
[+] revert x64
[+] revert x32
```

Zero `[!]` lines for all three. `make -C TK-BOF bof` exits 0 for the full category.

## Task Commits

Each task committed atomically:

1. **Task 1: Implement TK-BOF/use/use.c** - `1a733df` (feat)
2. **Task 2: Implement TK-BOF/rm/rm.c** - `1888e18` (feat)
3. **Task 3: Implement TK-BOF/revert/revert.c** - `a895e65` (feat)

## Files Created/Modified

- `TK-BOF/use/use.c` — Added `#include "../tkerror.h"`; ImpersonateLoggedOnUser call; failure path via TkErrorMessage to CALLBACK_ERROR; success path to CALLBACK_OUTPUT; arg-parsing block unchanged
- `TK-BOF/rm/rm.c` — `NTSTATUS status = NTDLL$NtClose(token_handle)`; `if (status < 0)` failure with `(ULONG) status` raw hex; success with `(ULONG_PTR) token_handle`; no tkerror.h; arg-parsing block unchanged
- `TK-BOF/revert/revert.c` — `ADVAPI32$RevertToSelf()` discard; `"[+] Reverted to process token.\n"` via CALLBACK_OUTPUT; no BeaconDataParse; no tkerror.h

## Verification Results

- `grep -c 'BeaconPrintf' TK-BOF/use/use.c` == 2 (error + success)
- `grep -c 'BeaconPrintf' TK-BOF/rm/rm.c` == 2 (error + success)
- `grep -c 'BeaconPrintf' TK-BOF/revert/revert.c` == 1 (success only)
- `grep -L 'tkerror' TK-BOF/rm/rm.c TK-BOF/revert/revert.c` lists both
- `grep -l 'tkerror' TK-BOF/use/use.c` returns use.c

## Decisions Made

- `rm.c` uses `status < 0` literal (not `!NT_SUCCESS(status)`) — NT_SUCCESS macro may not be available in this header set; literal check is unambiguous and matches PATTERNS.md
- `rm.c` excludes `tkerror.h` — `NtClose` returns NTSTATUS, not Win32 error code; `FormatMessage` would yield "The operation completed successfully" for many valid NTSTATUS failure codes (e.g., `STATUS_INVALID_HANDLE`)
- `revert.c` discards `RevertToSelf` return value — D-07 specifies success-only output; adding an error branch would add code that cannot fail in practice (RevertToSelf does not fail if the thread was impersonating); CLAUDE.md simplicity rule applies

## Deviations from Plan

None - plan executed exactly as written.

## Known Stubs

None. All three files have real implementations wired to their API calls.

## Threat Flags

None. No new network endpoints, auth paths, file access patterns, or schema changes introduced beyond what the plan's threat model covers (T-30-05 through T-30-09 all addressed by the implementations as specified).

## Self-Check: PASSED

- `TK-BOF/use/use.c` — present and contains ImpersonateLoggedOnUser call
- `TK-BOF/rm/rm.c` — present and contains NtClose call
- `TK-BOF/revert/revert.c` — present and contains RevertToSelf call
- Commits 1a733df, 1888e18, a895e65 — all in git log

---
*Phase: 30-core-token-bofs*
*Completed: 2026-05-23*
