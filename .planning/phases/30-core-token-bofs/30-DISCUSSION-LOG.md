# Phase 30: Core Token BOFs - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-05-23
**Phase:** 30-core-token-bofs
**Areas discussed:** Handle output format, Error verbosity, rm/revert success output

---

## Handle output format

### steal success output

| Option | Description | Selected |
|--------|-------------|----------|
| Minimal hex | `[+] Handle: 0x1a4` — just the handle value in hex | ✓ |
| Contextual decimal | `[+] Token stolen from PID 1234 — Handle: 420` — decimal with PID context | |
| Match PS-BOF style | Follow PS-BOF output conventions for consistency | |

**User's choice:** Minimal hex

---

### steal --no-apply annotation

| Option | Description | Selected |
|--------|-------------|----------|
| Same output, no annotation | `[+] Handle: 0x1a4` identical whether --no-apply or not | |
| Annotate no-apply | `[+] Handle: 0x1a4 (impersonation not applied)` — explicit reminder | ✓ |

**User's choice:** Annotate no-apply

---

### use success output

| Option | Description | Selected |
|--------|-------------|----------|
| Minimal confirmation | `[+] Impersonating handle 0x1a4` — echoes the active handle | ✓ |
| Silent on success | No output — only errors printed | |
| You decide | Follow BOF convention | |

**User's choice:** Minimal confirmation

---

## Error verbosity

### Error format

| Option | Description | Selected |
|--------|-------------|----------|
| FormatMessage text | `[-] steal: OpenProcess failed: Access is denied.` — human-readable | ✓ |
| Error code only | `[-] steal: OpenProcess failed (error 5)` — bare GetLastError | |
| Both | `[-] steal: OpenProcess failed (5): Access is denied.` — code + text | |

**User's choice:** FormatMessage text

---

### Error helper placement

| Option | Description | Selected |
|--------|-------------|----------|
| Shared helper in bofdefs.h or tkerror.h | One FormatMessage wrapper reused across all BOFs | ✓ |
| Inline per BOF | ~5-line FormatMessage copy in each .c file | |

**User's choice:** Shared helper

---

## rm/revert success output

### rm output

| Option | Description | Selected |
|--------|-------------|----------|
| Confirmation | `[+] Handle 0x1a4 closed.` — echoes the freed handle | ✓ |
| Silent | No output on success | |

**User's choice:** Confirmation

---

### revert output

| Option | Description | Selected |
|--------|-------------|----------|
| Confirmation | `[+] Reverted to process token.` — clear acknowledgment | ✓ |
| Silent | No output on success | |

**User's choice:** Confirmation

---

## Claude's Discretion

- OpenProcess access mask: `PROCESS_QUERY_INFORMATION` (standard choice, not discussed explicitly)
- DuplicateTokenEx impersonation level: `SecurityImpersonation` (standard, not delegation)
- Handle leak cleanup on failure paths: steal must close intermediate handles; rm/revert have none

## Deferred Ideas

None — discussion stayed within phase scope.
