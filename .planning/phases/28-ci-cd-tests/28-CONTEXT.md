# Phase 28: CI/CD Tests - Context

**Gathered:** 2026-05-21
**Status:** Ready for planning

<domain>
## Phase Boundary

Two parallel deliverables:
1. **Testing-Kit** (`~/github/Testing-Kit`): Add `capture` field + `{{var}}` variable substitution to the task runner, update README. This enables PID-chained test sequences.
2. **BOF-Collection** (this repo): Add tasks.yaml entries for all 6 PS-BOF commands (CI-01 through CI-06) and update `.github/workflows/test.yaml` to deploy PS-BOF into the CI container (CI-07).

Note: The Docker CI approach is acknowledged as needing a future rework. Phase 28 targets "make it work" — cleanup deferred.

</domain>

<decisions>
## Implementation Decisions

### Testing-Kit: capture/substitution feature

- **D-01:** Add `capture: {var_name: regex}` field to the task schema. The value is a dict of `{variable_name: regex_with_one_capture_group}`. Multiple captures per task are supported. Captured values are stored in a `variables` dict that persists for the duration of the task run.
- **D-02:** Variable substitution: before dispatching each task, replace `{{var_name}}` in `cmdline` with the corresponding value from `variables`. If a variable is referenced but not yet captured, the placeholder passes through unchanged (no error).
- **D-03:** Update `~/github/Testing-Kit/README.md` — add `capture` to the tasks.yaml field table and add a worked example showing capture → substitution.

### Testing-Kit: CI reinstall

- **D-04:** The CI container pre-dates the capture feature. The `.github/workflows/test.yaml` Docker bash block must reinstall adaptix-testing from git before running tests (e.g., `uv tool install --reinstall git+https://github.com/TheGr3atJosh/Testing-Kit`). This is the "just make it work" approach — no container rebuild required.

### Test sequencing

- **D-05:** Spawn `notepad.exe` once via `ps run`, capture its PID. Run dependent tests in this order: `ps grep {{pid}}` → `ps suspend {{pid}}` → `ps resume {{pid}}` → `ps kill {{pid}}`. This single process is used for CI-02, CI-04, CI-05, CI-06.
- **D-06:** `ps run --pipe` test is separate from the notepad sequence: `ps run --command "cmd.exe /c whoami" --pipe`. Expected output: `expected_regex: "(?i)ci_runner"` (the CI user is `ci_runner`). This covers CI-03.
- **D-07:** `ps list` test (CI-01) is independent: `expected` contains `"System"` and `"lsass.exe"` (two separate entries or one combined check).

### CI workflow deployment (test.yaml)

- **D-08:** PS-BOF deployment in the Docker bash block: `mkdir -p` the container's PS-BOF/_bin dir, copy `_bin/*.o`, copy `ps.axs`, and copy the workspace `bof-collection.axs` (container's version pre-dates Phase 26's `ps.axs` load). Container path follows FS-BOF pattern: `/tmp/adaptixc2/dist/BOF-Collection/PS-BOF/`.
- **D-09:** Build verification: add a PS-BOF x64 object count check (6 BOFs: list, kill, run, grep, suspend, resume → 6 x64 .o files).

### Plan structure

- **D-10:** Two plans in Wave 1 (parallel — different repos):
  - 28-01: Testing-Kit — capture feature (`run.py`) + README
  - 28-02: BOF-Collection CI — `tasks.yaml` entries + `test.yaml` update (depends on Testing-Kit having the capture feature available)

### Claude's Discretion

- Tasks.yaml comment header for PS-BOF section: follow FS-BOF pattern (`# ── PS-BOF ──`)
- ps run task that spawns notepad.exe: use `expected_regex: "Process started: PID \\d+"` as the assertion (ps run outputs `"Process started: PID %lu, TID %lu\n"`)
- ps suspend/resume/kill tasks: no `expected` needed (success = command completes without CALLBACK_ERROR output); use `not_expected: "error"` if that's cleaner
- ps grep output assertion: use `expected` for each of the four section labels (token, modules, cmdline, threads) — or `expected_regex` for a combined pattern; planner can choose
- adaptix-testing uv reinstall: add it immediately before the "Integration tests" echo line, after the build verification block

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Testing-Kit source (primary modification target)

