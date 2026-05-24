# Phase 31: tk make + tk privget - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-05-24
**Phase:** 31-tk-make-tk-privget
**Areas discussed:** LogonUser type (make), privget token source, privget success output

---

## LogonUser type (make)

| Option | Description | Selected |
|--------|-------------|----------|
| NEW_CREDENTIALS (9) | No new logon session on local machine; 4648 only; OPSEC-friendly for lateral movement | |
| INTERACTIVE (2) | Full interactive logon session, 4624+4648, full user token locally | |
| NETWORK_CLEARTEXT (8) | Network logon with cleartext credentials | |

**User's choice:** Add an optional `logon_type` parameter — default to 9 (NEW_CREDENTIALS).

---

**Q: Zero-sentinel or axs-side default?**

| Option | Description | Selected |
|--------|-------------|----------|
| if (logon_type == 0) logon_type = 9 | Zero-sentinel in stub; axs sends 0 when not specified | ✓ |
| Document as required in axs | axs always sends a value; stub trusts it | |

**User's choice:** Zero-sentinel in C stub (`if (logon_type == 0) logon_type = 9`).

---

**Q: Empty domain handling?**

| Option | Description | Selected |
|--------|-------------|----------|
| Pass "." when domain is empty | Standard local machine sentinel | ✓ |
| Pass NULL when domain is empty | System determines domain | |

**User's choice:** Pass `"."` when domain is empty.

---

## privget token source

| Option | Description | Selected |
|--------|-------------|----------|
| Thread token → fallback process token | OpenThreadToken first; fallback OpenProcessToken | ✓ |
| Process primary token only | OpenProcessToken always; simpler but wrong when impersonating | |

**User's choice:** Thread token → fallback process token.

---

**Q: Token access flags?**

| Option | Description | Selected |
|--------|-------------|----------|
| TOKEN_QUERY \| TOKEN_ADJUST_PRIVILEGES | Minimum required access | ✓ |
| TOKEN_ALL_ACCESS | Broad; inconsistent with least-privilege pattern | |

**User's choice:** `TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES`.

---

## privget success output

| Option | Description | Selected |
|--------|-------------|----------|
| [+] Enabled N privileges. | Count from PrivilegeCount | ✓ |
| [+] All privileges enabled. | Fixed string, no count | |
| List each privilege name | LookupPrivilegeName + loop — much more code | |

**User's choice:** `[+] Enabled %lu privileges.` with count from `PrivilegeCount`.

---

**Q: ERROR_NOT_ALL_ASSIGNED handling?**

| Option | Description | Selected |
|--------|-------------|----------|
| Warn, not error | [!] warning then still print count | ✓ |
| Treat as success silently | Ignore the status | |
| Treat as error, abort | Return on ERROR_NOT_ALL_ASSIGNED | |

**User's choice:** Warn with `[!] privget: not all privileges could be enabled.`, then continue to print count.

---

## Claude's Discretion

None — all decisions made by user.

## Deferred Ideas

None.
