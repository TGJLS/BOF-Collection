---
phase: 29-tk-bof-setup
reviewed: 2026-05-23T00:00:00Z
depth: standard
files_reviewed: 10
files_reviewed_list:
  - _include/bofdefs.h
  - Makefile
  - TK-BOF/bofdefs.h
  - TK-BOF/Makefile
  - TK-BOF/make/make.c
  - TK-BOF/privget/privget.c
  - TK-BOF/revert/revert.c
  - TK-BOF/rm/rm.c
  - TK-BOF/steal/steal.c
  - TK-BOF/use/use.c
findings:
  critical: 1
  warning: 2
  info: 0
  total: 3
status: fixed
fixed: 2026-05-23T00:00:00Z
---

# Phase 29: Code Review Report

**Reviewed:** 2026-05-23
**Depth:** standard
**Files Reviewed:** 10
**Status:** issues_found

## Summary

Phase 29 introduces the TK-BOF build skeleton: six stub BOFs (steal, use, make, rm, revert, privget) with arg-parsing scaffolding, a TK-BOF-local `bofdefs.h` declaring ADVAPI32/NTDLL symbols, and the associated `Makefile`. The stubs compile cleanly because no API calls are made yet. One blocker exists that will prevent Phase 30/31 from compiling: the TK-BOF-local `bofdefs.h` is never reached by the compiler due to include-search-order shadowing. Two warnings cover a type mismatch in the token-handle variables and an undeclared symbol referenced by a macro in the shared header.

## Critical Issues

### CR-01: TK-BOF/bofdefs.h is shadowed and never included

**File:** `TK-BOF/Makefile:5`

**Issue:** The CFLAGS define include paths as `-I ../_include -I .`. GCC's quoted-include resolution for `"bofdefs.h"` in any TK-BOF source file (e.g. `steal/steal.c`) searches in this order:

1. Directory of the translation unit (`steal/`) — no `bofdefs.h` there
2. `-I ../_include` — `_include/bofdefs.h` **found** (the shared KERNEL32 file)
3. `-I .` (i.e. `TK-BOF/`) — never reached

`TK-BOF/bofdefs.h` — which contains every `ADVAPI32$*` and `NTDLL$NtClose` declaration needed by the token-manipulation BOFs — is therefore dead code. In Phase 29 this causes no compile error because the stubs make no API calls. In Phase 30/31, any call to `ADVAPI32$OpenProcessToken`, `ADVAPI32$DuplicateTokenEx`, `ADVAPI32$ImpersonateLoggedOnUser`, `ADVAPI32$RevertToSelf`, `ADVAPI32$LogonUserA`, `ADVAPI32$GetTokenInformation`, `ADVAPI32$AdjustTokenPrivileges`, or `NTDLL$NtClose` will fail with an implicit-declaration error (or silent wrong-symbol linkage).

**Fix:** Swap the include-path order **and** make `TK-BOF/bofdefs.h` chain-include the shared header so both symbol sets are available:

```makefile
# TK-BOF/Makefile line 5 — put local dir first
CFLAGS = -I . -I ../_include -w -Wno-incompatible-pointer-types -Os -DBOF -c
```

```c
/* TK-BOF/bofdefs.h — add chain include at the top */
#pragma once
#include "../_include/bofdefs.h"   /* pulls in KERNEL32, MSVCRT, NTDLL$RtlExit* */
#include <windows.h>
#include <winternl.h>

// ADVAPI32 and NTDLL$NtClose declarations follow unchanged ...
```

With `-I .` first, `"bofdefs.h"` from any subdirectory resolves to `TK-BOF/bofdefs.h`, which in turn chains to `_include/bofdefs.h` via a relative path. Both sets of symbols are then visible. The `#pragma once` guards prevent double-inclusion.

---

## Warnings

### WR-01: token_handle declared as DWORD instead of HANDLE in rm.c and use.c

**File:** `TK-BOF/rm/rm.c:8`, `TK-BOF/use/use.c:8`

**Issue:** Both files store the incoming token handle value in a `DWORD`:

```c
DWORD token_handle = 0;
BeaconDataParse(&parser, Buffer, Length);
token_handle = (DWORD) BeaconDataInt(&parser);
```

`BeaconDataInt` returns `int` (32-bit). On x64 Windows, `HANDLE` is a 64-bit pointer-sized type. When Phase 30/31 passes `token_handle` to any Windows API expecting `HANDLE` (e.g. `NTDLL$NtClose(token_handle)`, `ADVAPI32$DuplicateTokenEx(token_handle, ...)`), the compiler must widen `DWORD` to `HANDLE`, which is safe only because kernel handle values are small integers in practice. However, the explicit cast discards the upper 32 bits at the call site and the variable type will produce a compiler warning (`-Wall`) or silent truncation if the handle ever exceeds 32 bits.

The correct type for a variable that will be used as a `HANDLE` is `HANDLE` (or at minimum `ULONG_PTR`).

**Fix:**

```c
/* rm.c and use.c — change the variable type */
HANDLE token_handle = NULL;

BeaconDataParse(&parser, Buffer, Length);
token_handle = (HANDLE)(ULONG_PTR)(DWORD) BeaconDataInt(&parser);
```

The triple cast makes the widening explicit and silent: `int` → `DWORD` (same width, sign drop) → `ULONG_PTR` (zero-extend to pointer width) → `HANDLE`. When Phase 30/31 passes `token_handle` directly to a WinAPI, no further cast is needed and the type is correct.

---

### WR-02: intZeroMemory macro references undeclared MSVCRT$memset

**File:** `_include/bofdefs.h:94`

**Issue:** The shared header defines:

```c
#define intZeroMemory(addr,size) MSVCRT$memset((addr),0,size)
```

`MSVCRT$memset` is not declared anywhere in `_include/bofdefs.h` (the MSVCRT block on lines 77–81 declares only `calloc`, `free`, `vsnprintf`, and `_snprintf`). If any TK-BOF (or other BOF) calls `intZeroMemory`, the compiler will emit an implicit-declaration error under C99/C11, or link to the wrong symbol.

The macro is currently unused across all source files — a search of the entire repository finds no call sites — so this is latent rather than immediately broken.

**Fix:** Add the missing declaration to the MSVCRT block in `_include/bofdefs.h`:

```c
// in the MSVCRT section, after the existing declarations:
WINBASEAPI void * __cdecl MSVCRT$memset(void *dest, int c, size_t count);
```

Alternatively, remove `intZeroMemory` from the header entirely if zero-initialisation will be handled by `HEAP_ZERO_MEMORY` flags on every allocation.

---

_Reviewed: 2026-05-23_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
