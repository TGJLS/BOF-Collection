# Phase 33: CI/CD Tests - Context

**Gathered:** 2026-05-25
**Status:** Ready for planning

<domain>
## Phase Boundary

Add `tasks.yaml` entries that exercise TK-BOF commands against a live Adaptix beacon in CI, and add a new CI setup step to `test.yaml` that creates the `tk_test` Windows user needed for `tk make` testing.

**TK-12 is already complete** — the `test.yaml` deploy block (mkdir, cp .o files, cp tk.axs, x64 object count verification) was added during Phase 32 (commit 22bb238). No further test.yaml deploy changes are needed.

In scope: `tasks.yaml` TK-BOF section (steal immediate, steal --no-apply + use + rm, revert, make, privget), new `test.yaml` PowerShell step to create `tk_test` user.
Not in scope: whoami BOF implementation, cross-process token verification, ps.axs or any other category's CI.

</domain>

<decisions>
## Implementation Decisions

### Steal source — fixture process
- **D-01:** Spawn a fresh long-lived process for TK-BOF tests using `ps run --command "ping -n 999 127.0.0.1"`. Capture PID via `Process started: PID (\d+)` into `{{tk_pid}}`. Use a distinct capture variable (not `{{pid}}`) so TK-BOF tests are independent of PS-BOF fixture state.
- **D-02:** The tk-fixture ping process is NOT explicitly killed at the end of the TK-BOF block. It dies when the beacon exits (CI container teardown). No cleanup task needed.

### Steal — test both impersonation paths
- **D-03:** Test immediate impersonation: `tk steal {{tk_pid}}` (no --no-apply). Verify via `expected_regex: "\\[\\+\\] Handle: 0x[0-9a-fA-F]+"`. Follow immediately with `tk revert` to clean up thread token. Verify `[+] Reverted to process token.`
- **D-04:** Test deferred impersonation: `tk steal {{tk_pid}} --no-apply`. Capture handle via `Handle: 0x([0-9a-fA-F]+)` into `{{tk_handle}}`. Then `tk use {{tk_handle}}` (verify `[+] Impersonating handle`). Then `tk rm {{tk_handle}}` (verify `[+] Handle ... closed`).
- **D-05:** No cross-process whoami verification. `ImpersonateLoggedOnUser` sets the thread token; `ps run` spawns a new process that inherits the process token, not the thread token — whoami via ps run would not reflect impersonation. The `[+] Handle:` output is the success signal.

### Impersonation verification
- **D-06:** Use `expected_regex` matching `\[\+\] Handle: 0x[0-9a-fA-F]+` for steal success. Use `not_expected: "error"` as secondary guard where appropriate.

### tk make — separate test account
- **D-07:** Add a new PowerShell setup step in `test.yaml` (before the "Run CI Container" step) to create a local Windows user: username `tk_test`, password `Tk_Test_Pass1!`. The step should be idempotent (check if user exists first, like the CI_USER creation step).
- **D-08:** `tk make --username tk_test --password Tk_Test_Pass1!` in tasks.yaml. No --domain (defaults to `.` in make.c). Verify `[+] Handle: 0x...` in output.
- **D-09:** After `tk make`, follow with `tk revert` to drop the impersonation.

### tk privget
- **D-10:** `tk privget` with no args. Accept either `[+] Enabled N privileges.` or `[!] not all privileges could be enabled. [+] Attempted N privileges (partial).` — both are valid in a constrained CI token. Use `not_expected: "error"` rather than a strict expected string, since partial success is legitimate.

### Task ordering in tasks.yaml
- **D-11:** TK-BOF block order: spawn fixture → steal immediate → revert → steal --no-apply + capture handle → use → rm → make → revert → privget. This minimizes impersonation state leak between tasks.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Existing CI test infrastructure (primary patterns)
- `.github/ci/tasks.yaml` — Full existing test file. PS-BOF section at bottom shows the established pattern: `cmdline`, `expected_regex`, `capture:`, `not_expected`. TK-BOF block appends after the PS-BOF block.
- `.github/workflows/test.yaml` — Full CI workflow. PowerShell user-creation step (Create CI user) is the template for the new `tk_test` user step. TK-BOF deploy block (lines 270–294) is already present — do NOT duplicate it.

### TK-BOF source (output strings to match)
- `TK-BOF/steal/steal.c` — Success: `[+] Handle: 0x%llx\n` (immediate) or `[+] Handle: 0x%llx (impersonation not applied)\n` (--no-apply)
- `TK-BOF/use/use.c` — Success: `[+] Impersonating handle 0x%llx\n`
- `TK-BOF/rm/rm.c` — Success: `[+] Handle 0x%llx closed.\n`
- `TK-BOF/revert/revert.c` — Success: `[+] Reverted to process token.\n`
- `TK-BOF/make/make.c` — Success: `[+] Handle: 0x%llx\n` (immediate) or `[+] Handle: 0x%llx (impersonation not applied)\n` (--no-apply)
- `TK-BOF/privget/privget.c` — Success: `[+] Enabled %lu privileges.\n` or `[!] privget: not all privileges could be enabled.` + `[+] Attempted %lu privileges (partial).\n`

### Requirements
- `.planning/REQUIREMENTS.md` — TK-11 (tasks.yaml entries), TK-12 (test.yaml deploy block — already done)

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- PS-BOF tasks.yaml block (lines 171–205 of `.github/ci/tasks.yaml`): exact pattern to follow — spawn fixture, capture PID, operate, kill. TK-BOF uses the same Testing-Kit features.
- `ps run --command "ping -n 999 127.0.0.1"` fixture pattern: already proven reliable on Windows Server SKUs in CI.

### Established Patterns
- **Testing-Kit capture:** `capture:` block under the spawning task with regex group → `{{var}}` in subsequent cmdlines.
- **Hex handle capture:** Same mechanism — `Handle: 0x([0-9a-fA-F]+)` → `{{tk_handle}}`. Testing-Kit capture is regex-group based; works for any pattern.
- **expected_regex vs expected:** Use `expected_regex` when the matched value contains dynamic content (PIDs, handles, counts). Use `expected` for exact static strings.
- **CI user creation idempotency:** The existing "Create CI user" step checks `Get-LocalUser ... -ErrorAction SilentlyContinue` before creating. Copy this pattern for `tk_test`.

### Integration Points
- `tasks.yaml`: append TK-BOF block after the last PS-BOF task (`# ps-kill`).
- `test.yaml`: insert "Create tk_test user" PowerShell step after "Create CI user" step (or anywhere before the "Run CI Container" step).

</code_context>

<specifics>
## Specific Ideas

- tk_test user credentials: `tk_test` / `Tk_Test_Pass1!` — mirrors CI_USER naming convention, passes Windows password complexity.
- Fixture process uses `{{tk_pid}}` (not `{{pid}}`) to avoid dependency on PS-BOF fixture state.
- TK-BOF test block terminates without an explicit kill — beacon exit cleans up.
- Partial privget success (`[!] not all privileges could be enabled.`) is an acceptable CI outcome — use `not_expected: "error"` rather than a strict count match.

</specifics>

<deferred>
## Deferred Ideas

- Cross-process impersonation verification (whoami BOF) — would require a new BOF that calls GetUserNameW or TokenUser on the current thread token. Not in scope for this collection.
- tk steal from a SYSTEM process (winlogon/lsass) — requires SeDebugPrivilege; unreliable in CI. Better as a manual operator test.

</deferred>

---

*Phase: 33-ci-cd-tests*
*Context gathered: 2026-05-25*
