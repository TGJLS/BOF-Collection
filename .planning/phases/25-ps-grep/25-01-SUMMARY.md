---
plan: 25-01
phase: 25
status: complete
completed: 2026-05-20
commit: ae42e9b6c8f486edf95a5489f56d4efb30217eb7
---

# Summary: Plan 25-01 — bofdefs.h Additions and grep.c Implementation

## What Was Built

Replaced the 5-line empty stub at `PS-BOF/grep/grep.c` with a fully-functional C BOF that accepts a
single PID argument and outputs four sections: [Token], [Modules], [Cmdline], and [Threads].

Also added `#include <tlhelp32.h>` and two new declaration sections to `_include/bofdefs.h`:
- KERNEL32 toolhelp (CreateToolhelp32Snapshot, Thread32First, Thread32Next)
- NTDLL process info (NtQueryInformationProcess)

## Tasks Completed

- **T01**: Added 4 API declarations to `_include/bofdefs.h` and added `#include <tlhelp32.h>` to
  make the LPTHREADENTRY32 type available to all BOFs that include bofdefs.h.
- **T02**: Implemented `PS-BOF/grep/grep.c` with WideToUtf8 helper, get_tokens, get_modules,
  get_cmdline, and get_threads helper functions, and a go() entry point using goto cleanup.

## Verification

- `make -C PS-BOF all` exits 0 — all 12 targets (`[+]`) including `grep x64` and `grep x32`
- All 17 source assertions pass (bofdefs.h declarations + grep.c structural checks)
- `PS-BOF/_bin/grep.x64.o` and `PS-BOF/_bin/grep.x32.o` produced

## Key Implementation Decisions Applied

- `#include <tlhelp32.h>` added to bofdefs.h (not just grep.c) — required because LPTHREADENTRY32
  is needed at the point where bofdefs.h is parsed by all other BOFs
- `ProcessCommandLineInformation` defined as `((PROCESSINFOCLASS)60)` — absent from MinGW winternl.h
- NtQueryInformationToken probe pattern (check return_len > 0) used for TokenUser and
  TokenIntegrityLevel; TokenElevation uses fixed-size struct
- PUNICODE_STRING cast for cmdline buffer — NtQueryInformationProcess returns UNICODE_STRING header
  not raw WCHAR*
- GetSidSubAuthority / GetSidSubAuthorityCount called directly per CONTEXT.md exception
- goto cleanup at go() level for process_handle; each helper manages its own resources
</content>
</invoke>