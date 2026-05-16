# Phase 24: ps run - Context

**Gathered:** 2026-05-16
**Status:** Ready for planning

<domain>
## Phase Boundary

Implement `PS-BOF/run/run.c` — a single BOF file that launches a new process using one of three creation methods (CreateProcess, CreateProcessWithLogon, CreateProcessWithToken) with optional PPID spoofing and stdout/stderr capture via anonymous pipe. No .axs wiring (Phase 26), no CI tests (Phase 28). The file replaces the existing empty stub.

</domain>

<decisions>
## Implementation Decisions

### Output format

- **D-01:** Use `BeaconPrintf(CALLBACK_OUTPUT, "Process started: PID %d, TID %d\n", pid, tid)` on success. Text output only — no `BeaconPkgBytes`/`BeaconPkgInt32`, no `adaptix.h`. Works on all agents (not Adaptix-specific). Consistent with kill/suspend/resume pattern.
- **D-02:** When `--pipe` is set and stdout/stderr is captured, deliver output via `BeaconOutput(CALLBACK_OUTPUT, buf, len)` — binary-safe, already in `beacon.h`. Do NOT use `BeaconPrintf` for pipe output (breaks on null bytes or non-printable chars).
- **D-03:** `_include/adaptix.h` is NOT recreated for this phase. The decision to use BeaconPrintf/BeaconOutput is a deliberate departure from Kharon's `BeaconPkgBytes`/`BeaconPkgInt32` approach.

### Pipe read approach

- **D-04:** Use a simple blocking `ReadFile` loop. After `CreateProcessW` succeeds, close `pipe_write`; then loop `KERNEL32$ReadFile(pipe_read, ...)` appending into a growing buffer until `ReadFile` returns FALSE (ERROR_BROKEN_PIPE or process exit). No `PeekNamedPipe`, no `WaitForSingleObject` polling, no timeout. Works reliably for short-lived commands (the expected use case: `cmd /c whoami`, etc.). If the operator launches a long-running process with `--pipe`, the BOF blocks until that process exits — this is expected behavior.

### Error messages

- **D-05:** Use `KERNEL32$FormatMessageA` (already declared in `_include/bofdefs.h`) for human-readable error messages at all failure points. Format: `BeaconPrintf(CALLBACK_ERROR, "CreateProcessW failed (%d): %s\n", err, msg)`. ps run has significantly more failure points than kill — readable messages are more operator-friendly. Free the FormatMessageA buffer with `KERNEL32$LocalFree` (already in bofdefs.h).

### Claude's Discretion

- **Arg parse order**: method (BeaconDataInt), command (BeaconDataExtract wstr), state (BeaconDataInt: 0=normal, 1=CREATE_SUSPENDED), pipe (BeaconDataInt: 0=no capture, 1=capture), ppid (BeaconDataInt: 0=no spoofing, nonzero=target parent PID), domain (BeaconDataExtract wstr), username (BeaconDataExtract wstr), password (BeaconDataExtract wstr), token (BeaconDataInt as HANDLE). Follows Kharon's order plus explicit ppid (replacing BeaconInformation).
- **Method enum**: Plain `#define` constants (CREATE_METHOD_DEFAULT=0, CREATE_METHOD_LOGON=1, CREATE_METHOD_TOKEN=2) — no C++ enum class.
- **PS_CREATE_ARGS struct**: Define as a local C struct in run.c — no shared header needed since run.c is the only consumer.
- **Static helper**: `static PBYTE read_pipe_output(HANDLE pipe_read, ULONG *out_len)` — self-contained pipe reader, returns malloc'd buffer (caller frees) or NULL on failure.
- **Cleanup**: `goto cleanup` pattern (established in list.c) for multi-resource error paths. Resources: `attribute_buff`, `pipe_read`, `pipe_write`, `parent_handle`, `process_info` handles.
- **No spoofarg**: Entirely dropped (out of scope per REQUIREMENTS.md PS-EX-04). No `NtQueryInformationProcess`, `ReadProcessMemory`, `WriteProcessMemory` needed.
- **No blockdlls**: Entirely dropped (out of scope per REQUIREMENTS.md).
- **No DuplicateHandle**: Kharon duplicates pipe_write into parent_handle's process for PPID+pipe combined use. Our simplified approach: bInheritHandles=TRUE causes child to inherit pipe_write from the calling BOF process's handle table regardless of PROC_THREAD_ATTRIBUTE_PARENT_PROCESS. DuplicateHandle not required.
- **EXTENDED_STARTUPINFO_PRESENT**: Used only for CreateProcess (Default method). WithLogon and WithToken use plain STARTUPINFOW — those APIs don't support extended startup info with attribute lists.
- **STARTF_USESHOWWINDOW | SW_HIDE**: Set in startup info for all methods to suppress console window.

### bofdefs.h additions (planner must add these 9 declarations)

Add under new "process creation (PS-BOF run)" sections, following existing style:

