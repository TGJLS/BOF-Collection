# Summary: Plan 24-01 — bofdefs.h Additions + run.c Implementation

**Phase:** 24 — ps-run
**Plan:** 24-01
**Completed:** 2026-05-18
**Status:** Complete

## What Was Built

Added 10 new process-creation API declarations to `_include/bofdefs.h` and implemented
`PS-BOF/run/run.c` — replacing the empty stub with a fully-functional process-launch BOF
supporting three creation methods, PPID spoofing, and stdout/stderr pipe capture.

## Changes Made

### `_include/bofdefs.h`
Added two new comment-separated sections after the existing KERNEL32 process management block:

**KERNEL32 — process creation (PS-BOF run):** 8 declarations
- `KERNEL32$CreateProcessW` — default process creation
- `KERNEL32$GetStdHandle` — stdin handle for pipe setup
- `KERNEL32$CreatePipe` — anonymous pipe for stdout/stderr capture
- `KERNEL32$SetHandleInformation` — prevent child inheriting read end
- `KERNEL32$InitializeProcThreadAttributeList` — PPID spoofing attribute list
- `KERNEL32$UpdateProcThreadAttribute` — set PROC_THREAD_ATTRIBUTE_PARENT_PROCESS
- `KERNEL32$DeleteProcThreadAttributeList` — cleanup attribute list
- `KERNEL32$DuplicateHandle` — copy pipe_write into parent process for PPID+pipe combined case

**ADVAPI32 — process creation with credentials/token (PS-BOF run):** 2 declarations
- `ADVAPI32$CreateProcessWithLogonW` — launch as specified user
- `ADVAPI32$CreateProcessWithTokenW` — launch under stolen token

### `PS-BOF/run/run.c`
Full implementation with:
- Method constants: `CREATE_METHOD_DEFAULT=0`, `CREATE_METHOD_LOGON=1`, `CREATE_METHOD_TOKEN=2`
- Fallback `#define` guards for `EXTENDED_STARTUPINFO_PRESENT`, `PROC_THREAD_ATTRIBUTE_PARENT_PROCESS`, `LOGON_WITH_PROFILE`, `CREATE_NO_WINDOW`
- `PS_RUN_ARGS` struct (local to run.c)
- `fmt_err()` static helper — `KERNEL32$FormatMessageA` + `KERNEL32$LocalFree` (D-05)
- `read_pipe_output()` static helper — simple blocking `KERNEL32$ReadFile` loop, streams to `BeaconOutput` (D-04)
- `go()` entry point:
  - Parses 9 args in documented order (method, command, state, pipe, ppid, domain, username, password, token)
  - Copies `lpCommandLine` to writable `cmd_buf[32768]` before passing to CreateProcessW
  - `STARTUPINFOEXW` path for Default method (supports PPID spoofing)
  - Plain `STARTUPINFOW` for WithLogon/WithToken (these APIs reject attribute lists)
  - `DuplicateHandle` into parent process when PPID+pipe are both active
  - Closes `pipe_write` before `ReadFile` loop (required for EOF)
  - `goto cleanup` single exit point for all resources

## Key Design Decisions Applied
- D-01: `BeaconPrintf(CALLBACK_OUTPUT, "Process started: PID %lu, TID %lu\n")`
- D-02: `BeaconOutput(CALLBACK_OUTPUT, buf, len)` for pipe capture (binary-safe)
- D-04: Simple blocking ReadFile loop (no PeekNamedPipe, no timeout)
- D-05: `FormatMessageA` for human-readable error messages
- Research correction: `KERNEL32$DuplicateHandle` required when PPID spoofing + pipe are both active

## Commits
- `feat(24-01-T01): add 10 process-creation declarations to bofdefs.h` — bofdefs.h + run.c

## Acceptance Criteria Status
- [x] bofdefs.h contains all 10 new declarations
- [x] run.c parses 9 args in documented order
- [x] Default method supports PPID spoofing via STARTUPINFOEXW
- [x] Default method supports pipe capture via anonymous pipe
- [x] PPID+pipe combined uses DuplicateHandle into parent process
- [x] goto cleanup covers all resources
- [x] fmt_err uses FormatMessageA + LocalFree
- [x] Success message: "Process started: PID %lu, TID %lu"
- [x] Pipe output via BeaconOutput(CALLBACK_OUTPUT, ...)
