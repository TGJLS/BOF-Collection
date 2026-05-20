# Phase 25: ps grep - Context

**Gathered:** 2026-05-20
**Status:** Ready for planning

<domain>
## Phase Boundary

Implement `PS-BOF/grep/grep.c` — a single BOF that takes a PID as its only argument and outputs four sections: (1) token user, elevation type, and integrity level; (2) loaded modules with name, base address, entry point, and size; (3) command-line string; (4) thread IDs. No .axs wiring (Phase 26), no CI tests (Phase 28). The file replaces the existing empty stub.

</domain>

<decisions>
## Implementation Decisions

### Arg interface

- **D-01:** Accept PID only (`BeaconDataInt`) — always dump all 4 sections. No per-section boolean flags. PS-07 requires all four; flags would add dead code and complicate Phase 26 packing for no operator benefit.

### Thread enumeration

- **D-02:** Use `CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0)` + `Thread32First`/`Thread32Next` (Kharon's pattern). Filter by `th32OwnerProcessID == target_pid`. Output: TID per thread. Three new bofdefs.h declarations required: `KERNEL32$CreateToolhelp32Snapshot`, `KERNEL32$Thread32First`, `KERNEL32$Thread32Next`. Add under a new "KERNEL32 — toolhelp (PS-BOF grep)" section following existing style.

### Module output fields

- **D-03:** Output name, base address, entry point, and size per module. Matches Kharon's `get_modules()` output (adds `EntryPoint` beyond PS-07's minimum of name/base/size). Format: text table via `BeaconPrintf`.

### Output format

- **D-04:** Text output via `BeaconPrintf(CALLBACK_OUTPUT, ...)` throughout — no `BeaconPkgBytes`/`BeaconPkgInt32`. Consistent with kill/suspend/resume/run pattern. No `adaptix.h` involvement.

### Wide string handling

- **D-05:** Module names from `GetModuleFileNameExW` and cmdline from `NtQueryInformationProcess(ProcessCommandLineInformation)` are wide strings. Convert to narrow with `KERNEL32$WideCharToMultiByte(CP_UTF8, ...)` before `BeaconPrintf`. Pattern already established in list.c.

### Process access rights

- **D-06:** Open with `PROCESS_QUERY_INFORMATION | PROCESS_VM_READ`. `EnumProcessModulesEx` requires `PROCESS_VM_READ` in addition to `PROCESS_QUERY_INFORMATION`. Single `OpenProcess` call; same handle reused for all four helper functions.

### bofdefs.h additions (planner must add these)

Add under new sections following existing style:

```c
// KERNEL32 — toolhelp (PS-BOF grep)
WINBASEAPI HANDLE WINAPI KERNEL32$CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID);
WINBASEAPI BOOL   WINAPI KERNEL32$Thread32First(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
WINBASEAPI BOOL   WINAPI KERNEL32$Thread32Next(HANDLE hSnapshot, LPTHREADENTRY32 lpte);

// NTDLL — process info (PS-BOF grep)
WINBASEAPI NTSTATUS NTAPI NTDLL$NtQueryInformationProcess(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
```

Note: `<tlhelp32.h>` defines `THREADENTRY32` and `TH32CS_SNAPTHREAD`; it is already included transitively via `<windows.h>` in bofdefs.h.

### Claude's Discretion

- **Section ordering in output**: Token → Modules → Cmdline → Threads. Mirrors Kharon's call order in `go()`.
- **Error handling per section**: Each helper function is independent. If one fails (e.g., `OpenProcessToken` denied), output an error for that section and continue with the rest rather than aborting the whole BOF.
- **Cleanup**: `goto cleanup` at `go()` level for `process_handle`. Each helper manages its own local resources (token_handle, snapshot, cmdline buffer, etc.) internally and closes/frees before returning.
- **Integrity level string**: Use Kharon's thresholds exactly: `>= SECURITY_MANDATORY_SYSTEM_RID` → "System", `>= HIGH` → "High", `>= MEDIUM` → "Medium", `>= LOW` → "Low", else "Untrusted".
- **GetSidSubAuthority / GetSidSubAuthorityCount**: These are inline functions in the Win32 SDK (not dynamic-resolvable). Kharon calls them directly; we do the same.
- **NtQueryInformationToken vs GetTokenInformation**: Use `NTDLL$NtQueryInformationToken` (already in bofdefs.h) for TokenUser, TokenElevation, and TokenIntegrityLevel — equivalent to GetTokenInformation, avoids adding ADVAPI32$GetTokenInformation.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Kharon source (primary reference — translate C++ → C)

