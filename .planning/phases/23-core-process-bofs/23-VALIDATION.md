---
phase: 23
slug: core-process-bofs
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-05-16
---

# Phase 23 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | make (cross-compile build system, MinGW-w64) |
| **Config file** | PS-BOF/Makefile |
| **Quick run command** | `make -C PS-BOF clean all 2>&1 \| grep -E '(error\|warning\|\.o$)'` |
| **Full suite command** | `make -C PS-BOF clean all && ls PS-BOF/_bin/*.o` |
| **Estimated runtime** | ~5 seconds |

---

## Sampling Rate

- **After every task commit:** Run `make -C PS-BOF clean all 2>&1 | grep -E '(error|warning|\.o$)'`
- **After every plan wave:** Run `make -C PS-BOF clean all && ls PS-BOF/_bin/*.o`
- **Before `/gsd-verify-work`:** Full suite must be green (all 8 .o files present)
- **Max feedback latency:** ~5 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 23-01-01 | 01 | 1 | PS-01, PB-01 | — | N/A | build | `make -C PS-BOF clean all && ls PS-BOF/_bin/list.x64.o PS-BOF/_bin/list.x86.o` | ✅ W0 | ⬜ pending |
| 23-02-01 | 02 | 1 | PS-02 | — | N/A | build | `make -C PS-BOF clean all && ls PS-BOF/_bin/kill.x64.o PS-BOF/_bin/kill.x86.o` | ✅ W0 | ⬜ pending |
| 23-03-01 | 03 | 2 | PS-08, PS-09 | — | N/A | build | `make -C PS-BOF clean all && ls PS-BOF/_bin/suspend.x64.o PS-BOF/_bin/resume.x64.o` | ✅ W0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- No new test infrastructure needed — build system already exists via PS-BOF/Makefile.

*Existing infrastructure covers all phase requirements. Validation is build-success + binary artifact presence.*

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| ps list output populates Process Browser | PB-01 | Requires live Adaptix C2 + beacon | Run `ps list` via Adaptix beacon; verify Process Browser table shows Name/PID/PPID/Session/User/Arch |
| ps kill terminates target process | PS-02 | Requires live Windows target | Run `ps kill <PID>`; verify process disappears from subsequent `ps list` |
| ps suspend/resume change process state | PS-08, PS-09 | Requires live Windows target | Run `ps suspend <PID>`; verify process is suspended; `ps resume <PID>` returns it to running |
| owner shows N/A for protected processes | PS-01 | Requires live Windows target | System/smss/csrss should show N/A for user; no 4703 audit event generated |

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 10s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