- `~/github/Testing-Kit/run.py` — the task runner; add `capture` parsing after `poll_for_result`, add variable dict init before the task loop, add `{{var}}` substitution before `dispatch`. The task loop is in `main()` starting at line 620.
- `~/github/Testing-Kit/README.md` — add `capture` to the field table and add a worked example section.

### Existing CI infrastructure

- `.github/workflows/test.yaml` — the integration test workflow. Docker bash block starts at line 257. "BOF rebuild" copies happen at lines 264–268. Build verification at lines 270–277. "Integration tests" at line 303. Add PS-BOF copy steps and uv reinstall here.
- `.github/ci/tasks.yaml` — existing test suite (38 FS-BOF entries). Add PS-BOF entries after the existing entries.
- `.github/ci/config.yaml` — CI config; SSH preamble is at the `ssh.preamble` list. No changes needed for Phase 28.

### PS-BOF command output format (what to assert against)

- `PS-BOF/run/run.c` line 287–288 — outputs `"Process started: PID %lu, TID %lu\n"` on success
- `PS-BOF/ps.axs` — authoritative command signatures and flag names for all 6 PS-BOF commands

### Prior phase decisions (scope boundaries)

- `.planning/phases/26-ps-axs-process-browser/26-CONTEXT.md` — D-03 through D-07: exact arg interfaces (what each command packs)
- `.planning/phases/25-ps-grep/25-CONTEXT.md` — D-01: ps grep outputs four sections (Token, Modules, Cmdline, Threads)

### Requirements

- `.planning/REQUIREMENTS.md` — CI-01 through CI-07 (each requirement maps to one task entry or one workflow update)

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets

- `run.py` main task loop (lines 616–678): `variables = {}` init before the loop; after `poll_for_result` succeeds, check `task.get("capture")` and extract regex groups into `variables`; before `dispatch`, do `cmdline = cmdline` with `str.replace` substitution for each `{{key}}` in `variables`. ~20–25 lines total.
- Existing `check_output()` function (line 447) is unchanged — capture is orthogonal to assertions.

### Established Patterns

- tasks.yaml task format: `cmdline` (required), `expected`, `expected_regex`, `not_expected`, `not_expected_regex`, `allowed_to_fail` (all optional). `capture` is additive.
- FS-BOF copy pattern in test.yaml: `cp /workspace/FS-BOF/_bin/*.o /tmp/adaptixc2/dist/BOF-Collection/FS-BOF/_bin/` — mirror for PS-BOF.
- Build verification pattern: `count=$(ls /workspace/PS-BOF/_bin/*.x64.o | wc -l)` then assert count equals 6.

### Integration Points

- `bof-collection.axs` workspace version already has `ax.script_load(path + "PS-BOF/ps.axs")` (Phase 26 D-09) — must be copied to container since container image pre-dates this.
- Container path structure: `/tmp/adaptixc2/dist/BOF-Collection/` — the `PS-BOF/` subdir may not exist in the pre-built image; always `mkdir -p`.

</code_context>

<specifics>
## Specific Ideas

- The `capture` field name and `{{var}}` placeholder syntax were explicitly chosen (dict supports multiple captures per task).
- The notepad.exe spawn sequence matches the ROADMAP success criteria order for CI-02/CI-04/CI-05/CI-06.
- The "just make it work" note means: don't refactor the Docker setup — minimum viable changes to unblock PS-BOF CI coverage.
- The CI user in GitHub Actions is `ci_runner` — `ps run --pipe whoami` expected output is `(?i)ci_runner`.

</specifics>

<deferred>
## Deferred Ideas

- Docker CI approach rework — user noted the current approach needs a full rework eventually; deferred to a future phase.

</deferred>

---

*Phase: 28-CI/CD Tests*
*Context gathered: 2026-05-21*
