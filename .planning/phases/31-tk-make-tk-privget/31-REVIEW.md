---
phase: 31-tk-make-tk-privget
reviewed: 2026-05-24T00:00:00Z
depth: standard
files_reviewed: 3
files_reviewed_list:
  - TK-BOF/make/make.c
  - TK-BOF/bofdefs.h
  - TK-BOF/privget/privget.c
findings:
  critical: 1
  warning: 2
  info: 1
  total: 4
status: fixed
fixed_in: c38c5bd
---

# Phase 31: Code Review Report

**Reviewed:** 2026-05-24
**Depth:** standard
**Files Reviewed:** 3
**Status:** fixed (all 4 findings resolved in commit c38c5bd)

## Summary

Reviewed `make.c`, `privget.c`, and `TK-BOF/bofdefs.h`. The Win32 API signatures in `bofdefs.h` are correct. The `make.c` token-creation flow and `privget.c` two-pass `GetTokenInformation` pattern are sound. `AdjustTokenPrivileges` error handling correctly distinguishes `ERROR_NOT_ALL_ASSIGNED` (partial success) from hard failure. The intentional handle-persistence design (printing but not closing `hToken`/`hDup` on success) is consistent with `steal.c` and is by design.

Two real defects found: a NULL-pointer crash in `make.c` if the C2 sends a short/malformed argument buffer, and a silent incorrect fallback in `privget.c` when `OpenThreadToken` fails for any reason other than the thread having no token. One additional warning for the misleading output count when `AdjustTokenPrivileges` partially succeeds.

## Critical Issues

### CR-01: Unvalidated `BeaconDataExtract` return values crash beacon on malformed input

**File:** `TK-BOF/make/make.c:19-28`

**Issue:** `BeaconDataExtract` returns `NULL` when the parser is exhausted or the argument buffer is malformed. `username` and `password` are passed directly to `LogonUserA` on line 28 without a NULL check. `LogonUserA` will dereference both pointers unconditionally; a NULL `username` or `password` causes an access violation inside the beacon process. `domain` is safe because the sentinel expression `(domain && domain[0])` short-circuits on NULL, but `username` and `password` are not guarded.

The FS-BOF collection validates every `BeaconDataExtract` return (see `FS-BOF/type/type.c` lines 17-21 for the established pattern). `make.c` is the only BOF in this codebase that skips this validation for user-controlled string arguments.

**Fix:**
```c
username = BeaconDataExtract(&parser, NULL);
password = BeaconDataExtract(&parser, NULL);
domain   = BeaconDataExtract(&parser, NULL);

if (!username || !password)
{
    BeaconPrintf(CALLBACK_ERROR, "[-] make: missing required argument (username or password)\n");
    return;
}
```

## Warnings

### WR-01: `OpenThreadToken` failure is silently swallowed on all error codes, not just `ERROR_NO_TOKEN`

**File:** `TK-BOF/privget/privget.c:17-29`

**Issue:** The fallback from `OpenThreadToken` to `OpenProcessToken` is unconditional. The correct semantics are: fall back only when the thread has no impersonation token (`ERROR_NO_TOKEN`, value 1008). If `OpenThreadToken` fails with any other error — most importantly `ERROR_ACCESS_DENIED` (5), which fires when the thread does carry an impersonation token but the requested access mask is denied — the code silently opens the process token instead. The operator then adjusts privileges on the process primary token believing they are acting on their impersonated identity, which is wrong. This is a logic correctness issue, not just a quality issue, because it causes silent misbehavior when the beacon is already impersonating.

**Fix:**
```c
if (!ADVAPI32$OpenThreadToken(KERNEL32$GetCurrentThread(),
                              TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                              TRUE, &hToken))
{
    if (KERNEL32$GetLastError() != ERROR_NO_TOKEN)
    {
        dwError = KERNEL32$GetLastError();
        TkErrorMessage(dwError, errMsg, sizeof(errMsg));
        BeaconPrintf(CALLBACK_ERROR, "[-] privget: OpenThreadToken failed: %s\n", errMsg);
        return;
    }
    /* Thread has no token — fall back to process token */
    if (!ADVAPI32$OpenProcessToken(KERNEL32$GetCurrentProcess(),
                                   TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                                   &hToken))
    {
        dwError = KERNEL32$GetLastError();
        TkErrorMessage(dwError, errMsg, sizeof(errMsg));
        BeaconPrintf(CALLBACK_ERROR, "[-] privget: OpenProcessToken failed: %s\n", errMsg);
        return;
    }
}
```

Note: `GetLastError()` must be captured before the `OpenProcessToken` call clobbers it, hence the early capture shown above.

### WR-02: Output count is "attempted" not "enabled" but is labeled "Enabled" unconditionally

**File:** `TK-BOF/privget/privget.c:60,75-78`

**Issue:** `privCount` is set to `pTokPriv->PrivilegeCount` — the total number of privileges in the token — before `AdjustTokenPrivileges` runs. When `GetLastError()` returns `ERROR_NOT_ALL_ASSIGNED` (1300), the code correctly prints a partial-success warning on line 76, but then line 78 still prints `"[+] Enabled %lu privileges."` using the pre-adjustment total count. An operator seeing `"[!] not all privileges could be enabled"` followed by `"[+] Enabled 28 privileges"` will be uncertain how many were actually enabled.

This does not cause incorrect behavior in the API path, but it delivers misleading output when the system cannot grant all privileges (e.g., removed privileges, integrity level constraints).

**Fix:** Differentiate the two cases in the output:
```c
if (dwError == 1300)
{
    BeaconPrintf(CALLBACK_OUTPUT,
        "[!] privget: not all privileges could be enabled (attempted %lu).\n",
        (unsigned long) privCount);
}
else
{
    BeaconPrintf(CALLBACK_OUTPUT, "[+] Enabled %lu privileges.\n",
        (unsigned long) privCount);
}
```

## Info

### IN-01: Magic number `1300` should be `ERROR_NOT_ALL_ASSIGNED`

**File:** `TK-BOF/privget/privget.c:68,75`

**Issue:** The literal `1300` is used twice to represent `ERROR_NOT_ALL_ASSIGNED`. This constant is defined in `<winerror.h>` (which is pulled in via `<windows.h>`). Using the named constant makes the intent self-documenting and immune to transcription errors.

**Fix:**
```c
// line 68
if (dwError != 0 && dwError != ERROR_NOT_ALL_ASSIGNED)

// line 75
if (dwError == ERROR_NOT_ALL_ASSIGNED)
```

---

_Reviewed: 2026-05-24_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
_Fixed: 2026-05-24 — all findings resolved in commit c38c5bd_
