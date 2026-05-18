# Phase 24: ps run — Research

## Summary

Phase 24 implements `PS-BOF/run/run.c` — a single-file BOF that launches processes via three
Windows APIs (CreateProcessW, CreateProcessWithLogonW, CreateProcessWithTokenW) with optional
PPID spoofing and stdout/stderr capture. The Kharon source (`kit_process_creation.cc`) has been
fully read and confirms the CONTEXT.md design decisions — with **one important correction**: when
PPID spoofing and pipe capture are both active simultaneously, `DuplicateHandle` is required to
copy `pipe_write` into the spoofed parent's handle table. Additionally, `KERNEL32$DuplicateHandle`
and `MSVCRT$realloc` are missing from `bofdefs.h` and must be added (10 new declarations total,
not 9).

---

## API Signatures & Flags

### CreateProcessW (Default method)
```c
WINBASEAPI BOOL WINAPI KERNEL32$CreateProcessW(
    LPCWSTR lpApplicationName,          // NULL — use lpCommandLine
    LPWSTR  lpCommandLine,              // writable command string (MUST be writable buffer)
    LPSECURITY_ATTRIBUTES lpProcessAttributes,  // NULL
    LPSECURITY_ATTRIBUTES lpThreadAttributes,   // NULL
    BOOL    bInheritHandles,            // TRUE (inherit pipe handles)
    DWORD   dwCreationFlags,            // CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT | CREATE_NO_WINDOW
    LPVOID  lpEnvironment,              // NULL
    LPCWSTR lpCurrentDirectory,         // NULL
    LPSTARTUPINFOW lpStartupInfo,       // cast from &StartupInfoEx.StartupInfo when PPID active
    LPPROCESS_INFORMATION lpProcessInformation
);
```
- Supports `EXTENDED_STARTUPINFO_PRESENT` (0x00080000) for attribute list (PPID spoofing)
- `lpCommandLine` **must be a writable buffer** — copy parsed WCHAR* before calling

### CreateProcessWithLogonW (WithLogon method)
```c
WINADVAPI BOOL WINAPI ADVAPI32$CreateProcessWithLogonW(
    LPCWSTR lpUsername,         // from args
    LPCWSTR lpDomain,           // from args
    LPCWSTR lpPassword,         // from args
    DWORD   dwLogonFlags,       // LOGON_WITH_PROFILE (0x1) — Kharon always uses this
    LPCWSTR lpApplicationName,  // NULL
    LPWSTR  lpCommandLine,      // writable command buffer
    DWORD   dwCreationFlags,    // CREATE_SUSPENDED | CREATE_NO_WINDOW (no EXTENDED_STARTUPINFO_PRESENT)
    LPVOID  lpEnvironment,      // NULL
    LPCWSTR lpCurrentDirectory, // NULL
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
);
```
- **Does NOT support `EXTENDED_STARTUPINFO_PRESENT`** — plain `STARTUPINFOW` only
- **Does NOT support PPID spoofing** — attribute lists are rejected
- Requires Secondary Logon service (`seclogon`) to be running

### CreateProcessWithTokenW (WithToken method)
```c
WINADVAPI BOOL WINAPI ADVAPI32$CreateProcessWithTokenW(
    HANDLE  hToken,             // stolen token handle from args
    DWORD   dwLogonFlags,       // LOGON_WITH_PROFILE (0x1) — Kharon always uses this
    LPCWSTR lpApplicationName,  // NULL
    LPWSTR  lpCommandLine,      // writable command buffer
    DWORD   dwCreationFlags,    // CREATE_SUSPENDED | CREATE_NO_WINDOW (no EXTENDED_STARTUPINFO_PRESENT)
    LPVOID  lpEnvironment,      // NULL
    LPCWSTR lpCurrentDirectory, // NULL
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
);
```
- **Does NOT support `EXTENDED_STARTUPINFO_PRESENT`** — plain `STARTUPINFOW` only
- **Does NOT support PPID spoofing**
- Requires `SE_IMPERSONATE_NAME` privilege in the calling process

