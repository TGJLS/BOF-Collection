# Phase 23: Core Process BOFs - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-05-16
**Phase:** 23-core-process-bofs
**Areas discussed:** Adaptix beacon extensions, EnableDebugPrivilege, Error output style, GetUserByToken C translation

---

## Adaptix beacon extensions

| Option | Description | Selected |
|--------|-------------|----------|
| Extend `_include/beacon.h` | Add Adaptix extensions alongside existing Beacon APIs | |
| New `_include/adaptix.h` | Separate header for all Adaptix-specific functions | ✓ |

**User's choice:** `_include/adaptix.h` — Adaptix-specific extensions isolated in their own header

**Notes:** User identified that BeaconHeapAlloc/Free/ReAlloc should NOT be added since beacon.h already has BeaconFormatAlloc/Free for the formatp use case, and MSVCRT$ is the existing memory pattern. BeaconPrintfW also excluded — all error strings in this phase are narrow ASCII. Final adaptix.h scope: BeaconPkgBytes + BeaconPkgInt32 only. Investigation confirmed these write to a separate PACKAGE transport (UUID-tagged) that routes to the Adaptix Process Browser; they cannot be substituted with BeaconFormatAppend + BeaconOutput.

---

## EnableDebugPrivilege

| Option | Description | Selected |
|--------|-------------|----------|
| Skip it | Match Kharon's actual go() behavior; some protected process owners show N/A | ✓ |
| Call at top of go() | More owners resolved; generates 4703 audit event on sensitive targets | |

**User's choice:** Skip — Kharon defines the function but never calls it from go()

**Notes:** No OPSEC reason to deviate from Kharon's actual behavior.

---

## Error output style (kill / suspend / resume)

| Option | Description | Selected |
|--------|-------------|----------|
| Narrow `BeaconPrintf` | Matches FS-BOF/Exit-BOF; all error strings are ASCII | ✓ |
| Wide `BeaconPrintfW` | Matches Kharon source exactly; requires BeaconPrintfW in adaptix.h | |

**User's choice:** Narrow `BeaconPrintf` — keeps these simple BOFs free of Adaptix-specific declarations

---

## GetUserByToken C translation

| Option | Description | Selected |
|--------|-------------|----------|
| `goto cleanup` label | Idiomatic C for multi-resource error paths | ✓ |
| Separate helper function | Factor cleanup into free_user_token_resources() | |
| Inline at each exit point | Duplicate free() calls — verbose with 6+ exit paths | |

**User's choice:** `goto cleanup` — standard Windows C pattern for multi-resource cleanup

---

## Claude's Discretion

- **suspend.c / resume.c implementation**: No Kharon source exists; Claude to write from scratch following kill.c pattern (BeaconDataInt → OpenProcess → NtSuspendProcess/NtResumeProcess → CloseHandle → BeaconPrintf)
- **bofdefs.h additions**: Claude to add the missing Win32/NTDLL declarations per D-09 in CONTEXT.md

## Deferred Ideas

None — discussion stayed within phase scope.
