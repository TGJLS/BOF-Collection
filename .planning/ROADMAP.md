# Roadmap: BOF-Collection

## Milestones

- [x] **v1.0 — FS-BOF** ✅ SHIPPED 2026-04-08 — 7 phases, 14 plans, 71 files — New FS-BOF category (dir, type, mkdir, copy, move, del) migrated from SAL-BOF and extended with wildcard/UNC support. See [v1.0 archive](milestones/v1.0-fs-bof.md).
- [x] **v1.1 — Exit BOFs** ✅ SHIPPED 2026-04-09 — 2 phases, 6 plans, 8 files — exitprocess and exitthread BOFs in Postex-BOF; x32 i686 toolchain fix; typed exit commands registered beacon-only. See [v1.1 archive](milestones/v1.1-exit-bobs.md).
- [x] **v1.2 — Fix Compilation on Various Systems** ✅ SHIPPED 2026-04-20 — 3 phases, 7 plans, 24 files — Cross-platform compilation fixes for Arch GCC 15.2 / MinGW; CI workflow; Docker build verified. See [v1.2 archive](milestones/v1.2-ROADMAP.md).
- [x] **v1.3 — BOF-Collection Spinoff** ✅ SHIPPED 2026-05-03 — 5 phases, 11 plans, 37 files — Standalone AdaptixC2 BOF collection with FS-BOFs, Exit BOFs, CI/CD, and documentation. See [v1.3 archive](milestones/v1.3-ROADMAP.md).
- [x] **v1.4 — Testing** ✅ SHIPPED 2026-05-15 — 4 phases, 9 plans — PowerShell reference script, 38-entry tasks.yaml suite (38/38 pass), BOF error-string fixes, GitHub Actions CI/CD. See [v1.4 archive](milestones/v1.4-ROADMAP.md).
- [ ] **v1.5 — PS-BOF** — 7 phases (22–28) — Process management BOFs ported from Kharon: ps list/kill/run/grep/suspend/resume, Adaptix Process Browser integration, CI/CD test coverage.

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

### v1.5 PS-BOF (Phases 22–28)

- [x] **Phase 22: PS-BOF Setup** — PS-BOF directory, Makefile skeleton, and NT/PSAPI API declarations in bofdefs.h
- [x] **Phase 23: Core Process BOFs** — ps list, ps kill, ps suspend, ps resume (simple BOFs + Adaptix process format) (completed 2026-05-16)
- [x] **Phase 24: ps run** — Process creation BOF with CreateProcess, WithLogon, WithToken, and PPID spoofing (completed 2026-05-18)
- [x] **Phase 25: ps grep** — Process inspector BOF: token, modules, command line, threads
- [ ] **Phase 26: ps.axs + Process Browser** — Adaptix script wiring all 6 commands plus Process Browser integration
- [ ] **Phase 27: Documentation** — PS-BOF README and root README update with Kharon credit
- [ ] **Phase 28: CI/CD Tests** — tasks.yaml entries for all PS-BOF commands and GitHub Actions workflow update

## Phase Details

### Phase 22: PS-BOF Setup
**Goal**: The PS-BOF category exists as a buildable skeleton — directory structure, Makefile, and all required NT/PSAPI API declarations added to bofdefs.h — so subsequent phases can implement BOFs without infrastructure work.
**Depends on**: Phase 21 (v1.4 complete)
**Requirements**: (none — pure infrastructure; unblocks PS-01 through PS-09)
**Success Criteria** (what must be TRUE):
  1. `make` run from the PS-BOF directory exits 0 with no BOF .o files missing from the expected target list
  2. bofdefs.h contains declarations for NtQuerySystemInformation, NtSuspendProcess, NtResumeProcess, EnumProcessModulesEx, GetModuleFileNameExW, and GetModuleInformation resolvable via NTDLL$/PSAPI$ prefixes
  3. The PS-BOF Makefile follows the same pattern as FS-BOF and Postex-BOF Makefiles (x64/x32 targets, -Os, strip)
**Plans**: 1 plan
  - [x] 22-01-PLAN.md — PS-BOF directory tree, Makefile, 6 stub sources, bofdefs.h NT/PSAPI extensions, root Makefile SUBDIRS wiring

### Phase 23: Core Process BOFs
**Goal**: Operators can list all running processes, kill a process by PID, and suspend or resume a process by PID using standard BOF output compatible with any C2 framework.
**Depends on**: Phase 22
**Requirements**: PS-01, PS-02, PS-08, PS-09
**Success Criteria** (what must be TRUE):
  1. Operator runs `ps list` and sees a table of all running processes with name, PID, PPID, session ID, owner (domain\user), and architecture
  2. Operator runs `ps kill <PID>` (and optionally `ps kill <PID> <exit_code>`) and the target process is no longer visible in a subsequent `ps list` output
  3. Operator runs `ps suspend <PID>` and the target process enters a suspended state; `ps resume <PID>` returns it to running
**Plans**: 3 plans
  - [x] 23-01-PLAN.md — Add 10 declarations to `_include/bofdefs.h` (KERNEL32/ADVAPI32/NTDLL/MSVCRT$malloc); port `list.cc` → `PS-BOF/list/list.c` (NtQuerySystemInformation loop + GetUserByToken goto-cleanup + BeaconPrintf text table output) — Wave 1
  - [x] 23-02-PLAN.md — Port `kill.cc` → `PS-BOF/kill/kill.c` (BeaconDataInt PID + optional exit_code; KERNEL32$OpenProcess(PROCESS_TERMINATE) + KERNEL32$TerminateProcess) — Wave 2 (depends on 23-01)
  - [x] 23-03-PLAN.md — Implement `suspend.c` and `resume.c` from scratch per D-08 (BeaconDataInt PID; KERNEL32$OpenProcess(PROCESS_SUSPEND_RESUME); NTDLL$NtSuspendProcess / NTDLL$NtResumeProcess) — Wave 2 (depends on 23-01)