---

## PPID Spoofing Pattern (CreateProcessW only)

Used only when `ppid != 0` and `method == DEFAULT`. From Kharon lines 219–231:

1. Count attributes: `update_attr_count = 1` (just PPID, blockdlls is out of scope)
2. Size probe: `KERNEL32$InitializeProcThreadAttributeList(NULL, 1, 0, &attribute_size)`
3. Allocate: `attribute_buff = MSVCRT$malloc(attribute_size)`
4. Initialize: `KERNEL32$InitializeProcThreadAttributeList(attribute_buff, 1, 0, &attribute_size)`
5. Open parent: `parent_handle = KERNEL32$OpenProcess(PROCESS_CREATE_PROCESS | PROCESS_DUP_HANDLE, FALSE, ppid)`
   - `PROCESS_DUP_HANDLE` is also required (needed for the DuplicateHandle path below)
6. Update attribute: `KERNEL32$UpdateProcThreadAttribute(attribute_buff, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &parent_handle, sizeof(HANDLE), NULL, NULL)`
7. Set `startup_info_ex.StartupInfo.cb = sizeof(STARTUPINFOEXW)`
8. Set `startup_info_ex.lpAttributeList = attribute_buff`
9. Add `EXTENDED_STARTUPINFO_PRESENT` to `dwCreationFlags`
10. Pass `&startup_info_ex.StartupInfo` (cast) to `CreateProcessW`
11. Cleanup: `KERNEL32$DeleteProcThreadAttributeList(attribute_buff)` → `MSVCRT$free(attribute_buff)` → `KERNEL32$CloseHandle(parent_handle)`

---

## Pipe Capture Pattern

Used when `pipe != 0`. From Kharon lines 259–289 and D-04:

```c
SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
KERNEL32$CreatePipe(&pipe_read, &pipe_write, &sa, 0);
KERNEL32$SetHandleInformation(pipe_read, HANDLE_FLAG_INHERIT, 0); // child must NOT inherit read end
// Set STARTF_USESTDHANDLES on whichever startup info is active
si.hStdOutput = pipe_write;
si.hStdError  = pipe_write;
si.hStdInput  = KERNEL32$GetStdHandle(STD_INPUT_HANDLE);
// ... CreateProcess ...
KERNEL32$CloseHandle(pipe_write); pipe_write = NULL; // MUST close before ReadFile loop
// Simple blocking read loop (D-04 — no PeekNamedPipe, no timeout):
DWORD bytes_read = 0;
char buf[4096];
while (KERNEL32$ReadFile(pipe_read, buf, sizeof(buf), &bytes_read, NULL) && bytes_read > 0) {
    BeaconOutput(CALLBACK_OUTPUT, buf, bytes_read);
}
KERNEL32$CloseHandle(pipe_read); // in cleanup
```

**Key**: `pipe_write` must be closed **before** the ReadFile loop or the loop never terminates
(the write end stays open in the parent, so ReadFile never gets EOF).

---

## PPID + Pipe Interaction — Critical Correction to D-Discretion

**CONTEXT.md D-Discretion states**: "DuplicateHandle not required — bInheritHandles=TRUE causes
child to inherit pipe_write from the calling BOF process's handle table."

**This is incorrect for the PPID+pipe combined case.** When `PROC_THREAD_ATTRIBUTE_PARENT_PROCESS`
is active, Windows inherits handles from the *spoofed parent's* handle table, not the BOF's handle
table. `pipe_write` is in the BOF's table → the child never inherits it → capture fails silently.

**Kharon's fix** (lines 268–277):
```c
if (use_extended_info && ppid && parent_handle) {
    DuplicateHandle(GetCurrentProcess(), pipe_write, parent_handle,
                    &pipe_duplicate, 0, TRUE, DUPLICATE_SAME_ACCESS);
    CloseHandle(pipe_write);
    pipe_write = pipe_duplicate; // use duplicated handle for stdout/stderr
}
```

