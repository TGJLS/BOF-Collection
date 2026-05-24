---
phase: 31-tk-make-tk-privget
plan: "01"
subsystem: tk-bof
tags: [c, bof, windows, token, advapi32, logonuser, impersonation]

# Dependency graph
requires:
  - phase: 29-tk-bof-setup
    provides: bofdefs.h with ADVAPI32$LogonUserA, ADVAPI32$ImpersonateLoggedOnUser, NTDLL$NtClose; tkerror.h; Makefile

provides:
  - ADVAPI32$OpenThreadToken declaration in bofdefs.h (used by privget/Plan 02)
  - KERNEL32$GetCurrentThread declaration in bofdefs.h (used by privget/Plan 02)
  - KERNEL32$GetCurrentProcess declaration in bofdefs.h (used by privget/Plan 02)
  - TK-BOF/make/make.c — full LogonUserA credential token creation BOF (TK-03)

affects:
  - 31-02-privget (depends on the 3 new bofdefs.h declarations)

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "make.c follows steal.c pattern: BeaconDataParse/Extract/Int arg parsing, TkErrorMessage for Win32 errors, NtClose on all failure paths that hold a handle"
    - "Domain sentinel: empty/null domain defaults to . (local machine)"
    - "logon_type sentinel: zero defaults to 9 (LOGON32_LOGON_NEW_CREDENTIALS)"

key-files:
  created:
    - TK-BOF/make/make.c
  modified:
    - TK-BOF/bofdefs.h

key-decisions:
  - "Domain sentinel uses . (not NULL) — LogonUserA treats NULL domain as the current domain, . specifies local machine; . is the safe default matching Kharon behavior"
  - "No NtClose on LogonUserA failure path — hToken is not set on failure so there is no handle to close (D-07)"
  - "Handle printed as 0x%llx with (unsigned long long) cast — matches steal.c pattern, avoids LLP64/LP64 printf format mismatch on MinGW"

patterns-established:
  - "Token creation BOF pattern: parse args → sentinel defaults → API call → error path (no handle to close) → optional impersonation → print handle"

requirements-completed:
  - TK-03

# Metrics
duration: 15min
completed: 2026-05-24
---

# Phase 31 Plan 01: tk-make BOF Summary

**LogonUserA credential token creation BOF (make.c) with domain/logon-type sentineling, optional impersonation, and NtClose handle hygiene on all failure paths**

## Performance

- **Duration:** ~15 min
- **Started:** 2026-05-24T10:00:00Z
- **Completed:** 2026-05-24T10:15:00Z
- **Tasks:** 2
- **Files modified:** 2

## Accomplishments

- Added 3 declarations to bofdefs.h (ADVAPI32$OpenThreadToken, KERNEL32$GetCurrentThread, KERNEL32$GetCurrentProcess) — unblocks Plan 02 privget
- Implemented make.c: 5-arg parsing, domain/logon_type sentinels, LogonUserA call, ImpersonateLoggedOnUser with NtClose cleanup, handle print in both apply/no-apply paths
- All 12 TK-BOF targets (steal, use, make, rm, revert, privget x64+x32) compile cleanly with no regressions

## Task Commits

Each task was committed atomically:

1. **Task 1: Add 3 missing declarations to bofdefs.h** - `ec89d76` (feat)
2. **Task 2: Implement TK-BOF/make/make.c** - `99f05cc` (feat)

**Plan metadata:** (docs commit follows)

## Files Created/Modified

- `TK-BOF/bofdefs.h` - Added ADVAPI32$OpenThreadToken, KERNEL32$GetCurrentThread, KERNEL32$GetCurrentProcess declarations
- `TK-BOF/make/make.c` - Full implementation: LogonUserA credential token creation BOF per TK-03

## Decisions Made

- Domain sentinel uses `.` (not NULL): LogonUserA with NULL domain uses current domain; `.` specifies local machine and is the safe default matching Kharon behavior.
- No NtClose on LogonUserA failure: hToken is only set on success; the failure path has no handle to close.
- Handle printed as `0x%llx` with `(unsigned long long)` cast: matches steal.c and avoids LLP64/LP64 printf format mismatch on MinGW.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

One cwd drift incident (#3097): initial edits landed in the main repo instead of the worktree because absolute paths derived from environment context resolved to `/home/tgj/github/BOF-Collection/` rather than the worktree root. The accidental commit to main was reversed with `git reset --hard HEAD~1`. All subsequent edits used the correct worktree path `/home/tgj/github/BOF-Collection/.claude/worktrees/agent-a555ee2c7ba3839f8/`.

## Next Phase Readiness

- bofdefs.h now has all 3 declarations required by Plan 02 (privget)
- Plan 02 can proceed immediately

## Self-Check: PASSED

- FOUND: TK-BOF/bofdefs.h (3 new declarations verified)
- FOUND: TK-BOF/make/make.c (all acceptance criteria verified)
- FOUND: ec89d76 (Task 1 commit)
- FOUND: 99f05cc (Task 2 commit)
- Build: all 12 targets [+] (steal, use, make, rm, revert, privget x64+x32)

---
*Phase: 31-tk-make-tk-privget*
*Completed: 2026-05-24*
