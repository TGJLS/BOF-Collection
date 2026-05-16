# Phase 24: ps run - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-05-16
**Phase:** 24-ps-run
**Areas discussed:** Output format (BeaconPkgBytes/Int32 + adaptix.h), Output format (PID/TID text), Output format (pipe output delivery), Pipe read approach, Error messages

---

## Output Format — BeaconPkgBytes/Int32 + adaptix.h

| Option | Description | Selected |
|--------|-------------|----------|
| BeaconPrintf text | Use native Beacon functions for output; no adaptix.h; works on all agents | ✓ |
| BeaconPkgInt32/Bytes binary | Recreate _include/adaptix.h; match Kharon exactly; binary PID/TID/bytes | |

**User's choice:** BeaconPrintf text — "No I don't want to use the Kharon format. it should work on all agents. do not recreate use native Beacon functions"
**Notes:** adaptix.h was already deleted in Phase 23 execution. User confirmed the BeaconPrintf pattern is the right path forward, not just a workaround.

---

## Output Format — Success output (PID/TID)

| Option | Description | Selected |
|--------|-------------|----------|
| PID + TID on success | BeaconPrintf "Process started: PID %d, TID %d\n" | ✓ |
| PID only | Minimal; "Process started: PID %d\n" | |
| Silent on success | No output; operator uses ps list to find spawned process | |

**User's choice:** PID + TID on success
**Notes:** PID is needed for Phase 28 CI test (spawn process then kill by PID). TID included for completeness.

---

## Output Format — Pipe output delivery

| Option | Description | Selected |
|--------|-------------|----------|
| BeaconOutput(CALLBACK_OUTPUT, buf, len) | Binary-safe; already in beacon.h | ✓ |
| BeaconPrintf(CALLBACK_OUTPUT, "%s", buf) | Simpler but breaks on null bytes | |

**User's choice:** BeaconOutput — binary-safe delivery

---

## Pipe Read Approach

| Option | Description | Selected |
|--------|-------------|----------|
| Simple blocking ReadFile loop | ~15 lines; ReadFile until ERROR_BROKEN_PIPE; works for short commands | ✓ |
| Kharon's polling loop with timeout | ~50 lines; PeekNamedPipe + WaitForSingleObject + 10s timeout; handles long-running processes | |

**User's choice:** Simple blocking ReadFile loop
**Notes:** Expected use case is short commands (cmd /c whoami). Blocking on a long-running process with --pipe is acceptable/expected behavior. Keeps implementation simple.

---

## Error Messages

| Option | Description | Selected |
|--------|-------------|----------|
| Error code only (like kill.c) | BeaconPrintf error code number only | |
| FormatMessageA human-readable | KERNEL32$FormatMessageA already in bofdefs.h; include readable string + code | ✓ |

**User's choice:** FormatMessageA human-readable
**Notes:** ps run has significantly more failure points than kill — readable messages improve operator experience.

---

## Claude's Discretion

- Method enum: plain `#define` constants instead of C++ `enum class Create`
- PS_CREATE_ARGS struct: defined locally in run.c (no shared header needed)
- Arg parse order: method, command, state, pipe, ppid, domain, username, password, token
- Static `read_pipe_output` helper function in run.c
- goto cleanup pattern for multi-resource cleanup
- No DuplicateHandle (simplified from Kharon — bInheritHandles=TRUE is sufficient)
- No spoofarg, no blockdlls (out of scope)
- 9 new bofdefs.h declarations for process creation APIs

## Deferred Ideas

None — discussion stayed within phase scope.