**Our implementation must match this.** This requires `KERNEL32$DuplicateHandle` — which is
**currently missing from bofdefs.h**. It must be added as the 10th new declaration.

**When PPID is 0 (no spoofing):** `bInheritHandles=TRUE` is sufficient — child inherits
`pipe_write` from the BOF's handle table normally. No DuplicateHandle needed in that path.

---

## bofdefs.h Declaration Audit

The 9 declarations from CONTEXT.md D-06 are **confirmed correct**. One additional declaration
is required: `KERNEL32$DuplicateHandle` for the PPID+pipe interaction. Also, `MSVCRT$realloc`
may be needed if implementing a growing buffer for pipe output — Kharon uses it (line 70 of
kit_process_creation.cc). Our simple loop approach streams directly to BeaconOutput so no
growing buffer is needed, but `MSVCRT$realloc` is absent from bofdefs.h regardless.

### Declarations to add (10 total):

```c
// KERNEL32 — process creation (PS-BOF run)
WINBASEAPI BOOL   WINAPI KERNEL32$CreateProcessW(LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
WINBASEAPI HANDLE WINAPI KERNEL32$GetStdHandle(DWORD nStdHandle);
WINBASEAPI BOOL   WINAPI KERNEL32$CreatePipe(PHANDLE, PHANDLE, LPSECURITY_ATTRIBUTES, DWORD);
WINBASEAPI BOOL   WINAPI KERNEL32$SetHandleInformation(HANDLE, DWORD, DWORD);
WINBASEAPI BOOL   WINAPI KERNEL32$InitializeProcThreadAttributeList(LPPROC_THREAD_ATTRIBUTE_LIST, DWORD, DWORD, PSIZE_T);
WINBASEAPI BOOL   WINAPI KERNEL32$UpdateProcThreadAttribute(LPPROC_THREAD_ATTRIBUTE_LIST, DWORD, DWORD_PTR, PVOID, SIZE_T, PVOID, PSIZE_T);
WINBASEAPI VOID   WINAPI KERNEL32$DeleteProcThreadAttributeList(LPPROC_THREAD_ATTRIBUTE_LIST);
WINBASEAPI BOOL   WINAPI KERNEL32$DuplicateHandle(HANDLE, HANDLE, HANDLE, LPHANDLE, DWORD, BOOL, DWORD);  // ← NEW (10th)

// ADVAPI32 — process creation with credentials/token (PS-BOF run)
WINADVAPI BOOL WINAPI ADVAPI32$CreateProcessWithLogonW(LPCWSTR, LPCWSTR, LPCWSTR, DWORD, LPCWSTR, LPWSTR, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
WINADVAPI BOOL WINAPI ADVAPI32$CreateProcessWithTokenW(HANDLE, DWORD, LPCWSTR, LPWSTR, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
```

`KERNEL32$GetCurrentProcess` is already declared in bofdefs.h (line 98) — reused for
DuplicateHandle's `hSourceProcessHandle` parameter.

---

## Arg Parse Order Validation

Confirmed from Kharon `create.cc` lines 9–19. Note that Kharon reads `ppid` from
`BeaconInformation` (not from args), and has no explicit `--ppid` arg. Our design adds
`ppid` as an explicit arg between `pipe` and `domain`. The order is:

| Position | Field     | Type                   | Source         |
|----------|-----------|------------------------|----------------|
| 1        | method    | BeaconDataInt          | enum 0/1/2     |
| 2        | command   | BeaconDataExtract wstr | lpCommandLine  |
| 3        | state     | BeaconDataInt          | 0=normal,1=SUSPENDED |
| 4        | pipe      | BeaconDataInt          | 0=no,1=capture |
| 5        | ppid      | BeaconDataInt          | 0=no spoofing  |
| 6        | domain    | BeaconDataExtract wstr | WithLogon      |
| 7        | username  | BeaconDataExtract wstr | WithLogon      |
| 8        | password  | BeaconDataExtract wstr | WithLogon      |
| 9        | token     | BeaconDataInt (HANDLE) | WithToken      |

