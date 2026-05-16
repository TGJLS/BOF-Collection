# Phase 23: Core Process BOFs - Context

**Gathered:** 2026-05-16
**Status:** Ready for planning

<domain>
## Phase Boundary

Implement four working BOFs — ps list, ps kill, ps suspend, ps resume — by replacing the Phase 22 stubs with production C code ported and translated from Kharon C++. ps list produces Adaptix-compatible binary output for the Process Browser (PB-01). No axs wiring, no CI tests — those belong to Phases 26 and 28.

</domain>

<decisions>
## Implementation Decisions

### Adaptix-specific beacon API

- **D-01:** Create `_include/adaptix.h` (new root-level shared header) with C-compatible declarations for `BeaconPkgBytes` and `BeaconPkgInt32`. These are the only Adaptix extensions required for this phase; they are exported to BOFs by Adaptix's COFF ApiTable (entries 31 and 34) but absent from the standard `_include/beacon.h`.
- **D-02:** `BeaconPkgBytes` and `BeaconPkgInt32` CANNOT be replaced by `BeaconFormatAppend`/`BeaconFormatInt` + `BeaconOutput` — they write to a separate UUID-tagged `PACKAGE` transport that routes to the Adaptix Process Browser. Standard output functions write to the text output channel and will never reach the Process Browser.
- **D-03:** `BeaconPrintfW` is NOT added to `adaptix.h`. All error and status strings in kill/suspend/resume are plain ASCII; narrow `BeaconPrintf` is used throughout.
- **D-04:** `BeaconHeapAlloc`/`BeaconHeapFree` are NOT added to `adaptix.h`. Dynamic memory uses `MSVCRT$malloc`/`MSVCRT$free` (existing BOF pattern; `intAlloc`/base.c use `MSVCRT$calloc`). `BeaconFormatAlloc`/`BeaconFormatFree` (already in `beacon.h`) serve the `formatp` text-buffer use case.

### EnableDebugPrivilege in ps list

- **D-05:** Skip `EnableDebugPrivilege` entirely — do not include it in list.c and do not call it. Kharon defines it in list.cc but never calls it from `go()`. Without SeDebugPrivilege, OpenProcess fails on a handful of protected processes (System, smss.exe, csrss.exe) — their owner will show "N/A". This matches Kharon's actual runtime behavior and avoids generating a 4703 token audit event.

### Error and success output (kill / suspend / resume)

- **D-06:** Use narrow `BeaconPrintf(CALLBACK_ERROR, "...")` and `BeaconPrintf(CALLBACK_OUTPUT, "...")` for all error and success messages in kill.c, suspend.c, and resume.c. Matches the FS-BOF and Exit-BOF style. No wide-string output functions needed in these three simple BOFs.

### GetUserByToken C translation

- **D-07:** Translate Kharon's C++ cleanup lambda in `GetUserByToken` to a `goto cleanup` pattern. A single cleanup block at the bottom of the function frees `token_user_ptr`, `domain`, and `username`, and conditionally frees `user_domain` on the failure path. This is idiomatic C for multi-resource error paths.

### suspend.c and resume.c (no Kharon source)

