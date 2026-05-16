# Phase 23: Core Process BOFs - Pattern Map

**Mapped:** 2026-05-16
**Files analyzed:** 6 (2 new, 4 modified)
**Analogs found:** 6 / 6

---

## File Classification

| New/Modified File | Role | Data Flow | Closest Analog | Match Quality |
|---|---|---|---|---|
| `_include/adaptix.h` | config/header | request-response | `_include/beacon.h` | role-match |
| `_include/bofdefs.h` | config/header | — | `_include/bofdefs.h` itself (existing sections) | exact — extend in place |
| `PS-BOF/list/list.c` | BOF entrypoint | batch + PACKAGE transport | `Exit-BOF/exitprocess/exitprocess.c` (structure); RESEARCH.md Pattern 2–5 (logic) | role-match |
| `PS-BOF/kill/kill.c` | BOF entrypoint | request-response | `Exit-BOF/exitprocess/exitprocess.c` | role-match |
| `PS-BOF/suspend/suspend.c` | BOF entrypoint | request-response | `Exit-BOF/exitprocess/exitprocess.c` | role-match |
| `PS-BOF/resume/resume.c` | BOF entrypoint | request-response | `Exit-BOF/exitprocess/exitprocess.c` | role-match |

---

## Pattern Assignments

### `_include/adaptix.h` (new header, Adaptix PACKAGE transport declarations)

**Analog:** `_include/beacon.h` — same role (shared BOF API header), same `DECLSPEC_IMPORT` declaration style.

**Header guard pattern** (`_include/beacon.h` lines 30–32):
```c
#ifndef _BEACON_H_
#define _BEACON_H_
#include <windows.h>
```
For adaptix.h use `#pragma once` (matches bofdefs.h line 1 style — simpler, already used project-wide).

**DECLSPEC_IMPORT declaration pattern** (`_include/beacon.h` lines 46–51):
```c
DECLSPEC_IMPORT void    BeaconDataParse(datap * parser, char * buffer, int size);
DECLSPEC_IMPORT int     BeaconDataInt(datap * parser);
DECLSPEC_IMPORT void   BeaconOutput(int type, const char * data, int len);
DECLSPEC_IMPORT void   BeaconPrintf(int type, const char * fmt, ...);
```
Copy this style exactly. `BeaconPkgBytes` and `BeaconPkgInt32` are `VOID` return, not `void *`.

**Target adaptix.h** (derived from RESEARCH.md Code Examples + beacon.h style):
```c
#pragma once

DECLSPEC_IMPORT VOID BeaconPkgBytes(PBYTE Buffer, ULONG Length, PCHAR UUID);
DECLSPEC_IMPORT VOID BeaconPkgInt32(INT32 Data, PCHAR UUID);
```
No `#include <windows.h>` needed — list.c already includes it via `bofdefs.h`. No `extern "C"` wrapper needed — this project uses BOF-only C compilation.

**Caller convention** (UUID is always NULL in this phase):
```c
BeaconPkgBytes((PBYTE)system_proc_info->ImageName.Buffer,
               system_proc_info->ImageName.Length, NULL);
BeaconPkgInt32((INT32)HandleToUlong(system_proc_info->UniqueProcessId), NULL);
```

---

### `_include/bofdefs.h` (modify — add 10 declarations, D-09 + MSVCRT$malloc)

**Analog:** Existing sections within `_include/bofdefs.h` itself.

**Section header style** (`_include/bofdefs.h` lines 19–25, 70–78):
```c
// =============================================================================
// KERNEL32 — memory management
// =============================================================================
WINBASEAPI void * WINAPI KERNEL32$HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
```
and
```c
// =============================================================================
// NTDLL (Exit BOFs only)
// =============================================================================
WINBASEAPI VOID NTAPI NTDLL$RtlExitUserProcess(NTSTATUS Status);
```

**KERNEL32 declaration style** (`_include/bofdefs.h` lines 43–45):
```c
WINBASEAPI HANDLE WINAPI KERNEL32$CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
WINBASEAPI WINBOOL WINAPI KERNEL32$ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
WINBASEAPI WINBOOL WINAPI KERNEL32$CloseHandle(HANDLE hObject);
```
Pattern: `WINBASEAPI <return> WINAPI KERNEL32$<FuncName>(<params>);`

**ADVAPI32 declaration style** — no existing ADVAPI32 section; model after KERNEL32 but use `WINADVAPI`:
```c
WINADVAPI BOOL WINAPI ADVAPI32$OpenProcessToken(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
```
Pattern: `WINADVAPI <return> WINAPI ADVAPI32$<FuncName>(<params>);`