### Phase 24: ps run
**Goal**: Operators can launch a new process using any of three creation methods — default CreateProcess, CreateProcessWithLogon with supplied credentials, or CreateProcessWithToken using a stolen handle — with optional PPID spoofing and stdout/stderr capture.
**Depends on**: Phase 22
**Requirements**: PS-03, PS-04, PS-05, PS-06
**Success Criteria** (what must be TRUE):
  1. Operator runs `ps run --command "cmd.exe"` and a new cmd.exe process appears in `ps list` output
  2. Operator runs `ps run --command "cmd.exe /c whoami" --pipe true` and the captured output is returned in the beacon response
  3. Operator runs `ps run --command "notepad.exe" --state suspended` and the process launches suspended
  4. Operator runs `ps run --command "notepad.exe" --domain CORP --username admin --password Secret` and the process launches as the specified user
  5. Operator runs `ps run --command "notepad.exe" --token <handle>` and the process launches under the stolen token's identity
  6. Operator passes `--ppid <PID>` and the spawned process reports the specified PID as its parent
  7. The Adaptix ps.axs command for ps run exposes exactly these flags: --command, --state, --pipe, --domain, --username, --password, --token (matching Kharon's ax_config.axs process run definition)
**Plans**: TBD

### Phase 25: ps grep
**Goal**: Operators can inspect any running process by PID and receive its token user, elevation level, integrity level, loaded modules with base addresses and sizes, command line, and thread list.
**Depends on**: Phase 22
**Requirements**: PS-07
**Success Criteria** (what must be TRUE):
  1. Operator runs `ps grep <PID>` and the output includes the process token owner (domain\user), elevation type, and integrity level
  2. Output includes a list of loaded modules with name, base address, and size for each
  3. Output includes the process command-line string
  4. Output includes a list of thread IDs for the process
**Plans**: TBD

### Phase 26: ps.axs + Process Browser
**Goal**: All PS-BOF commands are accessible to operators via the Adaptix agent script, and the Adaptix Process Browser opens and auto-populates by running ps list against the active beacon session.
**Note**: ps list outputs a plain-text table via BeaconPrintf (not the Adaptix binary format). Process Browser integration will require ps list to produce structured binary output (BeaconPkgBytes/BeaconPkgInt32) — this phase must decide whether to add a separate binary-output variant or wire the browser against the text output with a parser.
**Depends on**: Phase 23 (ps list must exist for Process Browser to wire against)
**Requirements**: PB-02, PB-03
**Success Criteria** (what must be TRUE):
  1. Operator types `ps list`, `ps kill`, `ps run`, `ps grep`, `ps suspend`, or `ps resume` in an Adaptix beacon session and the corresponding BOF executes
  2. Operator opens the Adaptix Process Browser for a beacon session and the browser populates with the process list without a separate manual invocation
  3. ps.axs registers `ax.open_browser_process` as a menu action visible in the beacon session context menu
**Plans**: TBD

### Phase 27: Documentation
**Goal**: Operators can discover and understand all PS-BOF commands from the repository README and the PS-BOF category README, and Kharon is credited as the upstream source.
**Depends on**: Phase 26 (all commands stable before documenting)
**Requirements**: DOCS-01, DOCS-02
**Success Criteria** (what must be TRUE):
  1. PS-BOF/README.md contains a BOF command table listing ps list, ps kill, ps run, ps grep, ps suspend, and ps resume with usage examples for each
  2. Root README.md contains a PS-BOF row in the category table and a credit line for Kharon alongside the existing Extension Kit credit
**Plans**: TBD

### Phase 28: CI/CD Tests
**Goal**: The test suite covers all six PS-BOF commands with live beacon verification, and GitHub Actions runs the full suite on push and pull requests.
**Depends on**: Phase 26 (all BOF commands must exist and be wired via ps.axs)
**Requirements**: CI-01, CI-02, CI-03, CI-04, CI-05, CI-06, CI-07
**Success Criteria** (what must be TRUE):
  1. `ps list` test entry in tasks.yaml passes: output contains at least "System" and "lsass.exe"
  2. `ps run` + `ps kill` test sequence passes: a process is spawned, its PID captured, and killing it returns no error
  3. `ps run --pipe` test passes: `cmd /c whoami` output contains the beacon session username
  4. `ps grep` test passes: inspecting a known PID returns non-empty token, module, cmdline, and thread sections
  5. `ps suspend` and `ps resume` test entries pass: both commands execute against a spawned process with no error output
  6. GitHub Actions test.yaml runs the PS-BOF test cases on push/PR to main and dev branches
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
| 22. PS-BOF Setup | v1.5 | 1/1 | Complete    | 2026-05-16 |
| 23. Core Process BOFs | v1.5 | 3/3 | Complete    | 2026-05-16 |
| 24. ps run | v1.5 | 2/2 | Complete   | 2026-05-18 |
| 25. ps grep | v1.5 | 0/? | Not started | - |
| 26. ps.axs + Process Browser | v1.5 | 0/? | Not started | - |
| 27. Documentation | v1.5 | 0/? | Not started | - |
| 28. CI/CD Tests | v1.5 | 0/? | Not started | - |
