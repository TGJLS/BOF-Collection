# Phase 23: Core Process BOFs - Research

**Researched:** 2026-05-16
**Domain:** Windows BOF development — process enumeration, termination, suspension/resumption; Adaptix PACKAGE transport
**Confidence:** HIGH

---

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

- **D-01:** Create `_include/adaptix.h` (new root-level shared header) with C-compatible declarations for `BeaconPkgBytes` and `BeaconPkgInt32`. These are the only Adaptix extensions required for this phase; they are exported to BOFs by Adaptix's COFF ApiTable (entries 31 and 34) but absent from the standard `_include/beacon.h`.
- **D-02:** `BeaconPkgBytes`/`BeaconPkgInt32` CANNOT be replaced by `BeaconFormatAppend`/`BeaconFormatInt` + `BeaconOutput` — they write to a separate UUID-tagged PACKAGE transport that routes to the Adaptix Process Browser.
- **D-03:** `BeaconPrintfW` is NOT added to `adaptix.h`. All error and status strings in kill/suspend/resume are plain ASCII; narrow `BeaconPrintf` is used throughout.
- **D-04:** `BeaconHeapAlloc`/`BeaconHeapFree` are NOT added to `adaptix.h`. Dynamic memory uses `MSVCRT$malloc`/`MSVCRT$free`.
- **D-05:** Skip `EnableDebugPrivilege` entirely — do not include it in list.c and do not call it.
- **D-06:** Use narrow `BeaconPrintf(CALLBACK_ERROR, "...")` and `BeaconPrintf(CALLBACK_OUTPUT, "...")` for all error and success messages in kill.c, suspend.c, and resume.c.
- **D-07:** Translate Kharon's C++ cleanup lambda in `GetUserByToken` to a `goto cleanup` pattern in C.
- **D-08 (Claude's Discretion):** No Kharon source exists for suspend or resume. Implement from scratch following the kill.c pattern: parse PID with `BeaconDataInt`, call `KERNEL32$OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid)`, call `NTDLL$NtSuspendProcess` or `NTDLL$NtResumeProcess`, close the handle, report success/failure with `BeaconPrintf`. Roughly 20 lines each.
- **D-09 (Claude's Discretion):** Add missing declarations to `_include/bofdefs.h` before list.c and kill.c can compile. Group under new KERNEL32/ADVAPI32 sections following existing style:
  - `KERNEL32$OpenProcess(DWORD, BOOL, DWORD) -> HANDLE`
  - `KERNEL32$TerminateProcess(HANDLE, UINT) -> BOOL`
  - `KERNEL32$IsWow64Process(HANDLE, PBOOL) -> BOOL`
  - `KERNEL32$GetCurrentProcess() -> HANDLE`
  - `ADVAPI32$OpenProcessToken(HANDLE, DWORD, PHANDLE) -> BOOL`
  - `ADVAPI32$LookupAccountSidW(LPCWSTR, PSID, LPWSTR, LPDWORD, LPWSTR, LPDWORD, PSID_NAME_USE) -> BOOL`
  - `ADVAPI32$AdjustTokenPrivileges(HANDLE, BOOL, PTOKEN_PRIVILEGES, DWORD, PTOKEN_PRIVILEGES, PDWORD) -> BOOL`
  - `ADVAPI32$LookupPrivilegeValueW(LPCWSTR, LPCWSTR, PLUID) -> BOOL`
  - `NTDLL$NtQueryInformationToken(HANDLE, TOKEN_INFORMATION_CLASS, PVOID, ULONG, PULONG) -> NTSTATUS`
  - `NTDLL$NtSuspendProcess` and `NTDLL$NtResumeProcess` are ALREADY declared (Phase 22).

### Claude's Discretion

- D-08: suspend.c and resume.c implementation from scratch (pattern per kill.c).
- D-09: Exact placement and grouping of new bofdefs.h declarations.

### Deferred Ideas (OUT OF SCOPE)

None — discussion stayed within phase scope.
</user_constraints>

---

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| PS-01 | Operator can list all running processes with name, PID, PPID, session ID, owner (domain\user), and architecture | NtQuerySystemInformation loop + GetUserByToken + IsWow64Process pattern from list.cc |
| PS-02 | Operator can terminate a process by PID with an optional exit code | OpenProcess + TerminateProcess pattern from kill.cc; BeaconDataInt for two args |
| PS-08 | Operator can suspend a running process by PID | OpenProcess(PROCESS_SUSPEND_RESUME) + NtSuspendProcess; already declared in bofdefs.h |
| PS-09 | Operator can resume a suspended process by PID | OpenProcess(PROCESS_SUSPEND_RESUME) + NtResumeProcess; already declared in bofdefs.h |
| PB-01 | ps list BOF packs output in Adaptix-compatible format (per-process: name wstr, PID int32, PPID int32, session int32, user wstr, arch int32) | BeaconPkgBytes/BeaconPkgInt32 via adaptix.h; ApiTable entries 31 and 34 confirmed in Kharon source |
</phase_requirements>

---

## Summary

Phase 23 ports four C++ BOF source files from Kharon's process management subsystem into standard C, adds two infrastructure files (`_include/adaptix.h` and nine new declarations in `_include/bofdefs.h`), and replaces four stub `.c` files with production implementations.

The central complexity is `list.c`. It calls `NtQuerySystemInformation(SystemProcessInformation, ...)` to get a contiguous buffer of `SYSTEM_PROCESS_INFORMATION` records linked by `NextEntryOffset`, traverses them in a do/while loop, calls `OpenProcess` + `OpenProcessToken` + `NtQueryInformationToken` + `LookupAccountSidW` to resolve each process owner, and calls `IsWow64Process` to determine architecture. Each process entry is emitted as a pair of `BeaconPkgBytes` (name wstr, user wstr) and four `BeaconPkgInt32` calls (PID, PPID, session, arch) — not to BeaconOutput, but to a separate Adaptix PACKAGE transport that the Process Browser consumes.

The three supporting BOFs (kill, suspend, resume) are 20-30 lines each: parse a PID integer argument, open a handle with the appropriate access right, call one Win32/NT function, close the handle, and report success or failure with `BeaconPrintf`.

**Primary recommendation:** Implement in order — bofdefs.h additions first (they unblock compilation of all four BOFs), then adaptix.h, then list.c (most complex), then kill.c (straightforward port), then suspend.c and resume.c (scratch implementations following kill.c pattern).

---

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Process enumeration (list) | BOF (in-process) | — | NtQuerySystemInformation runs in the beacon process context; no kernel driver or service needed |
| Process termination (kill) | BOF (in-process) | — | OpenProcess + TerminateProcess is a standard user-mode operation |
| Process suspend/resume | BOF (in-process) | — | NtSuspendProcess/NtResumeProcess are NT APIs callable from user mode with PROCESS_SUSPEND_RESUME access |
| Process Browser output routing | Adaptix runtime | BOF | BeaconPkgBytes/Int32 write to a UUID-tagged PACKAGE buffer; Adaptix routes it to the Process Browser UI — the BOF just packs; routing is Adaptix's job |
| Dynamic API resolution | BOF loader (Adaptix COFF loader) | — | The KERNEL32$/NTDLL$/ADVAPI32$ prefix pattern is resolved by the COFF loader at load time; never static-link |

---

## Standard Stack

### Core

| Library / Component | Version | Purpose | Why Standard |
|--------------------|---------|---------|--------------|
| `_include/bofdefs.h` | project-local | Dynamic Win32 API resolution via KERNEL32$/NTDLL$/ADVAPI32$/MSVCRT$ prefixes | Established pattern for all BOFs in this collection; COFF loader resolves these at load time |
| `_include/beacon.h` | project-local (CS 4.12 compatible) | BeaconDataParse, BeaconDataInt, BeaconPrintf, formatp API | Standard Cobalt Strike BOF API; provided by Adaptix COFF loader |
| `_include/adaptix.h` | NEW — to be created | BeaconPkgBytes, BeaconPkgInt32 declarations | Adaptix-specific PACKAGE transport; not in beacon.h; ApiTable[31] and ApiTable[34] confirmed in Kharon source |
| MinGW cross-compiler | x86_64-w64-mingw32-gcc / i686-w64-mingw32-gcc | Produce x64 and x86 COFF .o files | Established toolchain for this project; build flags `-Os -DBOF -c` fixed |

### Supporting Types (from MinGW system headers)

| Type / Constant | Header | Notes |
|-----------------|--------|-------|
| `SYSTEM_PROCESS_INFORMATION` | `winternl.h` (via bofdefs.h) | Fields: NextEntryOffset, NumberOfThreads, ImageName (UNICODE_STRING), UniqueProcessId (HANDLE), InheritedFromUniqueProcessId (HANDLE), SessionId — all needed by list.c |
| `HandleToUlong(h)` | `basetsd.h` (via windows.h) | Macro to cast HANDLE to DWORD/ULONG; used to pass UniqueProcessId to OpenProcess and BeaconPkgInt32 |
| `TOKEN_USER`, `TOKEN_INFORMATION_CLASS`, `TokenUser` | `winnt.h` (via windows.h) | Used in GetUserByToken to query token SID |
| `NT_SUCCESS(status)` | `winternl.h` | Macro: `((NTSTATUS)(status) >= 0)`. Replaces Kharon's lowercase `nt_success()` in the port |
| `STATUS_BUFFER_TOO_SMALL` | `winternl.h` | `0xC0000023` — expected first-call return from NtQueryInformationToken |
| `SystemProcessInformation` | `winternl.h` | Enum value for NtQuerySystemInformation class |

### Build Output Locations

| BOF | x64 output | x32 output |
|-----|-----------|-----------|
| list | `PS-BOF/_bin/list.x64.o` | `PS-BOF/_bin/list.x32.o` |
| kill | `PS-BOF/_bin/kill.x64.o` | `PS-BOF/_bin/kill.x32.o` |
| suspend | `PS-BOF/_bin/suspend.x64.o` | `PS-BOF/_bin/suspend.x32.o` |
| resume | `PS-BOF/_bin/resume.x64.o` | `PS-BOF/_bin/resume.x32.o` |

The Makefile is already correct and must NOT be modified. Build verifies as `[+]` per target.

---

## Architecture Patterns

### System Architecture Diagram

```
Operator                  Adaptix C2                  Beacon (Windows target)
   |                           |                              |
   |-- ps list command ------->|                              |
   |                           |-- execute BOF (list.x64.o)->|
   |                           |                         go() calls:
   |                           |                           NtQuerySystemInformation
   |                           |                           (loop per process):
   |                           |                             OpenProcess
   |                           |                             OpenProcessToken
   |                           |                             NtQueryInformationToken
   |                           |                             LookupAccountSidW
   |                           |                             IsWow64Process
   |                           |                             BeaconPkgBytes (name)
   |                           |                             BeaconPkgInt32 (PID)
   |                           |                             BeaconPkgInt32 (PPID)
   |                           |                             BeaconPkgInt32 (session)
   |                           |                             BeaconPkgBytes (user)
   |                           |                             BeaconPkgInt32 (arch)
   |                           |<-- PACKAGE response --------|
   |<-- Process Browser UI ----|
   |   (parsed by Adaptix)
```

Kill/suspend/resume are simpler:
```
go() -> BeaconDataParse -> BeaconDataInt(pid) -> OpenProcess -> NtSuspendProcess/TerminateProcess/NtResumeProcess -> CloseHandle -> BeaconPrintf
```

### Recommended Project Structure

No structural changes needed. Files being modified/created:

```
_include/
├── bofdefs.h           # ADD: KERNEL32/ADVAPI32/MSVCRT additions per D-09
├── beacon.h            # unchanged
└── adaptix.h           # CREATE: BeaconPkgBytes + BeaconPkgInt32

PS-BOF/
├── list/list.c         # REPLACE stub with full list.cc port
├── kill/kill.c         # REPLACE stub with kill.cc port
├── suspend/suspend.c   # REPLACE stub with scratch per D-08
└── resume/resume.c     # REPLACE stub with scratch per D-08
```

### Pattern 1: KERNEL32$/NTDLL$/ADVAPI32$ Dynamic Resolution

**What:** Every Win32 API call uses a `DLL$Function` prefix so the BOF COFF loader can resolve it at runtime. Never call Win32 APIs directly.

**When to use:** Always — every API call in every .c file.

**Example:**
```c
// Source: existing _include/bofdefs.h + established project pattern
WINBASEAPI BOOL WINAPI KERNEL32$CloseHandle(HANDLE hObject);
// In code:
KERNEL32$CloseHandle(handle);
// Never: CloseHandle(handle);
```

### Pattern 2: NtQuerySystemInformation Buffer Loop

**What:** First call with NULL buffer gets required size; allocate; second call fills buffer.

**When to use:** list.c only.

**Example (translated from list.cc to C):**
```c
// Source: ~/github/Kharon/agent_kharon/src_core/process/list.cc lines 135-147
PVOID base_sysproc = NULL;
ULONG return_length = 0;
NTSTATUS status;
SYSTEM_PROCESS_INFORMATION *system_proc_info = NULL;

NTDLL$NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &return_length);

system_proc_info = (SYSTEM_PROCESS_INFORMATION*)MSVCRT$malloc(return_length);
if (!system_proc_info) return;

status = NTDLL$NtQuerySystemInformation(SystemProcessInformation, system_proc_info, return_length, &return_length);
if (!NT_SUCCESS(status)) {
    BeaconPrintf(CALLBACK_ERROR, "Failed to get system process information, error: %d\n", KERNEL32$GetLastError());
    MSVCRT$free(system_proc_info);
    return;
}
base_sysproc = system_proc_info;
```

**Pitfall:** `nt_success()` in Kharon source is Kharon-specific. The C port uses `NT_SUCCESS()` from `winternl.h`.

### Pattern 3: SYSTEM_PROCESS_INFORMATION Traversal Loop

**What:** do/while with NextEntryOffset advancement; break when offset is zero.

**Example (translated from list.cc to C):**
```c
// Source: ~/github/Kharon/agent_kharon/src_core/process/list.cc lines 151-197
do {
    // ... process one entry ...

    if (system_proc_info->NextEntryOffset == 0)
        break;
    system_proc_info = (SYSTEM_PROCESS_INFORMATION*)((UINT_PTR)system_proc_info + system_proc_info->NextEntryOffset);
} while (1);

MSVCRT$free(base_sysproc);  // free base pointer, not the advanced pointer
```

**Critical:** Always keep `base_sysproc` pointing to the original allocation. Free `base_sysproc` after the loop.

### Pattern 4: GetUserByToken — goto cleanup in C

**What:** Translates Kharon's C++ cleanup lambda to C `goto cleanup`. Allocates three intermediate buffers (token_user_ptr, domain, username) that are always freed. Allocates user_domain that is freed only on failure path.

**Example (C translation of list.cc lines 7-95):**
```c
// Source: ~/github/Kharon/agent_kharon/src_core/process/list.cc GetUserByToken
static WCHAR* GetUserByToken(HANDLE token_handle) {
    TOKEN_USER *token_user_ptr = NULL;
    SID_NAME_USE sid_name = SidTypeUnknown;
    NTSTATUS status;
    WCHAR *user_domain = NULL;
    WCHAR *domain = NULL;
    WCHAR *username = NULL;
    ULONG total_len = 0, return_len = 0, domain_len = 0, username_ln = 0;
    BOOL success = FALSE;

    status = NTDLL$NtQueryInformationToken(token_handle, TokenUser, NULL, 0, &return_len);
    if (status != STATUS_BUFFER_TOO_SMALL)
        goto cleanup;

    token_user_ptr = (TOKEN_USER*)MSVCRT$malloc(return_len);
    if (!token_user_ptr) goto cleanup;

    status = NTDLL$NtQueryInformationToken(token_handle, TokenUser, token_user_ptr, return_len, &return_len);
    if (!NT_SUCCESS(status)) goto cleanup;

    ADVAPI32$LookupAccountSidW(NULL, token_user_ptr->User.Sid, NULL,
        &username_ln, NULL, &domain_len, &sid_name);
    if (KERNEL32$GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        goto cleanup;

    total_len = username_ln + domain_len + 2;
    user_domain = (WCHAR*)MSVCRT$malloc(total_len * sizeof(WCHAR));
    if (!user_domain) goto cleanup;

    domain   = (WCHAR*)MSVCRT$malloc(domain_len * sizeof(WCHAR));
    username = (WCHAR*)MSVCRT$malloc(username_ln * sizeof(WCHAR));
    if (!domain || !username) goto cleanup;

    success = ADVAPI32$LookupAccountSidW(NULL, token_user_ptr->User.Sid,
        username, &username_ln, domain, &domain_len, &sid_name);
    if (!success) goto cleanup;

    // Build "domain\user" manually — no swprintf/MSVCRT dependency needed
    // (see Pitfall 3 for rationale)
    ULONG di = 0, ui = 0;
    while (di < domain_len && domain[di]) user_domain[di] = domain[di++];
    user_domain[di++] = L'\\';
    while (ui < username_ln && username[ui]) user_domain[di++] = username[ui++];
    user_domain[di] = L'\0';

cleanup:
    if (token_user_ptr) MSVCRT$free(token_user_ptr);
    if (domain)         MSVCRT$free(domain);
    if (username)       MSVCRT$free(username);
    if (!success && user_domain) {
        MSVCRT$free(user_domain);
        user_domain = NULL;
    }
    return user_domain;
}
```

### Pattern 5: BeaconPkgBytes / BeaconPkgInt32 Output (PB-01 format)

**What:** Per-process output in the exact field order required by PB-01. Wide strings passed as raw bytes (no null terminator counted in length for BeaconPkgBytes).

**Example (translated from list.cc lines 162-189):**
```c
// Source: ~/github/Kharon/agent_kharon/src_core/process/list.cc go() loop body
// ImageName
if (system_proc_info->ImageName.Buffer) {
    BeaconPkgBytes((PBYTE)system_proc_info->ImageName.Buffer,
                   system_proc_info->ImageName.Length);
} else {
    BeaconPkgBytes((PBYTE)L"[System]", (ULONG)(wcslen(L"[System]") * sizeof(WCHAR)));
}
BeaconPkgInt32((INT32)HandleToUlong(system_proc_info->UniqueProcessId));
BeaconPkgInt32((INT32)HandleToUlong(system_proc_info->InheritedFromUniqueProcessId));
BeaconPkgInt32((INT32)system_proc_info->SessionId);

// user string
if (!user_token) {
    BeaconPkgBytes((PBYTE)L"N/A", (ULONG)(wcslen(L"N/A") * sizeof(WCHAR)));
} else {
    BeaconPkgBytes((PBYTE)user_token, (ULONG)(wcslen(user_token) * sizeof(WCHAR)));
    MSVCRT$free(user_token);
}
BeaconPkgInt32((INT32)Isx64);  // 1=x86 (WoW64), 0=x64 native
```

**ImageName.Length:** `UNICODE_STRING.Length` is in bytes, not characters — use it directly for BeaconPkgBytes (do NOT multiply by sizeof(WCHAR) again).

**Arch convention:** `IsWow64Process` returns TRUE for 32-bit processes on 64-bit Windows (WoW64). So arch int32 is `1` for x86, `0` for x64 native. This matches Kharon exactly — do not invert.

### Pattern 6: BeaconDataInt for Argument Parsing (kill.c)

**What:** Two sequential BeaconDataInt calls extract PID and optional exit_code.

**Example (translated from kill.cc):**
```c
// Source: ~/github/Kharon/agent_kharon/src_core/process/kill.cc lines 4-9
datap data_parser = {0};
BeaconDataParse(&data_parser, args, len);
INT32 process_id       = BeaconDataInt(&data_parser);
INT32 process_exitcode = BeaconDataInt(&data_parser);
```

If only PID is provided (no exit_code argument), `BeaconDataInt` returns 0 for the second call — terminating with exit code 0 is correct behavior.

### Pattern 7: suspend.c / resume.c (D-08 scratch pattern)

**What:** Minimal BOF following kill.c pattern but calling NT suspend/resume APIs.

**Example (suspend.c — ~20 lines):**
```c
// Source: D-08 decision + established kill.c pattern + existing bofdefs.h NtSuspendProcess
#include "bofdefs.h"
#include "beacon.h"

void go(char *args, int len) {
    datap data_parser = {0};
    BeaconDataParse(&data_parser, args, len);
    INT32 pid = BeaconDataInt(&data_parser);

    HANDLE h = KERNEL32$OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, (DWORD)pid);
    if (!h) {
        BeaconPrintf(CALLBACK_ERROR, "OpenProcess failed: %d\n", KERNEL32$GetLastError());
        return;
    }

    NTSTATUS status = NTDLL$NtSuspendProcess(h);
    KERNEL32$CloseHandle(h);

    if (!NT_SUCCESS(status)) {
        BeaconPrintf(CALLBACK_ERROR, "NtSuspendProcess failed: 0x%08x\n", status);
        return;
    }
    BeaconPrintf(CALLBACK_OUTPUT, "Process suspended\n");
}
```

resume.c is identical except `NtResumeProcess` and message text.

### Anti-Patterns to Avoid

- **Direct Win32 calls:** Never `OpenProcess(...)` — always `KERNEL32$OpenProcess(...)`.
- **Using nt_success():** This is a Kharon internal macro. Use `NT_SUCCESS()` from winternl.h.
- **Using BeaconPrintfW:** Not declared in beacon.h or adaptix.h; not needed (D-03).
- **Using BeaconFormatAppend/BeaconOutput for process list:** Routes to text output, not Process Browser PACKAGE.
- **Freeing base_sysproc's advanced pointer:** The traversal advances `system_proc_info`; keep `base_sysproc` for free().
- **Multiplying ImageName.Length by sizeof(WCHAR):** `UNICODE_STRING.Length` is already in bytes.
- **Using calloc for the process list buffer:** `MSVCRT$calloc` exists but `MSVCRT$malloc` needs to be added to bofdefs.h (see Don't Hand-Roll).

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Process enumeration | Custom VirtualQuery/ReadProcessMemory loops | NtQuerySystemInformation(SystemProcessInformation) | Single-call kernel snapshot; safe; what Kharon uses |
| Token → username resolution | Manual SID string parsing | NtQueryInformationToken + LookupAccountSidW | OS resolves domain/user name correctly; handles domain-joined machines |
| Wide string formatting (domain\user) | MSVCRT$swprintf declaration | Manual copy loop (wcsncpy-style index walk) | MSVCRT$swprintf is not declared; `swprintf` in MinGW links against __mingw_swprintf not MSVCRT.dll; index-walking is safe and has zero extra dependencies |
| Process list output | BeaconOutput text table | BeaconPkgBytes + BeaconPkgInt32 | Process Browser parses binary PACKAGE; text output goes to wrong channel |

---

## Critical Infrastructure Gap: MSVCRT$malloc Not Declared

**This is the single most important gap blocking list.c compilation.**

Current `_include/bofdefs.h` has `MSVCRT$calloc` and `MSVCRT$free` but NOT `MSVCRT$malloc`. The Kharon source uses `malloc()` throughout. The port must use `MSVCRT$malloc`.

**Required addition to bofdefs.h (new line alongside existing MSVCRT block):**
```c
WINBASEAPI void *__cdecl MSVCRT$malloc(size_t _Size);
```

This must be added as part of the D-09 task (bofdefs.h additions), alongside the KERNEL32/ADVAPI32 additions. If omitted, list.c (and any code calling GetUserByToken) will fail to link.

---

## Common Pitfalls

### Pitfall 1: STATUS_INFO_LENGTH_MISMATCH vs STATUS_BUFFER_TOO_SMALL (NtQuerySystemInformation)

**What goes wrong:** Code checks for `STATUS_BUFFER_TOO_SMALL` after the sizing call to `NtQuerySystemInformation`. That API returns `STATUS_INFO_LENGTH_MISMATCH` (0xC0000004), not `STATUS_BUFFER_TOO_SMALL` (0xC0000023), when the buffer is too small.

**Why it happens:** Kharon's list.cc does NOT check the return code of the first NtQuerySystemInformation call at all — it just reads `return_length`. This is the correct pattern.

**How to avoid:** Do not check the return status of the first (sizing) NtQuerySystemInformation call. Only check the second (filling) call with `NT_SUCCESS()`. The first call always returns an error code but populates `return_length` correctly.

**Warning signs:** If the second call fails, `return_length` after the first call was zero — meaning the buffer allocation was zero bytes and the second call received an undersized buffer.

### Pitfall 2: Wide String Length in BeaconPkgBytes

**What goes wrong:** Passing `wcslen(buf) * sizeof(WCHAR)` when the buffer length is already `UNICODE_STRING.Length` (which is in bytes), resulting in double-counting.

**Why it happens:** `wcslen` returns character count; `UNICODE_STRING.Length` is already byte count. The two code paths are different:
- `ImageName.Buffer` comes from `SYSTEM_PROCESS_INFORMATION` → use `ImageName.Length` directly (bytes).
- String literals like `L"[System]"` or `L"N/A"` → use `wcslen(...) * sizeof(WCHAR)`.
- `user_token` from GetUserByToken → use `wcslen(user_token) * sizeof(WCHAR)`.

**How to avoid:** Check which path each BeaconPkgBytes call takes.

### Pitfall 3: swprintf/wide string formatting in BOF context

**What goes wrong:** Calling `_swprintf` or `swprintf` directly generates a reference to `__mingw_swprintf` (not MSVCRT.dll) or requires CRT initialization. Either fails in a BOF context.

**Why it happens:** MinGW's `swprintf` is not a thin MSVCRT.dll wrapper the way `strlen` is; it routes through MinGW's own implementation.

**How to avoid:** Build `domain\user` with a manual index walk (see Pattern 4 code). This avoids any CRT dependency and handles the concatenation correctly.

### Pitfall 4: Freeing the Advanced Pointer Instead of the Base

**What goes wrong:** After the NtQuerySystemInformation loop advances `system_proc_info` through the list, calling `MSVCRT$free(system_proc_info)` at the end frees the wrong pointer (middle of the allocation).

**How to avoid:** Save `base_sysproc = system_proc_info` before the loop starts. Free `base_sysproc` after the loop.

### Pitfall 5: OpenProcess Access Rights for Suspend/Resume

**What goes wrong:** Using `PROCESS_QUERY_INFORMATION` or `PROCESS_ALL_ACCESS` instead of `PROCESS_SUSPEND_RESUME`.

**Why it happens:** `NtSuspendProcess` and `NtResumeProcess` require `PROCESS_SUSPEND_RESUME` (0x0800). `PROCESS_QUERY_INFORMATION` does not grant this right and will cause a STATUS_ACCESS_DENIED error from the NT call even if OpenProcess succeeds.

**How to avoid:** `OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid)` explicitly. Do not use `PROCESS_ALL_ACCESS` (excessive privilege).

### Pitfall 6: BeaconDataParse Not Called in list.c

**What goes wrong:** list.c takes no arguments, so there is no BeaconDataParse call. If accidentally added (copying from kill.c), it's dead code at best and confusing at worst.

**How to avoid:** list.c's `go()` ignores the `args`/`len` parameters entirely — no parsing needed.

### Pitfall 7: stubs include `"bofdefs.h"` not `<bofdefs.h>`

**What goes wrong:** The stubs use `#include "bofdefs.h"`. With `-I ../_include`, this resolves to `_include/bofdefs.h`. Adding `#include "beacon.h"` in the same way also resolves correctly. Do NOT change these to angle-bracket includes — they work as-is.

**How to avoid:** Keep existing include style from the stubs.

---

## Code Examples

### adaptix.h — new file to create

```c
// Source: Kharon beacon.h lines 320-323 (C++ default removed per D-01)
// ApiTable[31] = BeaconPkgBytes, ApiTable[34] = BeaconPkgInt32
// (confirmed in Kharon src_beacon/Include/Kharon.h lines 1091, 1094)
#pragma once

DECLSPEC_IMPORT VOID BeaconPkgBytes(PBYTE Buffer, ULONG Length, PCHAR UUID);
DECLSPEC_IMPORT VOID BeaconPkgInt32(INT32 Data, PCHAR UUID);
```

**Caller always passes NULL for UUID in this phase:**
```c
BeaconPkgBytes((PBYTE)buffer, length, NULL);
BeaconPkgInt32(pid, NULL);
```

### bofdefs.h additions (D-09 + MSVCRT$malloc gap)

New sections to add after the existing NTDLL PS-BOF section:

```c
// Source: D-09 decision (CONTEXT.md) + verified against MinGW header signatures
// =============================================================================
// KERNEL32 — process management (PS-BOF)
// =============================================================================
WINBASEAPI HANDLE WINAPI KERNEL32$OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
WINBASEAPI BOOL WINAPI KERNEL32$TerminateProcess(HANDLE hProcess, UINT uExitCode);
WINBASEAPI BOOL WINAPI KERNEL32$IsWow64Process(HANDLE hProcess, PBOOL Wow64Process);
WINBASEAPI HANDLE WINAPI KERNEL32$GetCurrentProcess(VOID);

// =============================================================================
// ADVAPI32 — token / privilege (PS-BOF)
// =============================================================================
WINADVAPI BOOL WINAPI ADVAPI32$OpenProcessToken(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
WINADVAPI BOOL WINAPI ADVAPI32$LookupAccountSidW(LPCWSTR lpSystemName, PSID Sid, LPWSTR Name, LPDWORD cchName, LPWSTR ReferencedDomainName, LPDWORD cchReferencedDomainName, PSID_NAME_USE peUse);
WINADVAPI BOOL WINAPI ADVAPI32$AdjustTokenPrivileges(HANDLE TokenHandle, BOOL DisableAllPrivileges, PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState, PDWORD ReturnLength);
WINADVAPI BOOL WINAPI ADVAPI32$LookupPrivilegeValueW(LPCWSTR lpSystemName, LPCWSTR lpName, PLUID lpLuid);

// =============================================================================
// NTDLL — token query (PS-BOF)
// =============================================================================
WINBASEAPI NTSTATUS NTAPI NTDLL$NtQueryInformationToken(HANDLE TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass, PVOID TokenInformation, ULONG TokenInformationLength, PULONG ReturnLength);
```

And in the existing MSVCRT section, add:
```c
WINBASEAPI void *__cdecl MSVCRT$malloc(size_t _Size);
```

---

## Existing bofdefs.h State vs D-09 Requirements

| Declaration | Already Present | Needs Adding |
|-------------|----------------|-------------|
| `NTDLL$NtQuerySystemInformation` | YES (line 87) | — |
| `NTDLL$NtSuspendProcess` | YES (line 88) | — |
| `NTDLL$NtResumeProcess` | YES (line 89) | — |
| `KERNEL32$CloseHandle` | YES (line 45) | — |
| `KERNEL32$GetLastError` | YES (line 36) | — |
| `KERNEL32$OpenProcess` | NO | Add |
| `KERNEL32$TerminateProcess` | NO | Add |
| `KERNEL32$IsWow64Process` | NO | Add |
| `KERNEL32$GetCurrentProcess` | NO | Add |
| `ADVAPI32$OpenProcessToken` | NO | Add |
| `ADVAPI32$LookupAccountSidW` | NO | Add |
| `ADVAPI32$AdjustTokenPrivileges` | NO | Add |
| `ADVAPI32$LookupPrivilegeValueW` | NO | Add |
| `NTDLL$NtQueryInformationToken` | NO | Add |
| `MSVCRT$calloc` | YES (line 73) | — |
| `MSVCRT$free` | YES (line 74) | — |
| `MSVCRT$malloc` | NO | Add (unlisted in D-09 but required) |

**Total new declarations: 10** (9 from D-09 + MSVCRT$malloc)

---

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| `nt_success()` Kharon macro | `NT_SUCCESS()` from winternl.h | Always been different — Kharon defines its own | Use `NT_SUCCESS()` in the C port |
| `BeaconPrintfW` (Kharon C++) | `BeaconPrintf` (narrow, C) | D-03 decision | Error strings must be narrow ASCII |
| C++ lambda cleanup | `goto cleanup` | D-07 decision | Standard C multi-resource cleanup |
| Kharon's `malloc`/`free` | `MSVCRT$malloc` / `MSVCRT$free` | BOF pattern — always required | Must prefix all CRT calls |

---

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | `BeaconPkgBytes` and `BeaconPkgInt32` signatures with `PCHAR UUID` third parameter — C callers pass NULL | adaptix.h code example | If Adaptix's COFF loader passes UUID by another mechanism or the parameter type differs, adaptix.h signature needs adjustment |
| A2 | Manual index-walk to build `domain\user` is safe without any MSVCRT dependency | Pattern 4 / Pitfall 3 | If the loop has an off-by-one with multi-byte Unicode (surrogate pairs), user names with unusual characters could be truncated; for standard domain\user this is not a realistic concern |

**All other claims verified against:** Kharon source files (read directly), MinGW system headers (grepped directly), existing bofdefs.h and beacon.h (read directly), Kharon Kharon.h ApiTable (read directly).

---

## Open Questions

1. **adaptix.h placement for list.c include**
   - What we know: list.c includes `"bofdefs.h"`. The Makefile has `-I ../_include -I _include`. `_include/adaptix.h` (new file) will be reachable as `"adaptix.h"` from any BOF source.
   - What's unclear: Whether to include `adaptix.h` from within `bofdefs.h` (so all BOFs get it automatically) or include it separately in list.c only.
   - Recommendation: Include `adaptix.h` separately in list.c (not from bofdefs.h) — only list.c uses BeaconPkgBytes/Int32 in this phase; keeping it separate follows the principle of minimum footprint and makes it explicit which BOFs use the Adaptix PACKAGE transport.

2. **AdjustTokenPrivileges / LookupPrivilegeValueW usage**
   - What we know: D-09 lists these declarations as needed. D-05 says do not call EnableDebugPrivilege from go(). The declarations are needed because the function body exists in the reference code.
   - What's unclear: Whether to include the `EnableDebugPrivilege` function body at all (it is never called from go()).
   - Recommendation: Omit EnableDebugPrivilege entirely (D-05 explicitly says skip it, not just skip the call). If declarations for AdjustTokenPrivileges and LookupPrivilegeValueW are in bofdefs.h, they do no harm — they may be used by future phases.

---

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| x86_64-w64-mingw32-gcc | list/kill/suspend/resume x64 builds | Assumed yes (Phase 22 built successfully) | — | — |
| i686-w64-mingw32-gcc | list/kill/suspend/resume x32 builds | Assumed yes (Phase 22 built successfully) | — | — |
| x86_64-w64-mingw32-strip | Strip symbols from x64 .o | Assumed yes | — | — |
| i686-w64-mingw32-strip | Strip symbols from x32 .o | Assumed yes | — | — |

No new external dependencies introduced in this phase.

---

## Validation Architecture

### Test Framework

| Property | Value |
|----------|-------|
| Framework | make (Makefile-driven; [+]/[!] output pattern) |
| Config file | `PS-BOF/Makefile` (already exists) |
| Quick run command | `make -C PS-BOF 2>&1 \| grep -E '^\[.\]'` |
| Full suite command | `make -C PS-BOF 2>&1` |

### Phase Requirements → Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| PS-01 | ps list compiles and produces .o | build/compile | `make -C PS-BOF 2>&1 \| grep list` | N/A — source not yet written |
| PS-02 | ps kill compiles and produces .o | build/compile | `make -C PS-BOF 2>&1 \| grep kill` | N/A |
| PS-08 | ps suspend compiles and produces .o | build/compile | `make -C PS-BOF 2>&1 \| grep suspend` | N/A |
| PS-09 | ps resume compiles and produces .o | build/compile | `make -C PS-BOF 2>&1 \| grep resume` | N/A |
| PB-01 | list.c uses BeaconPkgBytes/Int32 not BeaconOutput | code review / grep | `grep -c 'BeaconPkgBytes\|BeaconPkgInt32' PS-BOF/list/list.c` | N/A |

Runtime behavior (actual process listing, killing, suspending) is deferred to Phase 28 (CI/CD tests requiring a live Windows beacon).

### Sampling Rate

- **Per task commit:** `make -C PS-BOF 2>&1 | grep -E '^\[.\]'` — all 8 targets must show `[+]`
- **Per wave merge:** full `make -C PS-BOF` — no `[!]` lines
- **Phase gate:** All 8 `[+]` lines before closing phase

### Wave 0 Gaps

- [ ] `_include/adaptix.h` — does not exist yet; Wave 0 must create it
- [ ] `_include/bofdefs.h` additions — 10 new declarations must be added before any BOF source compiles
- [ ] No test framework install needed (make already present)

---

## Security Domain

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | No | — |
| V3 Session Management | No | — |
| V4 Access Control | Yes (implicit) | OpenProcess uses minimum required access (PROCESS_TERMINATE, PROCESS_SUSPEND_RESUME, PROCESS_QUERY_LIMITED_INFORMATION) — never PROCESS_ALL_ACCESS |
| V5 Input Validation | Yes | PID argument is parsed with BeaconDataInt (Beacon-provided, not hand-rolled); no string parsing done on operator input |
| V6 Cryptography | No | — |

### Known Threat Patterns for BOF (process manipulation)

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| Excessive process handle access | Elevation of Privilege | Request minimum access: PROCESS_QUERY_LIMITED_INFORMATION for list, PROCESS_TERMINATE for kill, PROCESS_SUSPEND_RESUME for suspend/resume |
| Token privilege escalation | Elevation of Privilege | EnableDebugPrivilege omitted per D-05 — no SeDebugPrivilege requested |
| Operator-controlled PID used as handle | Tampering | Handle validated by OpenProcess return value; if NULL, BOF returns error before calling destructive API |

---

## Sources

### Primary (HIGH confidence)

- `~/github/Kharon/agent_kharon/src_core/process/list.cc` — read directly; source of truth for NtQuerySystemInformation loop, GetUserByToken, IsWow64Process, BeaconPkgBytes/Int32 call pattern
- `~/github/Kharon/agent_kharon/src_core/process/kill.cc` — read directly; source of truth for kill.c port
- `~/github/Kharon/agent_kharon/src_beacon/Include/Kharon.h` — read directly; ApiTable[31]=BeaconPkgBytes, ApiTable[34]=BeaconPkgInt32 confirmed
- `~/github/Kharon/agent_kharon/src_core/include/beacon.h` — read directly; BeaconPkgBytes/Int32 C++ signatures confirmed (PCHAR UUID default param)
- `/home/tgj/github/BOF-Collection/_include/bofdefs.h` — read directly; exact current state of declarations verified
- `/home/tgj/github/BOF-Collection/_include/beacon.h` — read directly; BeaconDataParse, BeaconDataInt, BeaconPrintf signatures verified
- `/usr/x86_64-w64-mingw32/include/winternl.h` — grepped directly; SYSTEM_PROCESS_INFORMATION fields, NT_SUCCESS, STATUS_INFO_LENGTH_MISMATCH, STATUS_BUFFER_TOO_SMALL confirmed
- `/usr/x86_64-w64-mingw32/include/winnt.h` — grepped directly; TOKEN_USER, TOKEN_INFORMATION_CLASS, TokenUser confirmed
- `/usr/x86_64-w64-mingw32/include/basetsd.h` — grepped directly; HandleToUlong macro confirmed

### Secondary (MEDIUM confidence)

- CONTEXT.md decisions D-01 through D-09 — authored by user in discuss-phase session; treated as locked

### Tertiary (LOW confidence)

- None

---

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — verified from actual source files and MinGW headers
- Architecture: HIGH — Kharon source read directly; data flow is explicit in list.cc
- Pitfalls: HIGH — verified against actual header definitions and MinGW implementation
- bofdefs.h gap (MSVCRT$malloc): HIGH — grepped bofdefs.h directly; absence confirmed

**Research date:** 2026-05-16
**Valid until:** 2026-06-16 (stable domain — Win32 API + established BOF patterns)