**NTDLL declaration style** (`_include/bofdefs.h` lines 87–89):
```c
WINBASEAPI NTSTATUS NTAPI NTDLL$NtQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
WINBASEAPI NTSTATUS NTAPI NTDLL$NtSuspendProcess(HANDLE ProcessHandle);
WINBASEAPI NTSTATUS NTAPI NTDLL$NtResumeProcess(HANDLE ProcessHandle);
```
Pattern: `WINBASEAPI NTSTATUS NTAPI NTDLL$<FuncName>(<params>);`

**MSVCRT declaration style** (`_include/bofdefs.h` lines 73–76):
```c
WINBASEAPI void *__cdecl MSVCRT$calloc(size_t _NumOfElements, size_t _SizeOfElements);
WINBASEAPI void __cdecl MSVCRT$free(void *_Memory);
```
New line to add alongside these:
```c
WINBASEAPI void *__cdecl MSVCRT$malloc(size_t _Size);
```

**Insertion point for new sections:** After line 89 (after the existing NTDLL PS-BOF section), before line 91 (PSAPI section). Insert three new sections: KERNEL32 process management, ADVAPI32 token/privilege, NTDLL token query. Add `MSVCRT$malloc` after line 73 (`MSVCRT$calloc`), keeping the MSVCRT block contiguous.

**Complete new declarations to add** (from RESEARCH.md Code Examples, verified against MinGW headers):
```c
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

---

### `PS-BOF/list/list.c` (replace stub — NtQuerySystemInformation loop + BeaconPkg output)

**Analog:** `Exit-BOF/exitprocess/exitprocess.c` (BOF structure); `_include/beacon.h` (API); RESEARCH.md Patterns 2–5 (logic — no closer codebase analog exists).

**BOF file structure** (`Exit-BOF/exitprocess/exitprocess.c` lines 1–7):
```c
#include <windows.h>
#include "bofdefs.h"
#include "beacon.h"

void go(char* buff, int len) {
    NTDLL$RtlExitUserProcess(0);
}
```
list.c extends this: add `#include "adaptix.h"` after `beacon.h`. Function signature stays `void go(char *args, int len)`. No `BeaconDataParse` — list takes no arguments (Pitfall 6).

**Imports pattern** (copy from exitprocess.c, extend):
```c
#include <windows.h>
#include "bofdefs.h"
#include "beacon.h"
#include "adaptix.h"
```

**NtQuerySystemInformation sizing + allocation** (RESEARCH.md Pattern 2):
```c
ULONG return_length = 0;
NTSTATUS status;
SYSTEM_PROCESS_INFORMATION *system_proc_info = NULL;
PVOID base_sysproc = NULL;

// First call: do NOT check return status — always fails but sets return_length
NTDLL$NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &return_length);

system_proc_info = (SYSTEM_PROCESS_INFORMATION*)MSVCRT$malloc(return_length);
if (!system_proc_info) return;

status = NTDLL$NtQuerySystemInformation(SystemProcessInformation,
    system_proc_info, return_length, &return_length);
if (!NT_SUCCESS(status)) {
    BeaconPrintf(CALLBACK_ERROR, "Failed to get system process information, error: %d\n",
                 KERNEL32$GetLastError());
    MSVCRT$free(system_proc_info);
    return;
}
base_sysproc = system_proc_info;
```

**do/while traversal loop** (RESEARCH.md Pattern 3):
```c
do {
    // ... process one entry ...
    if (system_proc_info->NextEntryOffset == 0)
        break;
    system_proc_info = (SYSTEM_PROCESS_INFORMATION*)
        ((UINT_PTR)system_proc_info + system_proc_info->NextEntryOffset);
} while (1);

MSVCRT$free(base_sysproc);  // ALWAYS free base pointer, not the advanced pointer
```

**GetUserByToken helper — goto cleanup pattern** (RESEARCH.md Pattern 4 — D-07):
```c
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

    status = NTDLL$NtQueryInformationToken(token_handle, TokenUser,
        token_user_ptr, return_len, &return_len);
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

    // Manual domain\user concatenation — do NOT use swprintf (Pitfall 3)
    {
        ULONG di = 0, ui = 0;
        while (di < domain_len && domain[di]) user_domain[di] = domain[di++];
        user_domain[di++] = L'\\';
        while (ui < username_ln && username[ui]) user_domain[di++] = username[ui++];
        user_domain[di] = L'\0';
    }

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

**BeaconPkg output per process** (RESEARCH.md Pattern 5 — PB-01 field order: name, PID, PPID, session, user, arch):
```c
// ImageName (wstr bytes — ImageName.Length is ALREADY in bytes, do not multiply)
if (system_proc_info->ImageName.Buffer) {
    BeaconPkgBytes((PBYTE)system_proc_info->ImageName.Buffer,
                   system_proc_info->ImageName.Length, NULL);
} else {
    BeaconPkgBytes((PBYTE)L"[System]",
                   (ULONG)(wcslen(L"[System]") * sizeof(WCHAR)), NULL);
}
BeaconPkgInt32((INT32)HandleToUlong(system_proc_info->UniqueProcessId), NULL);
BeaconPkgInt32((INT32)HandleToUlong(system_proc_info->InheritedFromUniqueProcessId), NULL);
BeaconPkgInt32((INT32)system_proc_info->SessionId, NULL);

