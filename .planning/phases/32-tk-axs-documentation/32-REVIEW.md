---
phase: 32-tk-axs-documentation
reviewed: 2026-05-24T00:00:00Z
depth: standard
files_reviewed: 8
files_reviewed_list:
  - TK-BOF/bofdefs.h
  - TK-BOF/make/make.c
  - TK-BOF/tk.axs
  - TK-BOF/README.md
  - bof-collection.axs
  - README.md
  - .github/ci/tasks.yaml
  - .github/workflows/test.yaml
findings:
  critical: 1
  warning: 2
  info: 1
  total: 4
status: issues_found
---

# Phase 32: Code Review Report

**Reviewed:** 2026-05-24
**Depth:** standard
**Files Reviewed:** 8
**Status:** issues_found

## Summary

This phase delivers the TK-BOF Adaptix integration: a WCHAR-based `make.c`, the `tk.axs` command script, `bof-collection.axs` loader update, and documentation. The C conversion and axs script are structurally correct — bof_pack format strings match C-side BeaconDataParse order, wide-char types and casts are used correctly, and the L"." fallback is in place. However, one CI blocker, one declaration-qualifier mismatch, one agent-scope narrowing, and one stale metadata description were found.

## Narrative Findings (AI reviewer)

## Critical Issues

### CR-01: CI workflow deploys bof-collection.axs referencing TK-BOF but never copies TK-BOF artifacts into the container

**File:** `.github/workflows/test.yaml:264-270`

**Issue:** The updated `bof-collection.axs` (line 11) adds `script_load(path + "TK-BOF/tk.axs")`. The CI workflow copies `bof-collection.axs` to the container at line 270, but the deployment block (lines 264-270) has no corresponding steps to copy `TK-BOF/_bin/*.o` or `TK-BOF/tk.axs` into the container's `BOF-Collection/TK-BOF/` directory. When the server loads `bof-collection.axs`, `script_load` for `tk.axs` will resolve relative to the container's BOF-Collection path — where neither `TK-BOF/tk.axs` nor `TK-BOF/_bin/` exists. This causes script load failure at server startup, which depending on Adaptix error handling could prevent the FS-BOF and PS-BOF commands (which load before TK-BOF) from registering as well.

**Fix:** Add the following deployment steps after the PS-BOF copy block (after line 269):

```bash
mkdir -p /tmp/adaptixc2/dist/BOF-Collection/TK-BOF/_bin
cp /workspace/TK-BOF/_bin/*.o /tmp/adaptixc2/dist/BOF-Collection/TK-BOF/_bin/
cp /workspace/TK-BOF/tk.axs /tmp/adaptixc2/dist/BOF-Collection/TK-BOF/
```

Also add a build verification check consistent with the existing pattern:

```bash
count=$(ls /workspace/TK-BOF/_bin/*.x64.o | wc -l)
[ "$count" -eq 6 ] && echo "✓ All 6 TK-BOF x64 objects compiled" || \
  { echo "✗ Expected 6 TK-BOF x64 objects, got $count"; exit 1; }
count=$(ls /tmp/adaptixc2/dist/BOF-Collection/TK-BOF/_bin/*.x64.o | wc -l)
[ "$count" -eq 6 ] && echo "✓ All 6 TK-BOF x64 objects deployed to container" || \
  { echo "✗ Expected 6 TK-BOF x64 objects in container bin, got $count"; exit 1; }
```

## Warnings

### WR-01: TK-BOF bofdefs.h redeclares symbols with WINBASEAPI that the shared bofdefs.h declares with WINADVAPI

**File:** `TK-BOF/bofdefs.h:9,27`

**Issue:** `TK-BOF/bofdefs.h` begins with `#include "../_include/bofdefs.h"` (line 2), which pulls in the shared header. The shared header declares `ADVAPI32$OpenProcessToken` (line 128) and `ADVAPI32$AdjustTokenPrivileges` (line 130) with `WINADVAPI`. `TK-BOF/bofdefs.h` then re-declares both with `WINBASEAPI` (lines 9 and 27). On MinGW, `WINADVAPI` expands to `__declspec(dllimport)` for the ADVAPI32 DLL, while `WINBASEAPI` expands to `__declspec(dllimport)` for KERNEL32. In practice both may expand identically in the MinGW toolchain used here, but the qualifier mismatch creates a redeclaration with semantically different storage-class attributes. A stricter compiler or toolchain update could treat this as an error. The correct qualifier for ADVAPI32 functions is `WINADVAPI`.

Similarly, `KERNEL32$GetCurrentProcess` is declared in both `_include/bofdefs.h` (line 105) and `TK-BOF/bofdefs.h` (line 38) with matching qualifiers — that duplicate is benign but unnecessary.

**Fix:** Remove the redundant re-declarations of `ADVAPI32$OpenProcessToken` and `ADVAPI32$AdjustTokenPrivileges` from `TK-BOF/bofdefs.h` entirely, since they are already provided by the included shared header. Change the qualifier on any remaining ADVAPI32 function declarations in `TK-BOF/bofdefs.h` from `WINBASEAPI` to `WINADVAPI` to match the actual DLL. Also remove the duplicate `KERNEL32$GetCurrentProcess` declaration.

### WR-02: tk.axs registers only for "beacon" agents, excluding "gopher" and "kharon" — inconsistent with every other module

**File:** `TK-BOF/tk.axs:68`

**Issue:** Line 68 registers the TK-BOF group for `["beacon"]` only. All other modules in the collection — FS-BOF (fs.axs line 80), Exit-BOF (exit.axs line 22), and PS-BOF (ps.axs line 89) — register for `["beacon", "gopher", "kharon"]`. Token manipulation is a BOF that executes inside whichever agent runs it; there is no functional reason to exclude gopher and kharon agents. Users running non-beacon agents will find `tk` commands silently absent from their command set.

The phase context notes "beacon-only registration" as intentional, but no design rationale distinguishes TK-BOF BOFs from PS-BOF BOFs with respect to agent type — both execute arbitrary Windows API calls in-process. If the intent truly is beacon-only, a comment explaining why should be added. If it was an oversight, the registration should be widened to match the other modules.

**Fix (if narrowing is unintentional):**
```javascript
ax.register_commands_group(group_tk, ["beacon", "gopher", "kharon"], ["windows"], []);
```

**Fix (if narrowing is intentional):** Add an inline comment:
```javascript
// beacon-only: token impersonation BOFs are only meaningful inside beacon agents
ax.register_commands_group(group_tk, ["beacon"], ["windows"], []);
```

## Info

### IN-01: bof-collection.axs metadata description does not mention TK-BOF after the new script_load was added

**File:** `bof-collection.axs:3`

**Issue:** The `description` field on line 3 reads `"Filesystem and process-control BOFs: type, mkdir, copy, move, del, rmdir, pwd, cd, exit process/thread"`. After this phase added `script_load(path + "TK-BOF/tk.axs")` on line 11, the description no longer reflects the full scope of what is loaded.

**Fix:**
```javascript
description: "Filesystem, process-control, and token management BOFs for AdaptixC2",
```

---

_Reviewed: 2026-05-24_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
