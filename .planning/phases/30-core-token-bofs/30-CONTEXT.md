# Phase 30: Core Token BOFs - Context

**Gathered:** 2026-05-23
**Status:** Ready for planning

<domain>
## Phase Boundary

Implement the real Win32 API logic bodies for 4 TK-BOF commands: `steal`, `use`, `rm`, `revert`. The arg-parsing scaffolding already exists (from Phase 29 stubs). This phase replaces the empty stub bodies with working API call chains and defines the operator-facing output for each command.

Commands in scope: `steal` (TK-01), `use` (TK-02), `rm` (TK-04), `revert` (TK-05).
Commands deferred to Phase 31: `make` (TK-03), `privget` (TK-06).
No axs wiring. No documentation. No CI entries.

</domain>

<decisions>
## Implementation Decisions

### Handle output format
- **D-01:** On successful steal (impersonation applied): `[+] Handle: 0x1a4` — minimal hex, no PID context.
- **D-02:** On successful steal with `--no-apply`: `[+] Handle: 0x1a4 (impersonation not applied)` — annotate that token is ready but not active.
- **D-03:** On successful `use`: `[+] Impersonating handle 0x1a4` — echoes the handle that is now active.

### Error verbosity
- **D-04:** Errors use FormatMessage human-readable text via a shared TK-BOF helper (e.g., `tkerror.h` or added to `bofdefs.h`). Format: `[-] steal: OpenProcess failed: Access is denied.` — function name as prefix, then `FormatMessageA` text.
- **D-05:** The helper wraps `KERNEL32$FormatMessageA` (since dynamic resolution is required in BOF context). It should be a small static inline or macro — not a full separate .c file. Placed at `TK-BOF/tkerror.h` if a separate file, or inlined into `bofdefs.h`.

### rm/revert success output
- **D-06:** On successful `rm`: `[+] Handle 0x1a4 closed.` — echoes the handle value that was freed.
- **D-07:** On successful `revert`: `[+] Reverted to process token.` — clear confirmation that impersonation is dropped.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Pattern references (primary)
- `TK-BOF/bofdefs.h` — All ADVAPI32$/NTDLL$ declarations needed across all 6 BOFs; steal/use/rm/revert use this directly
- `_include/bofdefs.h` — Root declarations including KERNEL32$OpenProcess, KERNEL32$GetLastError, KERNEL32$FormatMessageA — check before declaring anything new in TK-BOF/bofdefs.h
- `FS-BOF/cd/cd.c` — Canonical BOF output pattern: BeaconPrintf for success/error, GetLastError for error codes

### Requirements
- `.planning/REQUIREMENTS.md` — TK-01 (steal), TK-02 (use), TK-04 (rm), TK-05 (revert) are the 4 requirements for Phase 30

### Existing stubs (fill in these files)
- `TK-BOF/steal/steal.c` — Stub with arg parsing: `pid` (DWORD), `no_apply` (BOOL)
- `TK-BOF/use/use.c` — Stub with arg parsing: `token_handle` (HANDLE from DWORD cast)
- `TK-BOF/rm/rm.c` — Stub with arg parsing: `token_handle` (HANDLE from DWORD cast)
- `TK-BOF/revert/revert.c` — Stub with no args

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `TK-BOF/bofdefs.h`: declares all APIs needed — `ADVAPI32$OpenProcessToken`, `ADVAPI32$DuplicateTokenEx`, `ADVAPI32$ImpersonateLoggedOnUser`, `ADVAPI32$RevertToSelf`, `NTDLL$NtClose`
- `_include/bofdefs.h`: declares `KERNEL32$OpenProcess`, `KERNEL32$GetLastError`, `KERNEL32$FormatMessageA` (check for FormatMessageA — confirm before declaring)

### Established Patterns
- **Dynamic resolution**: `ADVAPI32$FunctionName(args)` call style throughout; no static linking
- **Arg parsing**: `BeaconDataParse` + `BeaconDataInt` / `BeaconDataExtract` — already scaffolded in stubs; do not change the parsing
- **Output**: `BeaconPrintf(CALLBACK_OUTPUT, "[+] ...")` for success, `BeaconPrintf(CALLBACK_ERROR, "[-] ...")` for errors
- **Handle printing**: use `%p` or `0x%lx` for hex handle values consistent with D-01

### Integration Points
- No new Makefile changes needed — stubs already compile; filling in bodies does not affect build targets
- `KERNEL32$OpenProcess` declared in root `_include/bofdefs.h` (confirmed grep hit at line 72) — steal uses it without any new declarations

### API call chains per command
- **steal**: `KERNEL32$OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid)` → `ADVAPI32$OpenProcessToken(hProcess, TOKEN_DUPLICATE, &hToken)` → `ADVAPI32$DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenImpersonation, &hDup)` → `ADVAPI32$ImpersonateLoggedOnUser(hDup)` (skip if no_apply) → print handle
- **use**: `ADVAPI32$ImpersonateLoggedOnUser(token_handle)` → print confirmation
- **rm**: `NTDLL$NtClose(token_handle)` → print confirmation
- **revert**: `ADVAPI32$RevertToSelf()` → print confirmation

</code_context>

<specifics>
## Specific Ideas

- Handle values in output must be printed in hex with `0x` prefix (e.g., `0x1a4`) — operators copy-paste these into subsequent `tk use` / `tk rm` calls; consistent format matters
- Error prefix uses the command name: `[-] steal: OpenProcess failed: ...` so operator knows which step failed in the call chain
- `FormatMessageA` in BOF context requires dynamic resolution — check if `KERNEL32$FormatMessageA` is already in `_include/bofdefs.h` before adding to `tkerror.h`
- steal must close intermediate handles on failure paths (hProcess, hToken) to avoid handle leaks in the beacon process
- rm and revert do not need handle leak cleanup — they have single calls with no intermediate state

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>

---

*Phase: 30-Core Token BOFs*
*Context gathered: 2026-05-23*