// user (wstr bytes — wcslen * sizeof because it comes from GetUserByToken malloc)
if (!user_token) {
    BeaconPkgBytes((PBYTE)L"N/A",
                   (ULONG)(wcslen(L"N/A") * sizeof(WCHAR)), NULL);
} else {
    BeaconPkgBytes((PBYTE)user_token,
                   (ULONG)(wcslen(user_token) * sizeof(WCHAR)), NULL);
    MSVCRT$free(user_token);
}
BeaconPkgInt32((INT32)Isx64, NULL);  // 1=x86/WoW64, 0=x64 native
```

**IsWow64Process call** (produces Isx64 used above):
```c
BOOL Isx64 = 0;
HANDLE proc_handle = KERNEL32$OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
    FALSE, (DWORD)HandleToUlong(system_proc_info->UniqueProcessId));
if (proc_handle) {
    KERNEL32$IsWow64Process(proc_handle, &Isx64);
    // also call OpenProcessToken here for user lookup
    KERNEL32$CloseHandle(proc_handle);
}
```

---

### `PS-BOF/kill/kill.c` (replace stub — BeaconDataInt parse + TerminateProcess)

**Analog:** `Exit-BOF/exitprocess/exitprocess.c` (structure); RESEARCH.md Pattern 6 (argument parsing).

**Imports pattern** (exitprocess.c lines 1–4 — omit adaptix.h, kill uses BeaconPrintf only):
```c
#include <windows.h>
#include "bofdefs.h"
#include "beacon.h"
```

**Argument parsing** (RESEARCH.md Pattern 6):
```c
datap data_parser = {0};
BeaconDataParse(&data_parser, args, len);
INT32 process_id       = BeaconDataInt(&data_parser);
INT32 process_exitcode = BeaconDataInt(&data_parser);
```
Second `BeaconDataInt` returns 0 if no exit_code argument was packed — this is correct (exit code 0).

**Core pattern** (port from kill.cc, D-06 error style):
```c
HANDLE h = KERNEL32$OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)process_id);
if (!h) {
    BeaconPrintf(CALLBACK_ERROR, "OpenProcess failed: %d\n", KERNEL32$GetLastError());
    return;
}

BOOL ok = KERNEL32$TerminateProcess(h, (UINT)process_exitcode);
KERNEL32$CloseHandle(h);

if (!ok) {
    BeaconPrintf(CALLBACK_ERROR, "TerminateProcess failed: %d\n", KERNEL32$GetLastError());
    return;
}
BeaconPrintf(CALLBACK_OUTPUT, "Process killed\n");
```

**Error output style** (matches FS-BOF/Exit-BOF — D-06):
```c
BeaconPrintf(CALLBACK_ERROR, "OpenProcess failed: %d\n", KERNEL32$GetLastError());
BeaconPrintf(CALLBACK_OUTPUT, "Process killed\n");
```
Narrow strings only. No wide variants. No `BeaconOutput` (text-channel only, not needed for simple status).

---

### `PS-BOF/suspend/suspend.c` (new from scratch — D-08 pattern)

**Analog:** `Exit-BOF/exitprocess/exitprocess.c` (structure); kill.c pattern (argument parsing + handle lifecycle).

**Complete implementation** (RESEARCH.md Pattern 7):
```c
#include <windows.h>
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

**Access right:** Must be `PROCESS_SUSPEND_RESUME` (0x0800). Do NOT use `PROCESS_ALL_ACCESS` or `PROCESS_QUERY_INFORMATION` — NtSuspendProcess requires exactly this right (Pitfall 5).

---

### `PS-BOF/resume/resume.c` (new from scratch — D-08 pattern, mirror of suspend.c)

**Analog:** `PS-BOF/suspend/suspend.c` (identical structure, swap one call and message).

**Complete implementation** (RESEARCH.md Pattern 7):
```c
#include <windows.h>
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

    NTSTATUS status = NTDLL$NtResumeProcess(h);
    KERNEL32$CloseHandle(h);

    if (!NT_SUCCESS(status)) {
        BeaconPrintf(CALLBACK_ERROR, "NtResumeProcess failed: 0x%08x\n", status);
        return;
    }
    BeaconPrintf(CALLBACK_OUTPUT, "Process resumed\n");
}
```

---

## Shared Patterns

### Dynamic API Resolution (`DLL$Function` prefix)
**Source:** `_include/bofdefs.h` throughout; all existing BOF `.c` files
**Apply to:** All four `.c` files, all API calls without exception

