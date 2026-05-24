---
plan: 32-01
phase: 32-tk-axs-documentation
status: complete
completed: 2026-05-24
commits:
  - 59d4159 (merge origin/main — PS-BOF integration)
  - b65eefc (feat(32-01): replace ADVAPI32$LogonUserA with LogonUserW in bofdefs.h)
  - 0f972a5 (feat(32-01): convert make.c to LogonUserW with WCHAR* args)
key-files:
  created: []
  modified:
    - TK-BOF/bofdefs.h
    - TK-BOF/make/make.c
    - Makefile
    - bof-collection.axs
    - README.md
    - .github/ci/tasks.yaml
    - .github/workflows/test.yaml
  new-from-merge:
    - PS-BOF/ps.axs
    - PS-BOF/README.md
    - PS-BOF/Makefile
    - PS-BOF/grep/grep.c
    - PS-BOF/kill/kill.c
    - PS-BOF/list/list.c
    - PS-BOF/resume/resume.c
    - PS-BOF/run/run.c
    - PS-BOF/suspend/suspend.c
---

## Summary

Plan 32-01 merged origin/main (PS-BOF commit 5cbcc03) into local main and converted `TK-BOF/make/make.c` to use `ADVAPI32$LogonUserW` with wide-char arguments instead of the ANSI `LogonUserA` variant.

## What Was Built

**Task 1 — Merge origin/main:** Merged commit 5cbcc03 ("Ps bof (#1)") into local main. Resolved one conflict in `Makefile` where local added `TK-BOF` and origin added `PS-BOF` to `SUBDIRS` — kept both. All other files auto-merged cleanly. Post-merge: `PS-BOF/` tree exists locally, `bof-collection.axs` contains the PS-BOF `script_load` line, `README.md` contains `## PS-BOF` and the Kharon credit line.

**Task 2 — bofdefs.h:** Replaced `ADVAPI32$LogonUserA(LPCSTR …)` declaration with `ADVAPI32$LogonUserW(LPCWSTR lpszUsername, LPCWSTR lpszDomain, LPCWSTR lpszPassword, DWORD dwLogonType, DWORD dwLogonProvider, PHANDLE phToken)`. Section header comment preserved. No other declarations changed.

**Task 3 — make.c:** Updated all four touch-points:
- Variable declarations: `char*` → `WCHAR*` for username, password, domain
- BeaconDataExtract casts: added `(WCHAR*)` cast on each extract
- Domain sentinel: `"."` → `L"."` with `const WCHAR *dom`
- API call: `ADVAPI32$LogonUserA` → `ADVAPI32$LogonUserW`
- Error string: "LogonUserA failed" → "LogonUserW failed"

Build result: all 12 targets built cleanly (`[+]` prefix, no `[!]` failures). `_bin/make.x64.o` and `_bin/make.x32.o` produced.

## Deviations

- `make.x86.o` referenced in plan acceptance criteria does not exist — the Makefile uses `x32` naming (`make.x32.o`). This is a plan typo; the binary was produced correctly.

## Self-Check: PASSED

All must-haves verified:
- `git merge-base --is-ancestor origin/main HEAD` exits 0
- `PS-BOF/ps.axs` exists
- `bof-collection.axs` contains `ax.script_load(path + "PS-BOF/ps.axs")`
- `README.md` contains `## PS-BOF` and `Kharon`
- `TK-BOF/bofdefs.h` contains `ADVAPI32$LogonUserW` and not `LogonUserA`
- `TK-BOF/make/make.c` uses `WCHAR*` vars, `(WCHAR*)` casts, `L"."`, calls `ADVAPI32$LogonUserW`
- Build produces both object files with no `[!]` failures
- `grep -rF 'LogonUserA' TK-BOF/` returns no matches
- No `TK-BOF/tk.axs` entry in `bof-collection.axs` (Wave 2 adds it)