- `~/github/Kharon/agent_kharon/src_core/process/grep.cc` — four helpers in scope: `get_tokens()`, `get_modules()`, `get_cmdline()`, `get_threads()`. Out-of-scope helpers (do NOT implement): `get_policy`, `get_basicex`, `get_handles`, `get_instcallbacks`, `get_protection`.

### BOF infrastructure

- `_include/beacon.h` — BeaconPrintf, BeaconOutput, BeaconDataParse, BeaconDataInt
- `_include/bofdefs.h` — add 4 new declarations (see D-02 bofdefs.h additions block) before implementing; read existing section style to match. PSAPI declarations (`EnumProcessModulesEx`, `GetModuleFileNameExW`, `GetModuleInformation`) already present. `NTDLL$NtQueryInformationToken`, `ADVAPI32$OpenProcessToken`, `ADVAPI32$LookupAccountSidW`, `KERNEL32$WideCharToMultiByte`, `KERNEL32$CloseHandle`, `MSVCRT$malloc`/`free` already declared.
- `PS-BOF/list/list.c` — goto cleanup pattern, WideCharToMultiByte for wide→narrow, NtQuerySystemInformation allocation pattern
- `PS-BOF/kill/kill.c` — simplest BOF pattern: single BeaconDataInt arg, BeaconPrintf output

### Requirements

- `.planning/REQUIREMENTS.md` — PS-07 (token user/elevation/integrity, modules with base addr + size, cmdline, thread list with TIDs)

### Stub to replace

- `PS-BOF/grep/grep.c` — replace empty stub with full implementation

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets

- `NTDLL$NtQueryInformationToken` — already in bofdefs.h; use for TokenUser (TokenUser class), TokenElevation (TokenElevation class), TokenIntegrityLevel (TokenIntegrityLevel class)
- `ADVAPI32$OpenProcessToken` / `ADVAPI32$LookupAccountSidW` — already declared; same usage as in list.c GetUserByToken
- `PSAPI$EnumProcessModulesEx` / `PSAPI$GetModuleFileNameExW` / `PSAPI$GetModuleInformation` — already declared (Phase 22); drop-in for get_modules() translation
- `KERNEL32$WideCharToMultiByte` — already declared; use for module names and cmdline
- `MSVCRT$malloc` / `MSVCRT$free` — already declared; use for token_user buffer, integrity buffer, cmdline buffer
- `KERNEL32$CloseHandle` — already declared; for process_handle and snapshot handle

### Established Patterns

- **Dynamic resolution**: `KERNEL32$`, `NTDLL$`, `ADVAPI32$`, `PSAPI$`, `MSVCRT$` prefixes — never call Win32 APIs directly
- **goto cleanup**: Single cleanup block per function for multi-resource paths (see list.c)
- **Wide→narrow**: `KERNEL32$WideCharToMultiByte(CP_UTF8, 0, wide, -1, narrow, size, NULL, NULL)`
- **Error output**: `BeaconPrintf(CALLBACK_ERROR, "...")` narrow string
- **Success/data output**: `BeaconPrintf(CALLBACK_OUTPUT, "...")` narrow string
- **Build flags**: `-Os -DBOF -c` + strip — do not change

### Integration Points

- `PS-BOF/Makefile` — already has `grep/grep.c` targets for x64 and x32; no Makefile changes needed
- `PS-BOF/_bin/` — Makefile produces `grep.x64.o` and `grep.x32.o` here
- Phase 26 (ps.axs) — wires `ps grep <pid>` and packs a single integer PID arg per D-01

</code_context>

<specifics>
## Specific Ideas

- User wants EntryPoint included in module output (beyond PS-07 minimum) — matches Kharon's get_modules() output exactly
- PID-only arg interface confirmed; Phase 26 packing is one `BeaconDataInt(pid)` call

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>

---

*Phase: 25-ps-grep*
*Context gathered: 2026-05-20*
