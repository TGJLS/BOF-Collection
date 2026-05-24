---
phase: 32-tk-axs-documentation
verified: 2026-05-24T00:00:00Z
status: gaps_found
score: 5/7 must-haves verified
overrides_applied: 0
gaps:
  - truth: "TK-BOF/README.md contains a command table with usage examples for all 6 commands"
    status: failed
    reason: "ROADMAP SC-2 requires a command table in TK-BOF/README.md. The file has per-command sections with fenced code blocks but no Markdown table. Plan 03 deliberately omitted the table following FS-BOF/README.md precedent — but this deviates from the ROADMAP contract, which explicitly says 'command table'. The PLAN must_haves narrowed scope below the roadmap success criterion."
    artifacts:
      - path: "TK-BOF/README.md"
        issue: "No Markdown table present. Per-command usage in fenced code blocks only. ROADMAP SC-2 mandates a command table."
    missing:
      - "Add a Markdown table (|Commands|Usage|Notes| style) listing all 6 subcommands with usage and description, positioned before or within the per-command sections"
  - truth: "TK-BOF/README.md contains Kharon attribution"
    status: failed
    reason: "ROADMAP SC-2 explicitly requires Kharon attribution in TK-BOF/README.md. The Kharon credit exists only in root README.md (Credits section). Plan 03 chose to place attribution only in root README.md per FS-BOF/README.md precedent. FS-BOF/README.md also lacks Kharon attribution — but TK-BOF is explicitly called out in ROADMAP SC-2 as needing it in the per-category README."
    artifacts:
      - path: "TK-BOF/README.md"
        issue: "No Kharon attribution anywhere in this file. Root README.md line 93 has the credit but ROADMAP SC-2 mandates it in TK-BOF/README.md."
    missing:
      - "Add a Credits or attribution line in TK-BOF/README.md referencing Kharon (https://github.com/entropy-z/Kharon)"
---

# Phase 32: tk.axs + Documentation Verification Report

**Phase Goal:** All 6 tk subcommands are registered in Adaptix and documentation is complete
**Verified:** 2026-05-24
**Status:** gaps_found
**Re-verification:** No — initial verification

## Goal Achievement

### Observable Truths

Truths are derived from two sources: ROADMAP.md Phase 32 Success Criteria (SC-1, SC-2, SC-3) and the PLAN frontmatter must_haves across all three plans. Per verification rules, ROADMAP success criteria are the non-negotiable contract. PLAN must_haves cannot reduce scope below roadmap SCs.

**ROADMAP Phase 32 Success Criteria:**
1. SC-1: `tk.axs` registers all 6 subcommands beacon-only with correct argument definitions
2. SC-2: `TK-BOF/README.md` contains a command table with usage examples for all 6 commands, a handle lifecycle note, and Kharon attribution
3. SC-3: Root `README.md` includes the TK-BOF category table and Kharon credit is updated to include token management

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | tk.axs exists and registers all 6 subcommands (steal, use, make, rm, revert, privget) beacon-only under parent `tk` | VERIFIED | `TK-BOF/tk.axs` exists (68 lines); `ax.create_command(` count = 7 (6 subcommands + 1 parent); `ax.register_commands_group(group_tk, ["beacon"], ["windows"], [])` confirmed |
| 2 | Each subcommand's bof_pack format string matches its .c file's BeaconDataParse arg order | VERIFIED | steal: `"int,int"` [pid, no_apply]; make: `"wstr,wstr,wstr,int,int"` [username, password, domain, no_apply, logon_type]; use/rm: `"int"` [token_handle] x2; revert/privget: no pack call (no args) |
| 3 | TK-BOF/README.md contains a command table with usage examples for all 6 commands | FAILED | File has per-command `## <cmd>` sections with fenced code blocks, but no Markdown table. ROADMAP SC-2 explicitly requires "a command table". |
| 4 | TK-BOF/README.md contains a handle lifecycle note and Kharon attribution | PARTIAL | Handle lifecycle: VERIFIED (`## Handle Lifecycle` section present before `## steal`, verbatim text confirmed). Kharon attribution: FAILED — no attribution in TK-BOF/README.md; ROADMAP SC-2 explicitly requires it here. |
| 5 | Root README.md contains a TK-BOF category table between PS-BOF and Credits | VERIFIED | `## TK-BOF` section at line 77; `## PS-BOF` at 64, `## Credits` at 90; 6-row table with `|Commands|Usage|Notes|` header confirmed |
| 6 | Root README.md Kharon credit updated to cover both PS-BOF and TK-BOF | VERIFIED | Line 93: `- [Kharon](https://github.com/entropy-z/Kharon): PS-BOF and TK-BOF command implementations`; old `PS-BOF command implementations`-only line absent |
| 7 | TK-BOF/make/make.c uses WCHAR* + LogonUserW, bofdefs.h declares LogonUserW (no LogonUserA) | VERIFIED | `WCHAR *username/password/domain` confirmed; `(WCHAR*) BeaconDataExtract` casts; `L"."` sentinel; `ADVAPI32$LogonUserW` call; LogonUserA absent from all TK-BOF/ files |

