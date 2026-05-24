---
phase: 31-tk-make-tk-privget
plan: "02"
subsystem: tk-bof
tags: [c, bof, windows, token, advapi32, privileges, impersonation]

# Dependency graph
requires:
  - phase: 31-tk-make-tk-privget
    plan: "01"
    provides: ADVAPI32$OpenThreadToken, KERNEL32$GetCurrentThread, KERNEL32$GetCurrentProcess in bofdefs.h

provides:
  - TK-BOF/privget/privget.c — full AdjustTokenPrivileges privilege-enable BOF (TK-06)

affects: []

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Two-pass GetTokenInformation: pass NULL/0 to get required size, then allocate and fill"
    - "OpenThreadToken silent fallback: thread token failure expected when not impersonating; fall through to OpenProcessToken without printing error"
    - "AdjustTokenPrivileges success check via GetLastError, not return value: function returns TRUE even on partial success (ERROR_NOT_ALL_ASSIGNED=1300)"
    - "Free heap and close handle before error check on AdjustTokenPrivileges path — avoids leaks on all branches"

key-files:
  created: []
  modified:
    - TK-BOF/privget/privget.c

key-decisions:
  - "TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES used in both OpenThreadToken and OpenProcessToken calls — TOKEN_ALL_ACCESS avoided per D-09"
  - "dwError captured immediately after AdjustTokenPrivileges before HeapFree/NtClose — GetLastError is valid only before any intervening API call"
  - "privCount captured before HeapFree so count survives deallocation for the output print"
  - "ERROR_NOT_ALL_ASSIGNED (1300) treated as warning (CALLBACK_OUTPUT [!]) not error; [+] count line still printed on partial-success path"

requirements-completed:
  - TK-06

# Metrics
duration: 10min
completed: 2026-05-24
---

# Phase 31 Plan 02: tk-privget BOF Summary

**AdjustTokenPrivileges privilege-enable BOF with OpenThreadToken/OpenProcessToken fallback, two-pass GetTokenInformation heap pattern, and ERROR_NOT_ALL_ASSIGNED warning path**

## Performance

- **Duration:** ~10 min
- **Started:** 2026-05-24
- **Completed:** 2026-05-24
- **Tasks:** 1
- **Files modified:** 1

## Accomplishments

- Replaced the 8-line stub in privget.c with a full implementation
- OpenThreadToken → silent fallback to OpenProcessToken when not impersonating
- Two-pass GetTokenInformation (NULL/0 size query, HeapAlloc, fill pass) with NULL-check and error path
- SE_PRIVILEGE_ENABLED loop over all privileges before AdjustTokenPrivileges
- AdjustTokenPrivileges result checked via GetLastError (not return value): hard error, ERROR_NOT_ALL_ASSIGNED warning, and full-success paths all handled
- HeapFree + NtClose on every exit path (cleanup before error check on the adjust path)
- All 12 TK-BOF targets compile cleanly with no regressions

## Task Commits

1. **Task 1: Implement TK-BOF/privget/privget.c** - `bf8e231` (feat)

## Files Created/Modified

- `TK-BOF/privget/privget.c` - Full implementation: OpenThreadToken/OpenProcessToken fallback, two-pass GetTokenInformation, enable-all loop, AdjustTokenPrivileges with partial-success warning

## Decisions Made

- TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES in both token-open calls (not TOKEN_ALL_ACCESS).
- GetLastError captured immediately after AdjustTokenPrivileges, before any cleanup call that might clobber it.
- privCount saved before HeapFree so the privilege count survives into the output line.
- ERROR_NOT_ALL_ASSIGNED (1300) is a warning, not a failure: both the `[!]` line and the `[+]` count line print on that path.

## Deviations from Plan

None - plan executed exactly as written.

## Known Stubs

None.

## Threat Flags

None — implementation touches no new network endpoints, auth paths, file access patterns, or schema changes. T-31-04 (HeapAlloc NULL-check) and T-31-05 (handle leak on all error paths) mitigations are both present.

## Self-Check: PASSED

- FOUND: TK-BOF/privget/privget.c (full implementation, all acceptance criteria verified)
- FOUND: bf8e231 (Task 1 commit)
- Build: all 12 targets [+] (steal, use, make, rm, revert, privget x64+x32), zero [!] lines
