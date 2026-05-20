# Phase 25: ps grep - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-05-20
**Phase:** 25-ps-grep
**Areas discussed:** Arg interface, Thread enumeration, Module output fields

---

## Arg interface

| Option | Description | Selected |
|--------|-------------|----------|
| PID only — always dump all 4 sections | Simpler packing in Phase 26 ps.axs. PS-07 always wants all sections. No dead flag logic. | ✓ |
| Match Kharon flags — selective sections | PID + booleans for modules/tokens/threads/cmdline. More flexible but adds extra BeaconDataInt calls and dead code for unsupported Kharon flags. | |
| You decide | Claude picks the right approach. | |

**User's choice:** PID only — always dump all 4 sections
**Notes:** None — choice was clear given PS-07 requires all sections.

---

## Thread enumeration

| Option | Description | Selected |
|--------|-------------|----------|
| CreateToolhelp32Snapshot (Kharon pattern) | 3 new bofdefs.h entries. Direct snapshot of threads for the target PID. Kharon tested it. Includes thread priority. | ✓ |
| NtQuerySystemInformation reuse | Zero new declarations. Reuses list.c pattern. Slight overhead iterating all processes for one PID's threads. | |
| You decide | Claude picks the right approach. | |

**User's choice:** CreateToolhelp32Snapshot (Kharon pattern)
**Notes:** None — follows Kharon source directly.

---

## Module output fields

| Option | Description | Selected |
|--------|-------------|----------|
| PS-07 only — name, base addr, size | Matches requirement exactly. Cleaner output. EntryPoint rarely useful. | |
| Include EntryPoint too | Matches Kharon get_modules() output. One extra field per module. | ✓ |
| You decide | Claude picks. | |

**User's choice:** Include EntryPoint too
**Notes:** Matches Kharon output exactly for completeness.

---

## Claude's Discretion

- Section ordering in output: Token → Modules → Cmdline → Threads
- Per-section independent error handling (failure in one section doesn't abort the rest)
- goto cleanup at go() level for process_handle; each helper manages its own resources
- Integrity level string thresholds from Kharon exactly
- NtQueryInformationToken (already in bofdefs.h) used instead of GetTokenInformation

## Deferred Ideas

None — discussion stayed within phase scope.