This order must be matched exactly by ps.axs packing in Phase 26.

---

## Startup Info: Which Method Uses Which

| Method              | Struct Type        | EXTENDED_STARTUPINFO_PRESENT | PPID Spoofing |
|---------------------|--------------------|------------------------------|---------------|
| Default (CreateProcessW) | STARTUPINFOEXW | Yes (when ppid≠0)         | Supported ✓   |
| WithLogon           | STARTUPINFOW       | No — API rejects it          | Not supported |
| WithToken           | STARTUPINFOW       | No — API rejects it          | Not supported |

All methods set `STARTF_USESHOWWINDOW | SW_HIDE` to suppress console window.
All methods use `CREATE_NO_WINDOW` in `dwCreationFlags`.

---

## struct PS_CREATE_ARGS (C translation of Kharon general.h)

```c
// Method constants (replaces Kharon's C++ enum class Create)
#define CREATE_METHOD_DEFAULT  0
#define CREATE_METHOD_LOGON    1
#define CREATE_METHOD_TOKEN    2

typedef struct {
    int     method;     // CREATE_METHOD_*
    DWORD   state;      // 0 or CREATE_SUSPENDED
    int     pipe;       // 0=no capture, 1=capture
    int     ppid;       // 0=no spoofing, nonzero=target PPID

    HANDLE  token;      // WithToken method

    WCHAR  *argument;   // lpCommandLine (writable copy needed)
    WCHAR  *domain;     // WithLogon
    WCHAR  *username;   // WithLogon
    WCHAR  *password;   // WithLogon
} PS_CREATE_ARGS;
```

No `spoofarg`, no `blockdlls` fields — both are out of scope per REQUIREMENTS.md.

---

## Kharon Translation Notes

| Kharon (C++)                        | Our C port                                      |
|-------------------------------------|-------------------------------------------------|
| `enum class Create { Default, ... }`| `#define CREATE_METHOD_DEFAULT 0` etc.          |
| Lambda `cleanup` with captures      | `goto cleanup` label + inline closes            |
| `PeekNamedPipe` polling loop        | Simple blocking `ReadFile` loop (D-04)          |
| `WaitForSingleObject` timeout       | No timeout — blocking until pipe EOF            |
| `BeaconPkgInt32/BeaconPkgBytes`     | `BeaconPrintf(OUTPUT)` / `BeaconOutput`         |
| `fmt_error()` returns `WCHAR*`      | Use `FormatMessageA` + `char*` (D-05)           |
| `BeaconInformation` for ppid        | Explicit `BeaconDataInt` from args (D-Discretion) |
| `auto` / `nullptr`                  | Explicit types / `NULL`                         |
| `malloc`/`free` (CRT linked)        | `MSVCRT$malloc` / `MSVCRT$free`                 |
| `spoofarg` PEB patching             | Not implemented (out of scope, PS-EX-04)        |
| `blockdlls` mitigation policy       | Not implemented (out of scope)                  |
| Process info returned to caller     | BeaconPrintf "Process started: PID %d, TID %d" |

---

## Edge Cases & Gotchas

1. **Writable lpCommandLine**: `CreateProcessW` may modify `lpCommandLine` in-place. The WCHAR*
   from `BeaconDataExtract` points into the beacon args buffer — do NOT pass it directly.
   Copy to a local `WCHAR cmd_buf[MAX_PATH+1]` or `MSVCRT$malloc` buffer first.

2. **Secondary Logon service**: `CreateProcessWithLogonW` requires `seclogon` to be running.
   If disabled, `GetLastError()` returns `ERROR_SERVICE_DISABLED` (1058). The error message
   from `FormatMessageA` will be human-readable.

3. **SE_IMPERSONATE_NAME**: `CreateProcessWithTokenW` requires the calling process's token
   to have `SeImpersonatePrivilege`. If missing, it fails with `ERROR_PRIVILEGE_NOT_HELD` (1314).

