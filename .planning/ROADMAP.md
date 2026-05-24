# Roadmap: BOF-Collection

## Milestones

- [x] **v1.0 — FS-BOF** ✅ SHIPPED 2026-04-08 — 7 phases, 14 plans, 71 files — New FS-BOF category (dir, type, mkdir, copy, move, del) migrated from SAL-BOF and extended with wildcard/UNC support. See [v1.0 archive](milestones/v1.0-fs-bof.md).
- [x] **v1.1 — Exit BOFs** ✅ SHIPPED 2026-04-09 — 2 phases, 6 plans, 8 files — exitprocess and exitthread BOFs in Postex-BOF; x32 i686 toolchain fix; typed exit commands registered beacon-only. See [v1.1 archive](milestones/v1.1-exit-bobs.md).
- [x] **v1.2 — Fix Compilation on Various Systems** ✅ SHIPPED 2026-04-20 — 3 phases, 7 plans, 24 files — Cross-platform compilation fixes for Arch GCC 15.2 / MinGW; CI workflow; Docker build verified. See [v1.2 archive](milestones/v1.2-ROADMAP.md).
- [x] **v1.3 — BOF-Collection Spinoff** ✅ SHIPPED 2026-05-03 — 5 phases, 11 plans, 37 files — Standalone AdaptixC2 BOF collection with FS-BOFs, Exit BOFs, CI/CD, and documentation. See [v1.3 archive](milestones/v1.3-ROADMAP.md).
- [x] **v1.4 — Testing** ✅ SHIPPED 2026-05-15 — 4 phases, 9 plans — PowerShell reference script, 38-entry tasks.yaml suite (38/38 pass), BOF error-string fixes, GitHub Actions CI/CD. See [v1.4 archive](milestones/v1.4-ROADMAP.md).
- [x] **v1.5 — PS-BOF** ✅ SHIPPED 2026-05-22 — 7 phases (22–28), 11 plans — Process management BOFs ported from Kharon: ps list/kill/run/grep/suspend/resume, Adaptix Process Browser integration, CI/CD test coverage. See [v1.5 archive](milestones/v1.5-ROADMAP.md).
- [ ] **v1.6 — TK-BOF** — 5 phases (29–33) — Token management BOFs ported from Kharon: tk steal/use/make/rm/revert/privget, tk.axs wiring, documentation, CI/CD coverage.

## Phases

<details>
<summary>✅ v1.0 FS-BOF (Phases 1–7) — SHIPPED 2026-04-08</summary>

See [v1.0 archive](milestones/v1.0-fs-bof.md) for full phase details.

</details>

<details>
<summary>✅ v1.1 Exit BOFs (Phases 8–9) — SHIPPED 2026-04-09</summary>

See [v1.1 archive](milestones/v1.1-exit-bofs.md) for full phase details.

</details>

<details>
<summary>✅ v1.2 Fix Compilation on Various Systems (Phases 10–12) — SHIPPED 2026-04-20</summary>

- [x] Phase 10: Known Fixes — 3/3 plans (completed 2026-04-20)
- [x] Phase 11: Full Audit — 2/2 plans (completed 2026-04-20)
- [x] Phase 12: Test Infrastructure & Docker Verification — 2/2 plans (completed 2026-04-20)

See [v1.2 archive](milestones/v1.2-ROADMAP.md) for full phase details.

</details>

<details>
<summary>✅ v1.3 BOF-Collection Spinoff (Phases 13–17) — SHIPPED 2026-05-03</summary>

- [x] Phase 13: Repo Setup — 3/3 plans (completed 2026-05-01)
- [x] Phase 14: Port FS-BOFs — 3/3 plans (completed 2026-05-01)
- [x] Phase 15: Port Exit BOFs — 1/1 plans (completed 2026-05-02)
- [x] Phase 16: Agent Scripts & CI/CD — 2/2 plans (completed 2026-05-03)
- [x] Phase 17: Documentation — 2/2 plans (completed 2026-05-03)

See [v1.3 archive](milestones/v1.3-ROADMAP.md) for full phase details.

</details>

<details>
<summary>✅ v1.4 Testing (Phases 18–21) — SHIPPED 2026-05-15</summary>

- [x] Phase 18: PowerShell Reference Script — 2/2 plans (completed 2026-05-06)
- [x] Phase 19: BOF Test Suite (tasks.yaml) — 2/2 plans (completed 2026-05-07)
- [x] Phase 20: Run & Validate Tests — 4/4 plans (completed 2026-05-15)
- [x] Phase 21: CI/CD Automation — 1/1 plan (completed 2026-05-15)

See [v1.4 archive](milestones/v1.4-ROADMAP.md) for full phase details.

</details>

<details>
<summary>✅ v1.5 PS-BOF (Phases 22–28) — SHIPPED 2026-05-22</summary>

