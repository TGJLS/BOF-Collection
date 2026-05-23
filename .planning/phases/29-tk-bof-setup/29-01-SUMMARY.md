---
phase: 29-tk-bof-setup
plan: "01"
subsystem: TK-BOF build skeleton
tags:
  - bof
  - build-system
  - mingw
  - token-api
  - skeleton
dependency_graph:
  requires:
    - FS-BOF/Makefile (pattern reference)
    - _include/bofdefs.h (root declarations — additive edit)
    - _include/beacon.h (datap, BeaconDataParse, BeaconDataInt, BeaconDataExtract)
  provides:
    - TK-BOF/bofdefs.h (ADVAPI32$/NTDLL$ token API declarations)
    - TK-BOF/Makefile (12-target cross-compile build)
    - TK-BOF/steal/steal.c, TK-BOF/use/use.c, TK-BOF/make/make.c (arg-parsed stubs)
    - TK-BOF/rm/rm.c, TK-BOF/revert/revert.c, TK-BOF/privget/privget.c (stubs)
    - KERNEL32$OpenProcess in root _include/bofdefs.h
  affects:
    - Makefile (root SUBDIRS now includes TK-BOF)
    - Phases 30-31 (will fill stub bodies with real token logic)
tech_stack:
  added:
    - TK-BOF category (new BOF subdirectory following FS-BOF pattern)
  patterns:
    - ADVAPI32$/NTDLL$ dynamic resolution (same macro convention as root bofdefs.h)
    - FS-BOF Makefile pattern (CC64/CC86/STRIP64/STRIP86, bof: clean, [+]/[!] output)
    - BeaconDataParse + BeaconDataInt/BeaconDataExtract arg scaffolding
key_files:
  created:
    - TK-BOF/bofdefs.h
    - TK-BOF/Makefile
    - TK-BOF/steal/steal.c
    - TK-BOF/use/use.c
    - TK-BOF/make/make.c
    - TK-BOF/rm/rm.c
    - TK-BOF/revert/revert.c
    - TK-BOF/privget/privget.c
  modified:
    - _include/bofdefs.h (added KERNEL32$OpenProcess)
    - Makefile (added TK-BOF to SUBDIRS)
decisions:
  - "D-01/D-02: TK-BOF/bofdefs.h declares only the 8 token-specific APIs (7 ADVAPI32 + 1 NTDLL); root declarations not reduplicated"
  - "D-03: KERNEL32$OpenProcess added to root _include/bofdefs.h in new process-management section band"
  - "D-04/D-05: Stubs parse args via BeaconDataParse scaffolding but return silently; no Win32 calls, no BeaconPrintf"
  - "D-06: TK-BOF added to root Makefile SUBDIRS (after FS-BOF and Exit-BOF)"
  - "D-07: TK-BOF Makefile CFLAGS uses -I . not -I _include (bofdefs.h at category root, no _include/ subdirectory)"
metrics:
  duration: "~10 minutes"
  completed: "2026-05-23"
  tasks_completed: 3
  tasks_total: 3
  files_created: 8
  files_modified: 2
---

# Phase 29 Plan 01: TK-BOF Build Skeleton Summary

**One-liner:** TK-BOF build skeleton with 8 ADVAPI32$/NTDLL$ token API declarations, 6 silent arg-parsed stubs, and 12-target MinGW cross-compile Makefile producing zero [!] failures.

## What Was Built

Created the complete TK-BOF build skeleton that Phases 30-31 will fill with token manipulation logic:

- `TK-BOF/bofdefs.h`: 7 ADVAPI32$ declarations (OpenProcessToken, DuplicateTokenEx, ImpersonateLoggedOnUser, RevertToSelf, LogonUserA, GetTokenInformation, AdjustTokenPrivileges) + 1 NTDLL$ (NtClose with NTSTATUS return, NTAPI calling convention). All signatures verified from MinGW 16.1.0 headers.
- `TK-BOF/Makefile`: 12 cross-compile targets (steal, use, make, rm, revert, privget × x64+x32). CFLAGS uses `-I .` (not `-I _include`) since bofdefs.h lives at the category root.
- Six command stubs with exact arg scaffolding per command spec.
- `_include/bofdefs.h`: KERNEL32$OpenProcess added in a new "process management (steal BOF)" section band.
- Root `Makefile`: TK-BOF appended to SUBDIRS.

## Verification Results

- `make -C TK-BOF/`: 12 [+] lines, 0 [!] lines
- `ls TK-BOF/_bin/ | sort`: 12 files (make.x32.o, make.x64.o, privget.x32.o, privget.x64.o, revert.x32.o, revert.x64.o, rm.x32.o, rm.x64.o, steal.x32.o, steal.x64.o, use.x32.o, use.x64.o)
- `file TK-BOF/_bin/steal.x64.o`: x86-64 COFF object
- `file TK-BOF/_bin/steal.x32.o`: Intel i386 COFF object
- `make clean && make` at repo root: succeeds (FS-BOF, Exit-BOF, TK-BOF all build)

## Task Commits

| Task | Name | Commit | Key Files |
|------|------|--------|-----------|
| 1 | Declare token API contracts | c04d07a | TK-BOF/bofdefs.h (created), _include/bofdefs.h (edited) |
| 2 | Write 6 silent BOF stubs | 25f8b9b | TK-BOF/steal/steal.c, use/use.c, make/make.c, rm/rm.c, revert/revert.c, privget/privget.c |
| 3 | Wire Makefile and root SUBDIRS | 633b4ff | TK-BOF/Makefile (created), Makefile (edited) |

## Deviations from Plan

None - plan executed exactly as written.

## Known Stubs

The six command `.c` files are intentional build-only stubs per design decisions D-04 and D-05. They parse their declared args and return silently — no logic, no output. This is by design: Phase 30 (Core Token BOFs) and Phase 31 (tk make + tk privget) will fill the stub bodies with real token-manipulation logic. The scaffolding (arg declarations, BeaconDataParse setup) established here will remain unchanged when Phase 30/31 implement the bodies.

| Stub | File | Reason |
|------|------|--------|
| steal.c body | TK-BOF/steal/steal.c | Intentional skeleton; Phase 30 implements OpenProcess + OpenProcessToken + DuplicateTokenEx + ImpersonateLoggedOnUser |
| use.c body | TK-BOF/use/use.c | Intentional skeleton; Phase 30 implements ImpersonateLoggedOnUser with stored handle |
| make.c body | TK-BOF/make/make.c | Intentional skeleton; Phase 31 implements LogonUserA + ImpersonateLoggedOnUser |
| rm.c body | TK-BOF/rm/rm.c | Intentional skeleton; Phase 30 implements NtClose |
| revert.c body | TK-BOF/revert/revert.c | Intentional skeleton; Phase 30 implements RevertToSelf |
| privget.c body | TK-BOF/privget/privget.c | Intentional skeleton; Phase 31 implements GetTokenInformation + AdjustTokenPrivileges |

## Self-Check: PASSED

Files created:
- TK-BOF/bofdefs.h: FOUND
- TK-BOF/Makefile: FOUND
- TK-BOF/steal/steal.c: FOUND
- TK-BOF/use/use.c: FOUND
- TK-BOF/make/make.c: FOUND
- TK-BOF/rm/rm.c: FOUND
- TK-BOF/revert/revert.c: FOUND
- TK-BOF/privget/privget.c: FOUND

Commits verified:
- c04d07a: FOUND (feat(29-01): declare token API contracts)
- 25f8b9b: FOUND (feat(29-01): write 6 silent BOF stubs)
- 633b4ff: FOUND (feat(29-01): wire TK-BOF Makefile and root SUBDIRS)
