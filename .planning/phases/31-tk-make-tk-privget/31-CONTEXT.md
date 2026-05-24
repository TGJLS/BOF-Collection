# Phase 31: tk make + tk privget - Context

**Gathered:** 2026-05-24
**Status:** Ready for planning

<domain>
## Phase Boundary

Implement the Win32 API logic bodies for two remaining TK-BOF commands: `make` (TK-03) and `privget` (TK-06). Both stubs exist with complete arg-parsing scaffolding from Phase 29. This phase replaces the empty stub bodies with working API call chains and defines the operator-facing output for each command.

Commands in scope: `make` (TK-03), `privget` (TK-06).
No axs wiring. No documentation. No CI entries.

</domain>

<decisions>
## Implementation Decisions

### make — LogonUserA call

- **D-01:** `logon_type` is a new 5th arg (after `no_apply`). Parsed via `BeaconDataInt`. Default sentinel: `if (logon_type == 0) logon_type = LOGON32_LOGON_NEW_CREDENTIALS` (9). axs (Phase 32) will send 0 when operator does not specify `--logon-type`.
- **D-02:** `LOGON32_PROVIDER_DEFAULT` (0) for `dwLogonProvider`.
- **D-03:** When `domain` is empty string, pass `"."` to LogonUserA — local machine authentication. Never pass NULL.
- **D-04:** On success with impersonation applied: `[+] Handle: 0x%llx\n` (same as steal, D-01 from Phase 30).
- **D-05:** On success with `--no-apply`: `[+] Handle: 0x%llx (impersonation not applied)\n` (same as steal, D-02 from Phase 30).
- **D-06:** Error prefix: `[-] make: LogonUserA failed: {FormatMessage text}` — command name as prefix, then TkErrorMessage text.
- **D-07:** On LogonUserA failure: no handle to close. On ImpersonateLoggedOnUser failure: close hToken with `NTDLL$NtClose(hToken)` before returning.

### privget — token selection

- **D-08:** `OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, TRUE, &hToken)` first. If that fails (no impersonation active on this thread), fall back to `OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken)`.
- **D-09:** `TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES` — minimum required access for `GetTokenInformation` + `AdjustTokenPrivileges`. No `TOKEN_ALL_ACCESS`.

### privget — output and error handling

- **D-10:** Success output: `[+] Enabled %lu privileges.\n` — count comes from `PTOKEN_PRIVILEGES->PrivilegeCount`.
- **D-11:** `ERROR_NOT_ALL_ASSIGNED` (1300) after `AdjustTokenPrivileges` → warn, not error: `[!] privget: not all privileges could be enabled.\n` then still print `[+] Enabled %lu privileges.`. Common on filtered admin tokens — not an abort condition.
- **D-12:** Other `AdjustTokenPrivileges` failures (non-zero NTSTATUS or GetLastError not 0/1300) → `[-] privget: AdjustTokenPrivileges failed: {TkErrorMessage text}`.
- **D-13:** `GetTokenInformation` is called twice: first with length 0 to get the required buffer size (`ReturnLength`), then with a `HeapAlloc`'d buffer. Use `KERNEL32$HeapAlloc` / `KERNEL32$HeapFree` (from `_include/bofdefs.h`).

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Pattern references (primary)
- `TK-BOF/steal/steal.c` — Canonical Phase 30 BOF: TkErrorMessage usage, NTDLL$NtClose for cleanup, `[+] Handle: 0x%llx` output format, handle close on error paths
- `TK-BOF/use/use.c` — ImpersonateLoggedOnUser pattern with TkErrorMessage
- `TK-BOF/rm/rm.c` — NtClose NTSTATUS check pattern
- `TK-BOF/tkerror.h` — TkErrorMessage static inline; used by all Phase 30 BOFs and must be used here too
- `TK-BOF/bofdefs.h` — All ADVAPI32$/NTDLL$ declarations; ADVAPI32$LogonUserA, ADVAPI32$GetTokenInformation, ADVAPI32$AdjustTokenPrivileges already declared
- `_include/bofdefs.h` — Root declarations including KERNEL32$OpenProcess, KERNEL32$GetLastError, KERNEL32$FormatMessageA, KERNEL32$HeapAlloc, KERNEL32$HeapFree, intAlloc, intFree

### Requirements
- `.planning/REQUIREMENTS.md` — TK-03 (make) and TK-06 (privget) are the two requirements for Phase 31

### Stubs to fill in
- `TK-BOF/make/make.c` — Stub with arg parsing: username, password, domain, no_apply (4 BeaconData calls); add 5th for logon_type
- `TK-BOF/privget/privget.c` — Stub with no args; replace empty body with full implementation

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `TK-BOF/tkerror.h`: `TkErrorMessage(DWORD dwError, char *buf, int bufSize)` — already used by steal, use; include with `#include "../tkerror.h"`
- `_include/bofdefs.h`: `KERNEL32$HeapAlloc` / `KERNEL32$HeapFree` — use for the `GetTokenInformation` two-pass buffer allocation in privget

### Established Patterns
- **Dynamic resolution**: `ADVAPI32$FunctionName(args)` call style throughout; never static
- **Arg parsing**: `BeaconDataParse` + `BeaconDataInt` / `BeaconDataExtract` — make stub already scaffolded for 4 args; add 5th `BeaconDataInt` for `logon_type`
- **Output**: `BeaconPrintf(CALLBACK_OUTPUT, "[+] ...")` for success, `BeaconPrintf(CALLBACK_ERROR, "[-] ...")` for errors, `BeaconPrintf(CALLBACK_OUTPUT, "[!] ...")` for warnings
- **Handle printing**: `0x%llx` with `(unsigned long long)` cast — consistent with steal.c
- **Handle close on error**: `NTDLL$NtClose(handle)` — not `KERNEL32$CloseHandle`

### Integration Points
- No new Makefile changes needed — both stubs already compile; filling in bodies does not affect build targets
- bofdefs.h already has all needed declarations — no new API declarations required for either BOF

### privget two-pass GetTokenInformation
```c
// Pass 1: get required buffer size
ADVAPI32$GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &tokenInfoLen);
// Pass 2: allocate and fill
PTOKEN_PRIVILEGES pTokPriv = (PTOKEN_PRIVILEGES) KERNEL32$HeapAlloc(
    KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, tokenInfoLen);
ADVAPI32$GetTokenInformation(hToken, TokenPrivileges, pTokPriv, tokenInfoLen, &tokenInfoLen);
// Enable all: set each privilege's Attributes = SE_PRIVILEGE_ENABLED
// Then AdjustTokenPrivileges once for the whole PTOKEN_PRIVILEGES
KERNEL32$HeapFree(KERNEL32$GetProcessHeap(), 0, pTokPriv);
```

</code_context>

<specifics>
## Specific Ideas

- `make` logon type sentinel: `if (logon_type == 0) logon_type = 9;` at the top of the function, before the LogonUserA call
- `make` domain sentinel: `const char *dom = (domain && domain[0]) ? domain : ".";` — use `dom` in LogonUserA call
- privget warning uses `[!]` prefix (not `[-]`), printed via `CALLBACK_OUTPUT` — not an error path
- `AdjustTokenPrivileges` with `DisableAllPrivileges = FALSE` and `NewState = pTokPriv` (all privs set to `SE_PRIVILEGE_ENABLED`) enables all in one call; check `GetLastError()` after the call (function returns TRUE even on partial success)

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>

---

*Phase: 31-tk-make-tk-privget*
*Context gathered: 2026-05-24*