- [x] Phase 22: PS-BOF Setup (1/1 plans) — completed 2026-05-16
- [x] Phase 23: Core Process BOFs (3/3 plans) — completed 2026-05-16
- [x] Phase 24: ps run (2/2 plans) — completed 2026-05-18
- [x] Phase 25: ps grep (1/1 plan) — completed 2026-05-20
- [x] Phase 26: ps.axs + Process Browser (1/1 plan) — completed 2026-05-20
- [x] Phase 27: Documentation (1/1 plan) — completed 2026-05-21
- [x] Phase 28: CI/CD Tests (2/2 plans) — completed 2026-05-22

See [v1.5 archive](milestones/v1.5-ROADMAP.md) for full phase details.

</details>

### v1.6 TK-BOF (Phases 29–33)

- [x] **Phase 29: TK-BOF Setup** — Build skeleton: directory layout, Makefile (x64+x32), bofdefs.h with ADVAPI32$/NTDLL$ dynamic resolution declarations (completed 2026-05-23)
- [x] **Phase 30: Core Token BOFs** — tk steal, tk use, tk rm, tk revert (steal/impersonate/close/revert token operations) (completed 2026-05-23)
- [x] **Phase 31: tk make + tk privget** — LogonUser credential token creation and AdjustTokenPrivileges privilege elevation (completed 2026-05-24)
- [x] **Phase 32: tk.axs + Documentation** — Subcommand wiring, TK-BOF README, root README update (completed 2026-05-24)
- [ ] **Phase 33: CI/CD Tests** — tasks.yaml entries and test.yaml deploy block for TK-BOF

## Phase Details

### Phase 29: TK-BOF Setup
**Goal**: The TK-BOF build skeleton exists and all 12 targets (x64+x32 for 6 commands) compile cleanly
**Depends on**: Phase 28 (v1.5 complete)
**Requirements**: TK-07
**Success Criteria** (what must be TRUE):
  1. `TK-BOF/` directory exists with one subdirectory per command (steal, use, make, rm, revert, privget)
  2. `TK-BOF/Makefile` builds all 12 targets (6 commands x x64+x32) with zero errors
  3. `TK-BOF/bofdefs.h` declares ADVAPI32$ and NTDLL$ dynamic resolution for all token-related Win32 APIs used across the 6 BOFs
  4. `make` in `TK-BOF/` produces `.o` files for all 12 targets with no `[!]` failures
**Plans**: 1 plan
Plans:
- [x] 29-01-PLAN.md — Create TK-BOF bofdefs.h contracts, 6 silent stubs with arg scaffolding, Makefile (12 targets), wire root SUBDIRS

### Phase 30: Core Token BOFs
**Goal**: Operators can steal, impersonate, close, and revert tokens through the beacon
**Depends on**: Phase 29
**Requirements**: TK-01, TK-02, TK-04, TK-05
**Success Criteria** (what must be TRUE):
  1. Operator runs `tk steal <pid>` and receives a token handle value printed to beacon output; impersonation is immediately active
  2. Operator runs `tk steal <pid> --no-apply` and receives a handle value without impersonation being applied
  3. Operator runs `tk use <handle>` with a previously printed handle and impersonation switches to that token
  4. Operator runs `tk rm <handle>` and the kernel object is freed (NtClose called); subsequent use of that handle fails
  5. Operator runs `tk revert` and impersonation is dropped back to the process token (RevertToSelf)
**Plans**: 2 plans
Plans:
**Wave 1**
- [x] 30-01-PLAN.md — Create TK-BOF/tkerror.h helper; implement steal.c (OpenProcess → OpenProcessToken → DuplicateTokenEx → optional ImpersonateLoggedOnUser, with handle-leak cleanup)

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 30-02-PLAN.md — Implement use.c (ImpersonateLoggedOnUser + TkErrorMessage), rm.c (NtClose + raw NTSTATUS hex), revert.c (RevertToSelf, no error branch)

### Phase 31: tk make + tk privget
**Goal**: Operators can create tokens from credentials and enable all privileges on the current token
**Depends on**: Phase 30
**Requirements**: TK-03, TK-06
**Success Criteria** (what must be TRUE):
  1. Operator runs `tk make --username <u> --password <p>` and receives a handle value; impersonation as that user is immediately active
  2. Operator runs `tk make --username <u> --password <p> --domain <d>` and the domain credential is used for LogonUser
  3. Operator runs `tk make --username <u> --password <p> --no-apply` and receives a handle without impersonation being applied
  4. Operator runs `tk privget` and all available privileges on the current token are enabled via AdjustTokenPrivileges
**Plans**: 2 plans
Plans:
**Wave 1**
- [x] 31-01-PLAN.md — Add 3 missing declarations to TK-BOF/bofdefs.h; implement make.c (LogonUserA + optional ImpersonateLoggedOnUser + handle print)

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 31-02-PLAN.md — Implement privget.c (OpenThreadToken/OpenProcessToken fallback, two-pass GetTokenInformation, AdjustTokenPrivileges with ERROR_NOT_ALL_ASSIGNED warning)

