---
phase: 23-core-process-bofs
plan: "01"
subsystem: PS-BOF / _include
tags:
  - bof
  - windows
  - process-enumeration
  - adaptix
  - package-transport
dependency_graph:
  requires: []
  provides:
    - _include/adaptix.h (BeaconPkgBytes/BeaconPkgInt32 declarations)
    - _include/bofdefs.h (10 new API declarations)
    - PS-BOF/list/list.c (compiled to list.x64.o, list.x32.o)
  affects:
    - PS-BOF kill/suspend/resume (Wave 2 — use new bofdefs.h declarations)
    - Phase 26 ps.axs (consumes PB-01 binary format from list BOF)
tech_stack:
  added:
    - adaptix.h: new Adaptix PACKAGE transport header (BeaconPkgBytes, BeaconPkgInt32)
  patterns:
    - NtQuerySystemInformation sizing + do/while traversal (Pattern 2-3)
    - GetUserByToken goto cleanup (D-07)
    - BeaconPkgBytes/BeaconPkgInt32 PACKAGE transport (D-02)
    - Manual wide-string concatenation (Pitfall 3 avoidance)
key_files:
  created:
    - _include/adaptix.h
  modified:
    - _include/bofdefs.h
    - PS-BOF/list/list.c
decisions:
  - "STATUS_BUFFER_TOO_SMALL defined locally in list.c (0xC0000023) — absent from MinGW winternl.h but correct Windows constant"
  - "No EnableDebugPrivilege in list.c per D-05 — ADVAPI32 declarations added to bofdefs.h for future use only"
  - "adaptix.h included only in list.c, not from bofdefs.h — minimum footprint per Open Question 1 recommendation"
metrics:
  duration: "~5 minutes"
  completed: "2026-05-16T10:37:57Z"
  tasks_completed: 2
  tasks_total: 2
  files_created: 1
  files_modified: 2
---

# Phase 23 Plan 01: Adaptix Infrastructure and PS list BOF Summary

**One-liner:** Adaptix PACKAGE transport headers + NtQuerySystemInformation process enumeration BOF emitting PB-01 binary format via BeaconPkgBytes/BeaconPkgInt32.

## What Was Built

### Task 1: Create `_include/adaptix.h` and extend `_include/bofdefs.h`

**`_include/adaptix.h`** (new, 4 lines):
- `#pragma once` header guard
- `DECLSPEC_IMPORT VOID BeaconPkgBytes(PBYTE Buffer, ULONG Length, PCHAR UUID);`
- `DECLSPEC_IMPORT VOID BeaconPkgInt32(INT32 Data, PCHAR UUID);`
- Caller always passes NULL for UUID (no UUID routing needed for ps list)

**`_include/bofdefs.h`** (+21 lines, 3 new sections + 1 declaration):
- `MSVCRT$malloc` added to existing MSVCRT block (alongside calloc/free)
- New section: KERNEL32 process management (OpenProcess, TerminateProcess, IsWow64Process, GetCurrentProcess) — 4 declarations, `WINBASEAPI WINAPI` style
- New section: ADVAPI32 token/privilege (OpenProcessToken, LookupAccountSidW, AdjustTokenPrivileges, LookupPrivilegeValueW) — 4 declarations, `WINADVAPI WINAPI` style
- New section: NTDLL token query (NtQueryInformationToken) — 1 declaration, `WINBASEAPI NTAPI` style

All 10 required declarations confirmed present. All pre-existing declarations unchanged. Build sanity: 12 `[+]` lines, 0 `[!]` after Task 1.

**Commit:** e72095e

### Task 2: Port `list.cc` to `PS-BOF/list/list.c`

