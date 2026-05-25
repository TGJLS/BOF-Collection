# Phase 33: CI/CD Tests - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-05-25
**Phase:** 33-ci-cd-tests
**Areas discussed:** Steal source PID, Impersonation verification, Handle capture + tk use flow, tk make credentials

---

## Steal source PID

| Option | Description | Selected |
|--------|-------------|----------|
| Reuse {{pid}} from ps-run-spawn-ping | Use existing PS-BOF fixture PID — no new process | |
| Spawn fresh process for TK-BOF | New ps run fixture, separate {{tk_pid}} capture | ✓ |
| System (PID 4) — always exists | Fixed known PID, but requires SeDebugPrivilege | |

**User's choice:** Spawn a fresh process for TK-BOF
**Notes:** Uses `ping -n 999 127.0.0.1` (same proven pattern as PS-BOF). Captures into `{{tk_pid}}`. No explicit cleanup — process dies with beacon exit.

---

## Impersonation verification

| Option | Description | Selected |
|--------|-------------|----------|
| Verify [+] Handle: in output | Match success string — reliable headless CI verification | ✓ |
| ps run whoami --pipe after steal | Known limitation: thread token doesn't propagate to new processes | |
| Skip verification, just no-error check | Weaker — only not_expected: "error" | |

**User's choice:** Verify [+] Handle: appears in output
**Notes:** `ImpersonateLoggedOnUser` sets thread token only; `ps run` spawns a new process that inherits the process token. Cross-process whoami verification is not viable here.

---

## Handle capture + tk use flow

| Option | Description | Selected |
|--------|-------------|----------|
| Test full handle lifecycle (steal --no-apply + use + rm) | Captures {{tk_handle}}, tests full operator workflow | ✓ |
| Just steal (immediate impersonation) | Simpler, no handle capture | |
| Both immediate and deferred impersonation | Maximum coverage | ✓ (both selected) |

**User's choice:** Yes — test both immediate steal AND the --no-apply + use + rm flow
**Notes:** Two separate steal tests: `tk steal {{tk_pid}}` (immediate, verify handle, then revert) and `tk steal {{tk_pid}} --no-apply` (capture {{tk_handle}}, then use, then rm).

---

## tk make credentials

| Option | Description | Selected |
|--------|-------------|----------|
| CI_USER / CI_PASS hardcoded | Use existing ci_runner creds directly | |
| Testing-Kit env substitution | No established pattern for this | |
| Separate low-priv test account | Better isolation — new user created in CI setup | ✓ |

**User's choice:** Separate test account (`tk_test` / `Tk_Test_Pass1!`)
**Notes:** Requires new PowerShell setup step in `test.yaml` (idempotent, mirroring the CI_USER creation step). Username/password follow the existing naming convention.

---

## Claude's Discretion

- Fixture cleanup: decided no explicit kill (beacon exit handles it) — user confirmed this approach.

## Deferred Ideas

- Cross-process impersonation verification (whoami BOF) — would require a new BOF outside this collection's scope.
- tk steal from SYSTEM process (winlogon/lsass) — requires SeDebugPrivilege; better as manual operator test, unreliable in CI.