Pattern: every Win32/NT/CRT call uses the module-prefix form.
```c
// CORRECT
KERNEL32$CloseHandle(h);
NTDLL$NtSuspendProcess(h);
MSVCRT$malloc(return_length);

// NEVER — direct call bypasses COFF loader resolution
CloseHandle(h);
NtSuspendProcess(h);
malloc(return_length);
```

### Error Output Style (D-06)
**Source:** `_include/beacon.h` lines 70–79; established by FS-BOF and Exit-BOF
**Apply to:** kill.c, suspend.c, resume.c (list.c has no error output to operator; it uses BeaconPrintf only for NtQuerySystemInformation failure)

```c
// Error path
BeaconPrintf(CALLBACK_ERROR, "OpenProcess failed: %d\n", KERNEL32$GetLastError());
// Success path
BeaconPrintf(CALLBACK_OUTPUT, "Process killed\n");
```
Narrow `const char*` format strings only. No `BeaconPrintfW` (not declared, D-03).

### Include Order
**Source:** `Exit-BOF/exitprocess/exitprocess.c` lines 1–3; stub files line 1
**Apply to:** All four `.c` files

```c
#include <windows.h>   // system types — always first
#include "bofdefs.h"   // DLL$ declarations — always second (provides winternl.h, psapi.h too)
#include "beacon.h"    // BeaconPrintf, BeaconDataParse, datap — third
// adaptix.h — fourth, list.c only
```
Keep `"bofdefs.h"` in double-quote form (not angle brackets) — the stubs already use this form; `-I ../_include` resolves it correctly (Pitfall 7).

### Handle Lifecycle
**Source:** `Exit-BOF/exitprocess/exitprocess.c` (single-call pattern); `_include/bofdefs.h` line 45 (`KERNEL32$CloseHandle`)
**Apply to:** kill.c, suspend.c, resume.c

Always close handle before returning on either success or failure path:
```c
HANDLE h = KERNEL32$OpenProcess(...);
if (!h) { BeaconPrintf(CALLBACK_ERROR, ...); return; }
// ... call the target API ...
KERNEL32$CloseHandle(h);  // close before checking status and before return
// ... check status, then return or report success ...
```

### NT_SUCCESS macro
**Source:** `winternl.h` (via `bofdefs.h` include chain — line 16 of bofdefs.h includes `<winternl.h>`)
**Apply to:** list.c (NtQuerySystemInformation), suspend.c, resume.c (NtSuspendProcess/NtResumeProcess)

```c
if (!NT_SUCCESS(status)) { ... }
```
Do NOT use `nt_success()` — that is a Kharon-specific macro not present in this project.

---

## No Analog Found

No files in this phase lack an analog. The four `.c` BOF files all follow the exitprocess.c structure. The RESEARCH.md patterns (2–7) provide the logic content where no closer codebase analog exists for the NtQuerySystemInformation loop and GetUserByToken helper — these are novel to this phase.

---

## Critical Implementation Notes for Planner

These are non-obvious constraints the planner must embed in task actions:

1. **bofdefs.h must be modified before any BOF compiles** — the 10 new declarations unblock compilation of all four source files. This is Wave 0.

2. **adaptix.h must exist before list.c compiles** — list.c includes `"adaptix.h"`. This is also Wave 0.

3. **MSVCRT$malloc is the most critical missing declaration** — identified in RESEARCH.md "Critical Infrastructure Gap". It is not in D-09's list but is required by list.c (GetUserByToken calls malloc three times). Add it to the MSVCRT section alongside `MSVCRT$calloc` (bofdefs.h line 73).

4. **First NtQuerySystemInformation call return code is NOT checked** — only the second call's status is checked with `NT_SUCCESS()`. Checking the first is wrong (it returns STATUS_INFO_LENGTH_MISMATCH, not success).

5. **BeaconPkgBytes length for UNICODE_STRING.Buffer fields**: use `ImageName.Length` directly (already bytes). For string literals and GetUserByToken results: use `wcslen(...) * sizeof(WCHAR)`.

6. **AdjustTokenPrivileges and LookupPrivilegeValueW**: add declarations to bofdefs.h per D-09 but do NOT implement EnableDebugPrivilege in list.c (D-05: skip entirely).

7. **Build verification command**: `make -C PS-BOF 2>&1 | grep -E '^\[.\]'` — all 8 targets must show `[+]`. The Makefile also builds `run` and `grep` (stubs); those are not in scope but must continue to compile.

---

## Metadata

**Analog search scope:** `/home/tgj/github/BOF-Collection/Exit-BOF/`, `/home/tgj/github/BOF-Collection/FS-BOF/`, `/home/tgj/github/BOF-Collection/_include/`, `/home/tgj/github/BOF-Collection/PS-BOF/`
**Files scanned:** 12 source files read directly
**Pattern extraction date:** 2026-05-16