**`PS-BOF/list/list.c`** (141 lines — stub replaced):
- Include order: `<windows.h>`, `"bofdefs.h"`, `"beacon.h"`, `"adaptix.h"`
- `STATUS_BUFFER_TOO_SMALL` defined locally as `((NTSTATUS)0xC0000023)` — absent from MinGW winternl.h (auto-fix deviation, see below)
- `GetUserByToken` helper: `goto cleanup` pattern (D-07), NtQueryInformationToken + LookupAccountSidW, manual index-walk domain\user concatenation (no swprintf — Pitfall 3)
- `go()`: no BeaconDataParse (list takes no args — Pitfall 6), NtQuerySystemInformation sizing call (return code not checked — Pitfall 1), MSVCRT$malloc buffer, do/while traversal with base pointer saved (Pitfall 4), all 6 PB-01 fields per process
- PB-01 field order: name wstr (ImageName.Length bytes direct), PID int32, PPID int32, session int32, user wstr (wcslen * sizeof), arch int32
- No EnableDebugPrivilege (D-05), no swprintf/wsprintf (Pitfall 3), no BeaconOutput/BeaconFormatAppend (D-02), no nt_success() (uses NT_SUCCESS()), no direct Win32 calls

**Artifacts:** `PS-BOF/_bin/list.x64.o` and `PS-BOF/_bin/list.x32.o` produced.

**Commit:** 4cb6091

## Build Verification

```
[+] list x64
[+] list x32
[+] kill x64
[+] kill x32
[+] run x64
[+] run x32
[+] grep x64
[+] grep x32
[+] suspend x64
[+] suspend x32
[+] resume x64
[+] resume x32
```

12 `[+]` lines, 0 `[!]` lines. All 6 PS-BOF targets (12 total with x64/x32) build cleanly.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] `STATUS_BUFFER_TOO_SMALL` not defined in MinGW winternl.h**

- **Found during:** Task 2, first compile attempt
- **Issue:** `STATUS_BUFFER_TOO_SMALL` (0xC0000023) is in MinGW's `ntstatus.h` but NOT in `winternl.h` which is what bofdefs.h includes. Compiler emitted `error: 'STATUS_BUFFER_TOO_SMALL' undeclared`.
- **Fix:** Added `#ifndef STATUS_BUFFER_TOO_SMALL` / `#define STATUS_BUFFER_TOO_SMALL ((NTSTATUS)0xC0000023)` / `#endif` to list.c immediately after the four includes. This is the correct Windows constant value (confirmed in `/usr/x86_64-w64-mingw32/include/ntstatus.h`).
- **Files modified:** `PS-BOF/list/list.c`
- **Commit:** 4cb6091 (included with Task 2)

## Requirements Satisfied

| Requirement | Status | Evidence |
|-------------|--------|---------|
| PS-01 (process list: name/PID/PPID/session/user/arch) | Satisfied at source level | list.c emits all 6 fields per process via PACKAGE transport |
| PB-01 (Adaptix-compatible binary format) | Satisfied | BeaconPkgBytes (name, user) + BeaconPkgInt32 (PID, PPID, session, arch) in correct order |

Runtime validation deferred to Phase 28 (live Windows beacon required).

## Known Stubs

None in this plan. list.c is fully implemented.

## Threat Flags

No new security-relevant surface introduced beyond the plan's threat model. All T-23-0x mitigations applied:
- T-23-01: PROCESS_QUERY_LIMITED_INFORMATION only (not PROCESS_ALL_ACCESS)
- T-23-02: EnableDebugPrivilege omitted (D-05)
- T-23-03: GetUserByToken frees user_domain and sets NULL on failure path
- T-23-05: ImageName.Length used directly (not multiplied by sizeof(WCHAR))
- T-23-06: base_sysproc saved before loop, freed after loop

## Self-Check: PASSED

- `_include/adaptix.h` exists: CONFIRMED
- `_include/bofdefs.h` has all 10 new declarations: CONFIRMED
- `PS-BOF/list/list.c` exists, 141 lines: CONFIRMED
- `PS-BOF/_bin/list.x64.o` exists: CONFIRMED
- `PS-BOF/_bin/list.x32.o` exists: CONFIRMED
- Commits e72095e and 4cb6091 exist in git log: CONFIRMED
- Build: 12 `[+]`, 0 `[!]`: CONFIRMED