4. **PROCESS_DUP_HANDLE on parent**: `OpenProcess` for PPID must include `PROCESS_DUP_HANDLE`
   (not just `PROCESS_CREATE_PROCESS`) to support the DuplicateHandle path when pipe is also
   active. Kharon line 220: `PROCESS_CREATE_PROCESS | PROCESS_DUP_HANDLE`.

5. **pipe_write must be closed before ReadFile**: Forgetting this causes the blocking ReadFile
   loop to hang indefinitely because the write-end is still open in the parent.

6. **Cleanup order**: Close `pipe_write` before `pipe_read` in the cleanup block. The ReadFile
   loop must have already consumed the pipe before reaching cleanup.

7. **MSVCRT$realloc missing**: Not declared in bofdefs.h. Our streaming ReadFile approach
   sends chunks directly via BeaconOutput and doesn't need a growing buffer, so no realloc
   needed. If a future variant needs a growing buffer, add the declaration then.

8. **CREATE_NO_WINDOW**: Set for all methods to prevent a console window flash. Kharon uses
   this (line 159: `CREATE_NO_WINDOW`). The CONTEXT.md omits this but Kharon includes it.

---

## Plan Structure Recommendation

Given the complexity (3 API methods, 2 optional features, 1 edge case interaction), recommend
**2 plans**:

- **24-01-PLAN.md** (Wave 1): Add 10 bofdefs.h declarations; implement `run.c` with
  Default+WithLogon+WithToken methods, PPID spoofing, pipe capture, and the
  DuplicateHandle interaction path.
- **24-02-PLAN.md** (Wave 2): Build verification — `make` from PS-BOF, confirm
  `run.x64.o` and `run.x32.o` produced with no errors or warnings.

Alternative: single plan if the planner judges the implementation straightforward enough.

---

## Validation Architecture

### Functional Test Matrix

| Test | Method | Pipe | PPID | Expected |
|------|--------|------|------|---------|
| Basic launch | Default | No | No | "Process started: PID N, TID M" |
| Suspended launch | Default | No | No | Process in suspended state |
| Pipe capture | Default | Yes | No | stdout content via BeaconOutput |
| PPID spoof | Default | No | Yes | ps list shows spoofed PPID |
| PPID + pipe | Default | Yes | Yes | Pipe capture works with DuplicateHandle |
| WithLogon | WithLogon | No | No | Process launched as specified user |
| WithToken | WithToken | No | No | Process launched under token identity |

### Build Verification
```bash
cd PS-BOF && make
# Expected: run.x64.o and run.x32.o in PS-BOF/_bin/ with no errors
```

### Compile-only Check
```bash
x86_64-w64-mingw32-gcc -Os -DBOF -c PS-BOF/run/run.c -I _include -o /dev/null
i686-w64-mingw32-gcc   -Os -DBOF -c PS-BOF/run/run.c -I _include -o /dev/null
```

### Live BOF Test (Phase 28 CI)
- `ps run --command "cmd.exe /c whoami" --pipe true` → output contains beacon username
- `ps run --command "notepad.exe" --state suspended` → process visible in ps list, suspended
- `ps kill <pid>` after ps run confirms PID is valid

---

## RESEARCH COMPLETE

Key findings for the planner:
1. **10 bofdefs.h declarations** (not 9) — `KERNEL32$DuplicateHandle` is required for PPID+pipe combined case
2. **D-Discretion correction**: When PPID spoofing + pipe are both active, `DuplicateHandle` into the parent process's handle table is mandatory (confirmed from Kharon lines 268–277)
3. **Writable command buffer**: `lpCommandLine` must be a copy, not the raw `BeaconDataExtract` pointer
4. **`CREATE_NO_WINDOW`** should be added to all creation flags (present in Kharon, absent from CONTEXT.md)
5. **`PROCESS_DUP_HANDLE`** must be included in the `OpenProcess` access mask for PPID (alongside `PROCESS_CREATE_PROCESS`)
6. **Simple blocking ReadFile** loop confirmed correct for our use case (D-04 validated)
7. **2 plans** recommended: implementation (Wave 1) + build verification (Wave 2)