- **D-08 (Claude's discretion):** No Kharon source exists for suspend or resume. Implement from scratch following the kill.c pattern: parse PID with `BeaconDataInt`, call `KERNEL32$OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid)`, call `NTDLL$NtSuspendProcess` or `NTDLL$NtResumeProcess`, close the handle, report success/failure with `BeaconPrintf`. Roughly 20 lines each.

### bofdefs.h additions

- **D-09 (Claude's discretion):** The planner must add the following missing declarations to `_include/bofdefs.h` before list.c and kill.c can compile. Group under new KERNEL32/ADVAPI32 sections following existing style:
  - `KERNEL32$OpenProcess(DWORD, BOOL, DWORD) -> HANDLE`
  - `KERNEL32$TerminateProcess(HANDLE, UINT) -> BOOL`
  - `KERNEL32$IsWow64Process(HANDLE, PBOOL) -> BOOL`
  - `KERNEL32$GetCurrentProcess() -> HANDLE`
  - `ADVAPI32$OpenProcessToken(HANDLE, DWORD, PHANDLE) -> BOOL`
  - `ADVAPI32$LookupAccountSidW(LPCWSTR, PSID, LPWSTR, LPDWORD, LPWSTR, LPDWORD, PSID_NAME_USE) -> BOOL`
  - `ADVAPI32$AdjustTokenPrivileges(HANDLE, BOOL, PTOKEN_PRIVILEGES, DWORD, PTOKEN_PRIVILEGES, PDWORD) -> BOOL`
  - `ADVAPI32$LookupPrivilegeValueW(LPCWSTR, LPCWSTR, PLUID) -> BOOL`
  - `NTDLL$NtQueryInformationToken(HANDLE, TOKEN_INFORMATION_CLASS, PVOID, ULONG, PULONG) -> NTSTATUS`
  - `NTDLL$NtQuerySystemInformation` is already declared; `NTDLL$NtSuspendProcess` and `NTDLL$NtResumeProcess` are already declared.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Kharon source (primary reference — translate C++ → C)

- `~/github/Kharon/agent_kharon/src_core/process/list.cc` — NtQuerySystemInformation loop, SYSTEM_PROCESS_INFORMATION traversal, GetUserByToken helper (OpenProcessToken + LookupAccountSidW), IsWow64Process for arch field, BeaconPkgBytes/Int32 output format
- `~/github/Kharon/agent_kharon/src_core/process/kill.cc` — OpenProcess + TerminateProcess pattern, BeaconDataInt argument parsing

### BOF infrastructure files (do not change build flags or patterns)

- `_include/beacon.h` — standard BOF beacon API: BeaconPrintf, BeaconOutput, BeaconDataParse, BeaconDataInt, formatp API
- `_include/bofdefs.h` — add missing declarations per D-09 before implementing; read existing section headers to match style
- `_include/adaptix.h` — NEW file to create: BeaconPkgBytes and BeaconPkgInt32 (C-compatible, no default param)
- `PS-BOF/Makefile` — includes `-I ../_include`; `_include/adaptix.h` is already on the include path via this flag — no Makefile changes needed
- `Exit-BOF/exitprocess/exitprocess.c` — simplest BOF pattern reference (suspend/resume will be similar in complexity)

### Requirements

- `.planning/REQUIREMENTS.md` — PS-01 (ps list fields), PS-02 (ps kill + optional exit code), PS-08 (suspend), PS-09 (resume), PB-01 (binary format per-process: name wstr, PID int32, PPID int32, session int32, user wstr, arch int32)

### Stub files to fill in

- `PS-BOF/list/list.c` — replace stub with full port from list.cc
- `PS-BOF/kill/kill.c` — replace stub with port from kill.cc
- `PS-BOF/suspend/suspend.c` — replace stub with scratch implementation per D-08
- `PS-BOF/resume/resume.c` — replace stub with scratch implementation per D-08

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets

- `NTDLL$NtSuspendProcess` / `NTDLL$NtResumeProcess` / `NTDLL$NtQuerySystemInformation` — already declared in `_include/bofdefs.h` (added in Phase 22); no further work needed for the NTDLL side
- `_include/beacon.h` `datap` API — `BeaconDataParse` + `BeaconDataInt` for parsing PID and exit_code arguments, consistent with kill.cc's pattern
- `KERNEL32$CloseHandle` — already declared in `_include/bofdefs.h` line 45

### Established Patterns

- **Dynamic resolution**: `KERNEL32$`, `NTDLL$`, `ADVAPI32$`, `MSVCRT$` prefixes — NEVER call Win32 APIs directly; every function resolves through the BOF import mechanism
- **Memory**: `MSVCRT$malloc` / `MSVCRT$free` for dynamic heap buffers; no `BeaconHeapAlloc` (not needed for this phase)
- **Build flags**: `-Os -DBOF -c` + `--strip-unneeded`; do not change
- **Error output**: `BeaconPrintf(CALLBACK_ERROR, "...")` narrow, matching FS-BOF/Exit-BOF convention
- **Success output (kill/suspend/resume)**: `BeaconPrintf(CALLBACK_OUTPUT, "Process killed\n")` style

### Integration Points

- `PS-BOF/_bin/` — Makefile already creates and populates this; all four BOFs produce `.x64.o` and `.x86.o` outputs here
- Phase 26 (ps.axs) consumes the binary output from ps list via the Process Browser — the PB-01 format (BeaconPkgBytes/Int32 per-process layout) must be correct before Phase 26 can wire it

### Architecture field (IsWow64Process)

- Kharon uses `IsWow64Process(handle, &Isx64)` → `BeaconPkgInt32(Isx64)`. `IsWow64Process` returns TRUE for 32-bit processes running on 64-bit Windows (WoW64). So the arch int32 is `1` for x86, `0` for native x64. Preserve this exactly — the Process Browser parser was built against this convention.

</code_context>

<specifics>
## Specific Ideas

- User confirmed: `_include/adaptix.h` as the canonical location for Adaptix-specific BOF extensions — sets the pattern for Phase 25 (ps grep uses EnumProcessModulesEx output) and Phase 26 (ps.axs wiring)
- `BeaconPkgBytes`/`BeaconPkgInt32` C declarations must drop the C++ default UUID parameter. Caller always passes NULL for this phase (no UUID routing needed for the process list data).

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>

---

*Phase: 23-Core Process BOFs*
*Context gathered: 2026-05-16*