```c
// KERNEL32 — process creation (PS-BOF run)
WINBASEAPI BOOL WINAPI KERNEL32$CreateProcessW(LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
WINBASEAPI HANDLE WINAPI KERNEL32$GetStdHandle(DWORD nStdHandle);
WINBASEAPI BOOL WINAPI KERNEL32$CreatePipe(PHANDLE, PHANDLE, LPSECURITY_ATTRIBUTES, DWORD);
WINBASEAPI BOOL WINAPI KERNEL32$SetHandleInformation(HANDLE, DWORD, DWORD);
WINBASEAPI BOOL WINAPI KERNEL32$InitializeProcThreadAttributeList(LPPROC_THREAD_ATTRIBUTE_LIST, DWORD, DWORD, PSIZE_T);
WINBASEAPI BOOL WINAPI KERNEL32$UpdateProcThreadAttribute(LPPROC_THREAD_ATTRIBUTE_LIST, DWORD, DWORD_PTR, PVOID, SIZE_T, PVOID, PSIZE_T);
WINBASEAPI VOID WINAPI KERNEL32$DeleteProcThreadAttributeList(LPPROC_THREAD_ATTRIBUTE_LIST);

// ADVAPI32 — process creation with credentials/token (PS-BOF run)
WINADVAPI BOOL WINAPI ADVAPI32$CreateProcessWithLogonW(LPCWSTR, LPCWSTR, LPCWSTR, DWORD, LPCWSTR, LPWSTR, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
WINADVAPI BOOL WINAPI ADVAPI32$CreateProcessWithTokenW(HANDLE, DWORD, LPCWSTR, LPWSTR, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
```

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Kharon source (primary reference — translate C++ → C)

- `~/github/Kharon/agent_kharon/src_core/process/create.cc` — BOF entry point: arg parsing order (method, command, state, pipe, domain, username, password, token), BeaconPkgInt32/Bytes output (we replace with BeaconPrintf/BeaconOutput)
- `~/github/Kharon/agent_kharon/src_core/kit/kit_process_creation.cc` — full process creation logic: STARTUPINFOEXW setup, attribute list for PPID spoofing, CreatePipe + SetHandleInformation, switch on method (CreateProcessW / CreateProcessWithLogonW / CreateProcessWithTokenW), pipe read loop (we simplify)
- `~/github/Kharon/agent_kharon/src_core/include/general.h` — PS_CREATE_ARGS struct definition, Create enum, `nt_current_process()` macro (`(HANDLE)-1`), `fmt_error()` helper (FormatMessageW — we use FormatMessageA instead)

### BOF infrastructure (do not change build flags or patterns)

- `_include/beacon.h` — BeaconPrintf, BeaconOutput, BeaconDataParse, BeaconDataExtract, BeaconDataInt, formatp API
- `_include/bofdefs.h` — add the 9 new declarations from D-06 before implementing; read existing section style; KERNEL32$FormatMessageA and KERNEL32$LocalFree already declared (line 37-38), KERNEL32$ReadFile already declared (line 44), KERNEL32$CloseHandle already declared (line 45)
- `PS-BOF/kill/kill.c` — established BOF pattern: arg parsing, error output, success output
- `PS-BOF/list/list.c` — goto cleanup pattern for multi-resource paths

### Requirements

- `.planning/REQUIREMENTS.md` — PS-03 (CreateProcess + pipe capture), PS-04 (CreateProcessWithLogon), PS-05 (CreateProcessWithToken), PS-06 (PPID spoofing via --ppid)

### Stub file to replace

- `PS-BOF/run/run.c` — replace empty stub with full implementation

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets

- `KERNEL32$FormatMessageA` + `KERNEL32$LocalFree` — already in bofdefs.h; use for human-readable error messages
- `KERNEL32$ReadFile` — already declared; used in pipe read loop
- `KERNEL32$CloseHandle` — already declared; used for pipe handles, process/thread handles, parent_handle
- `KERNEL32$OpenProcess(PROCESS_CREATE_PROCESS | PROCESS_DUP_HANDLE, ...)` — already declared; used for parent_handle when ppid≠0
- `MSVCRT$malloc` / `MSVCRT$free` — already declared; used for attribute_buff and pipe output buffer

### Established Patterns

- **Dynamic resolution**: All Win32 calls via BOF prefixes — `KERNEL32$`, `ADVAPI32$`, `NTDLL$`, `MSVCRT$`. Never call Win32 APIs directly.
- **Arg parsing**: `BeaconDataParse(&parser, args, len)` then `BeaconDataInt` / `BeaconDataExtract` in fixed order matching .axs packing.
- **Error output**: `BeaconPrintf(CALLBACK_ERROR, "...")` narrow string.
- **Success output**: `BeaconPrintf(CALLBACK_OUTPUT, "...")` narrow string.
- **Pipe output**: `BeaconOutput(CALLBACK_OUTPUT, buf, len)` for raw captured bytes.
- **Build flags**: `-Os -DBOF -c` + strip — do not change.
- **goto cleanup**: Single cleanup block at function bottom (see list.c GetUserByToken).

### Integration Points

- `PS-BOF/Makefile` — already has `run/run.c` targets for x64 and x32; no Makefile changes needed
- `PS-BOF/_bin/` — Makefile populates this; run.x64.o and run.x32.o produced here
- Phase 26 (ps.axs) — wires the `--command`, `--state`, `--pipe`, `--domain`, `--username`, `--password`, `--token`, `--ppid` flags and packs them in the arg order from D-Discretion above

</code_context>

<specifics>
## Specific Ideas

- User explicitly chose `BeaconPrintf`/`BeaconOutput` over Kharon's binary format: "it should work on all agents. do not recreate use native Beacon functions"
- Pipe read should be the simple blocking approach — the use case is short commands; blocking on a long-running process with `--pipe` is acceptable/expected behavior

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>

---

*Phase: 24-ps-run*
*Context gathered: 2026-05-16*