**Score:** 5/7 truths verified (2 failed — both in ROADMAP SC-2)

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `TK-BOF/tk.axs` | Adaptix axs script, 6 subcommands, beacon-only | VERIFIED | 68 lines; 7 create_command() calls; group registered `["beacon"], ["windows"], []`; no int32/int64; no gopher/kharon strings |
| `bof-collection.axs` | 4 script_load calls; TK-BOF after PS-BOF | VERIFIED | Exactly 4 script_load calls; PS-BOF line at position 3, TK-BOF at position 4; ordering confirmed by awk |
| `TK-BOF/README.md` | Per-command docs, handle lifecycle, command table, Kharon credit | PARTIAL (STUB on two items) | Handle lifecycle and per-command sections exist (59 lines, >= 50 min). No command table. No Kharon attribution. Two ROADMAP SC-2 requirements unmet. |
| `README.md` | TK-BOF section between PS-BOF and Credits, updated Kharon credit | VERIFIED | Section present; 6-row table confirmed; Kharon line updated; ordering PS-BOF(64) < TK-BOF(77) < Credits(90) |
| `TK-BOF/bofdefs.h` | ADVAPI32$LogonUserW with LPCWSTR args; no LogonUserA | VERIFIED | Declaration present with exact wide signature; section header preserved; LogonUserA absent |
| `TK-BOF/make/make.c` | WCHAR* vars, LogonUserW call, L"." sentinel | VERIFIED | All 4 touch-points updated; build artifacts `_bin/make.x64.o` and `_bin/make.x32.o` exist |
| `PS-BOF/` tree | Present from merge of origin/main | VERIFIED | ps.axs, README.md, Makefile confirmed; `git merge-base --is-ancestor origin/main HEAD` exits 0 |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| TK-BOF/tk.axs (steal hook) | TK-BOF/_bin/steal.<arch>.o | `ax.script_dir() + "_bin/steal." + ax.arch(id) + ".o"` | VERIFIED | `_bin/steal.` pattern confirmed in tk.axs |
| TK-BOF/tk.axs (make hook) | TK-BOF/_bin/make.<arch>.o | wstr,wstr,wstr,int,int bof_pack | VERIFIED | `ax.bof_pack("wstr,wstr,wstr,int,int", [username, password, domain, no_apply, logon_type])` confirmed; arg order matches make.c BeaconDataParse |
| bof-collection.axs | TK-BOF/tk.axs | ax.script_load | VERIFIED | `ax.script_load(path + "TK-BOF/tk.axs");` present; appears after PS-BOF line |
| README.md (## TK-BOF) | TK-BOF/README.md | `[More details](TK-BOF/README.md)` link | VERIFIED | Intro line contains `[More details](TK-BOF/README.md)`; TK-BOF/README.md exists |
| README.md (Credits) | https://github.com/entropy-z/Kharon | Kharon attribution covering PS-BOF and TK-BOF | VERIFIED | Exact line: `PS-BOF and TK-BOF command implementations`; one occurrence only |
| TK-BOF/make/make.c | TK-BOF/bofdefs.h | include + ADVAPI32$LogonUserW call | VERIFIED | `ADVAPI32$LogonUserW(username, dom, password, ...)` call matches declaration; no LogonUserA anywhere in TK-BOF/ |

### Data-Flow Trace (Level 4)

Not applicable. Phase 32 produces an Adaptix axs script (tk.axs) and documentation files. No dynamic data rendering — the axs commands execute BOF binaries whose runtime output flows through the Adaptix beacon channel, which cannot be verified without a live beacon target.

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| tk.axs has exactly 7 create_command() calls | `grep -cF 'ax.create_command(' TK-BOF/tk.axs` | 7 | PASS |
| bof-collection.axs has exactly 4 script_load calls | `grep -cF 'ax.script_load' bof-collection.axs` | 4 | PASS |
| TK-BOF/README.md has >= 50 lines | `wc -l TK-BOF/README.md` | 59 | PASS |
| No LogonUserA anywhere in TK-BOF/ | `grep -rF 'LogonUserA' TK-BOF/` | 0 matches | PASS |
| Build artifacts exist | `ls TK-BOF/_bin/make.x64.o TK-BOF/_bin/make.x32.o` | both present | PASS |

### Probe Execution

No probes declared in PLAN files. Phase 32 is axs script creation and documentation — no `scripts/*/tests/probe-*.sh` applicable.

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|-------------|------------|-------------|--------|----------|
| TK-08 | 32-01, 32-02 | `tk.axs` — all 6 subcommands registered beacon-only | SATISFIED | tk.axs exists with 7 create_command calls, beacon-only registration, correct bof_pack formats for all 6 commands |
| TK-09 | 32-03 | `TK-BOF/README.md` — command table, usage examples, handle lifecycle note, Kharon credit | BLOCKED | Handle lifecycle and per-command usage sections present. Command table absent. Kharon attribution absent. ROADMAP SC-2 explicitly requires both in TK-BOF/README.md. |
| TK-10 | 32-03 | Root `README.md` — TK-BOF category table added, Kharon credit updated | SATISFIED | TK-BOF section with 6-row table present; Kharon credit reads "PS-BOF and TK-BOF command implementations"; section ordering correct |

**Orphaned requirements check:** No phase 32 requirements found in REQUIREMENTS.md beyond TK-08, TK-09, TK-10. All three are accounted for above.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| — | — | — | — | No TBD, FIXME, XXX, TODO, HACK, or PLACEHOLDER markers found in any file modified by this phase |

No debt markers found. No stub returns. No hardcoded empty values in rendering paths.

### Human Verification Required

None for automated checks. The following items require human action to resolve the gaps identified above:

**1. Command table in TK-BOF/README.md**

The ROADMAP SC-2 contract explicitly requires a command table in `TK-BOF/README.md`. The plan chose to omit this following FS-BOF/README.md precedent (no table in category README). Decision required: either add the table to `TK-BOF/README.md`, or override the ROADMAP success criterion with documented acceptance.

**2. Kharon attribution in TK-BOF/README.md**

The ROADMAP SC-2 contract explicitly requires Kharon attribution in `TK-BOF/README.md`. Attribution currently exists only in root README.md Credits section. Decision required: either add a Credits line to `TK-BOF/README.md`, or override.

### Gaps Summary

Two gaps block ROADMAP SC-2 for TK-09. Both missing items are in `TK-BOF/README.md`. The root cause is that Plan 03's must_haves narrowed scope below what ROADMAP.md SC-2 contracts — following the FS-BOF/README.md precedent, which has no table and no attribution. The precedent is reasonable but conflicts with an explicit ROADMAP requirement.

**Root cause is shared:** both gaps are in the same file and stem from the same plan decision (follow FS-BOF/README.md style). A single re-execution that adds a command table and a Kharon attribution line to `TK-BOF/README.md` would close both gaps.

**Suggested override (if precedent is accepted as intentional):**

If the operator accepts FS-BOF/README.md precedent as a deliberate design choice overriding ROADMAP SC-2 for the table and attribution items, add to this file's frontmatter:

```yaml
overrides:
  - must_have: "TK-BOF/README.md contains a command table with usage examples for all 6 commands"
    reason: "FS-BOF/README.md and other category READMEs intentionally omit the command table (table lives in root README only); applying same convention to TK-BOF/README.md"
    accepted_by: "operator"
    accepted_at: "2026-05-24T00:00:00Z"
  - must_have: "TK-BOF/README.md contains Kharon attribution"
    reason: "Attribution consolidated in root README.md Credits section covers all categories including TK-BOF; per-category README attribution is redundant"
    accepted_by: "operator"
    accepted_at: "2026-05-24T00:00:00Z"
```

---

_Verified: 2026-05-24_
_Verifier: Claude (gsd-verifier)_