### Phase 32: tk.axs + Documentation
**Goal**: All 6 tk subcommands are registered in Adaptix and documentation is complete
**Depends on**: Phase 31
**Requirements**: TK-08, TK-09, TK-10
**Success Criteria** (what must be TRUE):
  1. `tk.axs` registers all 6 subcommands (steal, use, make, rm, revert, privget) beacon-only with correct argument definitions
  2. `TK-BOF/README.md` contains a command table with usage examples for all 6 commands, a handle lifecycle note explaining when to use `tk rm` vs `tk revert`, and Kharon attribution
  3. Root `README.md` includes the TK-BOF category in its BOF table and the Kharon credit is updated to include token management
**Plans**: 3 plans
**UI hint**: no
Plans:
**Wave 1**
- [x] 32-01-PLAN.md — Merge origin/main (PS-BOF reconciliation) + convert TK-BOF/make/make.c to LogonUserW with WCHAR* args and update TK-BOF/bofdefs.h

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 32-02-PLAN.md — Create TK-BOF/tk.axs registering all 6 subcommands beacon-only + add TK-BOF script_load to bof-collection.axs
- [x] 32-03-PLAN.md — Write TK-BOF/README.md (handle lifecycle + per-command sections) and update root README.md (## TK-BOF table + extended Kharon credit)

### Phase 33: CI/CD Tests
**Goal**: TK-BOF operations are covered by automated CI test entries
**Depends on**: Phase 32
**Requirements**: TK-11, TK-12
**Success Criteria** (what must be TRUE):
  1. `tasks.yaml` contains entries that steal a token from a known process, verify impersonation via `whoami`, revert, create a token with local credentials via `tk make`, and enable privileges via `tk privget`
  2. `test.yaml` deploy block includes the TK-BOF directory so CI builds and deploys TK-BOF artifacts alongside other categories
  3. All new `tasks.yaml` entries pass when run against a live Adaptix beacon
**Plans**: TBD

## Progress

| Phase | Milestone | Plans Complete | Status | Completed |
|-------|-----------|----------------|--------|-----------|
| 1–7. FS-BOF | v1.0 | 14/14 | Complete | 2026-04-08 |
| 8–9. Exit BOFs | v1.1 | 6/6 | Complete | 2026-04-09 |
| 10. Known Fixes | v1.2 | 3/3 | Complete | 2026-04-20 |
| 11. Full Audit | v1.2 | 2/2 | Complete | 2026-04-20 |
| 12. Test Infrastructure | v1.2 | 2/2 | Complete | 2026-04-20 |
| 13. Repo Setup | v1.3 | 3/3 | Complete | 2026-05-01 |
| 14. Port FS-BOFs | v1.3 | 3/3 | Complete | 2026-05-01 |
| 15. Port Exit BOFs | v1.3 | 1/1 | Complete | 2026-05-02 |
| 16. Agent Scripts & CI/CD | v1.3 | 2/2 | Complete | 2026-05-03 |
| 17. Documentation | v1.3 | 2/2 | Complete | 2026-05-03 |
| 18. PowerShell Reference Script | v1.4 | 2/2 | Complete | 2026-05-06 |
| 19. BOF Test Suite (tasks.yaml) | v1.4 | 2/2 | Complete | 2026-05-07 |
| 20. Run & Validate Tests | v1.4 | 4/4 | Complete | 2026-05-15 |
| 21. CI/CD Automation | v1.4 | 1/1 | Complete | 2026-05-15 |
| 22. PS-BOF Setup | v1.5 | 1/1 | Complete | 2026-05-16 |
| 23. Core Process BOFs | v1.5 | 3/3 | Complete | 2026-05-16 |
| 24. ps run | v1.5 | 2/2 | Complete | 2026-05-18 |
| 25. ps grep | v1.5 | 1/1 | Complete | 2026-05-20 |
| 26. ps.axs + Process Browser | v1.5 | 1/1 | Complete | 2026-05-20 |
| 27. Documentation | v1.5 | 1/1 | Complete | 2026-05-21 |
| 28. CI/CD Tests | v1.5 | 2/2 | Complete | 2026-05-22 |
| 29. TK-BOF Setup | v1.6 | 1/1 | Complete   | 2026-05-23 |
| 30. Core Token BOFs | v1.6 | 2/2 | Complete   | 2026-05-23 |
| 31. tk make + tk privget | v1.6 | 2/2 | Complete   | 2026-05-24 |
| 32. tk.axs + Documentation | v1.6 | 3/3 | Complete   | 2026-05-24 |
| 33. CI/CD Tests | v1.6 | 0/1 | Not started | — |
