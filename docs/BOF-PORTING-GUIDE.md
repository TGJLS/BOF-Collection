# BOF Porting Guide

Instructions for an LLM porting a BOF into BOF-Collection for AdaptixC2.

---

## Prerequisites

- Source BOF: a `.c` file with `void go(char* args, int alen)` entry point.
- Original loader: a CobaltStrike `.cna` script or similar, showing argument packing format and command syntax.
- Toolchain available: `x86_64-w64-mingw32-gcc`, `i686-w64-mingw32-gcc`.

---

## Step 1 — Choose or Create a Category

Each category is a module directory (`<NAME>-BOF/`) with a unified theme.

**If the BOF fits an existing category** (FS, PS, TK, Exit, Postex), place it inside that directory as a new subdirectory and update that module's `Makefile` and `.axs` file. Skip to Step 3.

**If you need a new category:**

1. Create `<NAME>-BOF/` with a `_bin/` subdirectory and one subdirectory per command.
2. Copy the Makefile pattern below into `<NAME>-BOF/Makefile`:
   ```makefile
   CC64    = x86_64-w64-mingw32-gcc
   CC86    = i686-w64-mingw32-gcc
   STRIP64 = x86_64-w64-mingw32-strip --strip-unneeded
   STRIP86 = i686-w64-mingw32-strip --strip-unneeded
   CFLAGS  = -I ../_include -I _include -w -Wno-incompatible-pointer-types -Os -DBOF -c

   all: bof

   bof: clean
   	@(mkdir _bin 2>/dev/null) && echo 'creating _bin directory' || echo '_bin directory exists'
   	@($(CC64) $(CFLAGS) <cmd>/<cmd>.c -o _bin/<cmd>.x64.o && $(STRIP64) _bin/<cmd>.x64.o) && echo '[+] <cmd> x64' || echo '[!] <cmd> x64'
   	@($(CC86) $(CFLAGS) <cmd>/<cmd>.c -o _bin/<cmd>.x32.o && $(STRIP86) _bin/<cmd>.x32.o) && echo '[+] <cmd> x32' || echo '[!] <cmd> x32'

   clean:
   	@(rm -rf _bin)
   ```
3. Add the new directory to root `Makefile` `SUBDIRS`:
   ```makefile
   SUBDIRS := FS-BOF Exit-BOF TK-BOF PS-BOF Postex-BOF <NAME>-BOF
   ```
4. Add a load line to `bof-collection.axs`:
   ```js
   ax.script_load(path + "<NAME>-BOF/<name>.axs");
   ```

---

## Step 2 — Place Source Files and Handle Headers

The repo ships two shared headers in `_include/` at the repo root:

| File | Purpose |
|------|---------|
| `_include/beacon.h` | Beacon API declarations (`BeaconPrintf`, `BeaconDataExtract`, `formatp`, etc.) |
| `_include/bofdefs.h` | Common WinAPI BOF declarations (`KERNEL32$`, `MSVCRT$`, etc.) and macros |

Place the BOF source inside the module:
```
<NAME>-BOF/
  <cmd>/
    <cmd>.c       # BOF source
    defs.h        # BOF-specific WinAPI declarations (see below)
```

### beacon.h

The upstream BOF will ship its own `beacon.h`. **Do not copy it into the module directory.** Use `-I ../_include` in the Makefile so it resolves from `_include/beacon.h`. Compare the upstream copy with `_include/beacon.h` to confirm they are equivalent — they almost always are since both derive from the Cobalt Strike beacon SDK. If the upstream copy adds declarations not present in `_include/beacon.h`, add only those additions to `_include/beacon.h`.

### bofdefs.h / defs.h

The upstream BOF will ship its own `defs.h` (or similarly named file) containing `LIBRARY$Function` declarations and `#define` aliases for the WinAPI calls it uses. **Do not copy this file verbatim into `_include/bofdefs.h`.** Instead:

1. Keep the upstream `defs.h` at `<NAME>-BOF/<cmd>/defs.h` (or `<NAME>-BOF/_include/defs.h` if shared across multiple commands in the module).
2. Check each declaration in the upstream `defs.h` against `_include/bofdefs.h`:
   - If a declaration already exists in `_include/bofdefs.h`, remove it from the local `defs.h` (avoid duplication).
   - If a declaration is new (a WinAPI call not yet in `_include/bofdefs.h`), keep it in the local `defs.h`. Optionally promote it to `_include/bofdefs.h` if it is likely useful to other BOFs.
3. The Makefile uses `-I ../_include -I _include`, so both the shared and local headers are available.

---

## Step 3 — Audit the Source

Before adapting, verify:

1. **Entry point**: must be `void go(char* args, int alen)`.
2. **API declarations**: every WinAPI call must appear as `LIBRARY$Function` in either `_include/bofdefs.h` or the local `defs.h`. Check all calls used in the source.
3. **Memory**: `malloc`/`free`/`realloc` must go through `MSVCRT$` aliases defined in `defs.h`.
4. **Argument parsing**: note the original CNA `bof_pack` format string — each character maps to a BOF argument type:
   - `z` / `str` → `BeaconDataExtract` (byte string; ASCII or UTF-8)
   - `Z` / `wstr` → `BeaconDataExtract` (UTF-16LE wide string; use for paths passed to Windows W-APIs)
   - `i` / `int` → `BeaconDataInt` (4-byte LE integer)
5. **No CRT globals**: globals like `errno`, `_environ` cause relocation errors. Use local state or MSVCRT wrappers.
6. **64-bit relocation**: if the build produces relocation errors on x64, add `HANDLE _trash = NULL;` in the global scope of the `.c` file.

---

## Step 4 — Write the Module Makefile

One compile line per command. Add a line for each `.c` file:
```makefile
@($(CC64) $(CFLAGS) <cmd>/<cmd>.c -o _bin/<cmd>.x64.o && $(STRIP64) _bin/<cmd>.x64.o) && echo '[+] <cmd> x64' || echo '[!] <cmd> x64'
@($(CC86) $(CFLAGS) <cmd>/<cmd>.c -o _bin/<cmd>.x32.o && $(STRIP86) _bin/<cmd>.x32.o) && echo '[+] <cmd> x32' || echo '[!] <cmd> x32'
```

---

## Step 5 — Write the `.axs` Script

Model: `TK-BOF/tk.axs`, `PS-BOF/ps.axs`, `FS-BOF/fs.axs`.

```js
var metadata = {
    name: "<NAME>-BOF",
    description: "<short description of all commands>",
};

var cmd_<cmd> = ax.create_command("<cmd-name>", "<one-line description>", "<cmd-name> <usage>");
// Positional required arg:
cmd_<cmd>.addArgString("<argname>", true);
// Optional flag with string value:
cmd_<cmd>.addArgFlagString("--flag", "key", false, "<description>");
// Optional bool flag:
cmd_<cmd>.addArgBool("--flag", "<description>", false);
// Optional flag with int value:
cmd_<cmd>.addArgFlagInt("--flag", "key", "<description>", 0);

cmd_<cmd>.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let arg1 = parsed_json["<argname>"] || "";
    let flag1 = parsed_json["--flag"] ? 1 : 0;
    let bof_params = ax.bof_pack("wstr,int", [arg1, flag1]);
    let bof_path = ax.script_dir() + "_bin/<cmd>." + ax.arch(id) + ".o";
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}" ${bof_params}`, "BOF: <cmd>");
});

// For grouped sub-commands (like `ps list`, `tk steal`):
var cmd_parent = ax.create_command("<parent>", "<description>");
cmd_parent.addSubCommands([cmd_<cmd>]);
var group = ax.create_commands_group("<NAME>-BOF", [cmd_parent]);

// For flat top-level commands:
// var group = ax.create_commands_group("<NAME>-BOF", [cmd_<cmd>]);

ax.register_commands_group(group, ["beacon", "gopher", "kharon"], ["windows"], []);
```

**bof_pack type reference:**

| axs type | CS equiv | BOF read function | Use for |
|----------|----------|-------------------|---------|
| `wstr`   | `Z`      | `BeaconDataExtract` | strings — wide/UTF-16 for W-API paths |
| `int`    | `i`      | `BeaconDataInt`    | 4-byte integers and booleans |

> **Note**: If the original CNA used `z` (ASCII) and the BOF calls `strcmp` on the result, verify at runtime. If comparisons fail, the C code needs to handle wide strings via `WideCharToMultiByte` conversion.

---

## Step 6 — Local Build Test

```bash
cd <NAME>-BOF
make
ls _bin/        # must contain .x64.o and .x32.o for every command
```

Then from repo root:
```bash
make
find . -name '*.o' | wc -l   # must be > 0
```

Commit the compiled `.o` files in `_bin/` — the CI expects them to be buildable and the repo ships pre-built objects.

---

## Step 7 — Write the README

Every module directory requires a `README.md`. Follow the style of existing READMEs (`PS-BOF/README.md`, `TK-BOF/README.md`):

- One `##` section per command.
- Each section opens with a **descriptive paragraph** — explain what the command does internally (which APIs it calls, what data it reads, how output is structured), not just what the flag names are.
- A code block showing the full usage line with all optional flags.
- A bullet list documenting **every flag** with a description of its effect and default behavior.
- Include behavioral edge cases (what happens when a required resource is missing, what output sections are produced, etc.).

Example structure:
```markdown
# <NAME>-BOF

<one-line summary>

## <cmd-name>

<descriptive paragraph: what this does internally, what output looks like, any preconditions>

\```
<cmd-name> <required-arg> [--flag1 <val>] [--flag2]
\```

- `<required-arg>` — <description>
- `--flag1 <val>` — <description and default>
- `--flag2` — <description>
```

**Do not add a Credits section to the per-category README.** Credits for upstream projects belong in the root `README.md` under the existing `## Credits` section. Add an entry there for any upstream BOF implementation the port is based on.

Also update the root `README.md` Modules section with a new row in the appropriate table (or a new `##` section + table if it is a new category), and add a `[More details](<NAME>-BOF/README.md)` link.

---

## Step 8 — Write Test Cases

Tests use [Testing-Kit](https://github.com/TheGr3atJosh/Testing-Kit): `adaptix-testing -c config.yaml -t tasks.yaml`.

### PowerShell preamble

The `ssh.preamble` block in the CI config runs PowerShell commands on the Windows target via SSH **after connecting but before uploading and starting the agent**. Use it to create test fixtures, disable Defender, configure firewall rules, and set up any preconditions the BOF needs. Each entry is a PowerShell one-liner executed via `-EncodedCommand`; a non-zero exit code aborts the run.

```yaml
ssh:
  host: <windows-target>
  username: <user>
  source_path: /tmp/ci_agent.exe
  agent_path: 'C:\ci\agent.exe'
  terminate: true
  preamble:
    # Baseline: always include these
    - 'New-Item -ItemType Directory -Force -Path C:\ci | Out-Null'
    - 'Set-MpPreference -DisableRealtimeMonitoring $true'
    - 'Add-MpPreference -ExclusionPath C:\ci'
    - 'New-NetFirewallRule -DisplayName CI_C2_8080 -Direction Inbound -Protocol TCP -LocalPort 8080 -Action Allow -Profile Any | Out-Null'
    # BOF-specific fixture setup:
    - 'New-Item -ItemType Directory -Force -Path C:\Temp\bof-test | Out-Null'
    - 'Set-Content -Path C:\Temp\bof-test\fixture.txt -Value "test content" -Encoding UTF8 -NoNewline'
```

> For BOFs that require specific software (e.g. Veeam, a database instance), the preamble cannot substitute for an actual installation. Use a dedicated lab environment and document the prerequisites clearly in the module `README.md`.

### tasks.yaml format

```yaml
tasks:
  # Basic — command must complete without error
  - cmdline: "<cmd-name> arg"

  # Assert output contains substring (case-insensitive)
  - cmdline: "<cmd-name> arg"
    expected: "expected output"

  # Assert output matches regex
  - cmdline: "<cmd-name> arg"
    expected_regex: "\\[\\+\\] Result: \\d+"

  # Assert substring is absent
  - cmdline: "<cmd-name> arg"
    not_expected: "error"

  # Capture a value from output for use in later tasks
  - cmdline: "<cmd-name> arg"
    expected_regex: "Handle: 0x[0-9a-fA-F]+"
    capture:
      my_handle: "Handle: (0x[0-9a-fA-F]+)"

  # Use captured value in a subsequent task
  - cmdline: "<cmd-name> --handle {{my_handle}}"
    expected: "[+] Success"

  # Mark as allowed_to_fail for cleanup or expected-error cases
  - cmdline: "<cmd-name> nonexistent-arg"
    allowed_to_fail: true
```

Write at least these test cases per command:
- **Happy path**: normal invocation with valid args; assert key output substring.
- **Missing required resource**: e.g. file not found, registry key absent; assert error message.
- **Invalid argument**: wrong type or out-of-range value; assert rejection.
- **Cleanup**: if the command creates state (processes, files, handles), add a teardown task at the end, marked `allowed_to_fail: true` if the resource may not exist.

### Add the new tasks to CI

Add your tasks to `.github/ci/tasks.yaml`. Group them under a comment header. Update the build verification step in `.github/workflows/test.yaml` to copy your module's `_bin/` into the container and assert the expected object count:

```bash
mkdir -p /tmp/adaptixc2/dist/BOF-Collection/<NAME>-BOF/_bin
cp /workspace/<NAME>-BOF/_bin/*.o /tmp/adaptixc2/dist/BOF-Collection/<NAME>-BOF/_bin/
cp /workspace/<NAME>-BOF/<name>.axs /tmp/adaptixc2/dist/BOF-Collection/<NAME>-BOF/
count=$(ls /workspace/<NAME>-BOF/_bin/*.x64.o | wc -l)
[ "$count" -eq <N> ] && echo "✓ All <N> <NAME>-BOF x64 objects compiled" || \
  { echo "✗ Expected <N> <NAME>-BOF x64 objects, got $count"; exit 1; }
```

---

## Step 9 — Push and Monitor CI

```bash
git checkout -b port/<cmd-name>
git add <NAME>-BOF/ bof-collection.axs Makefile README.md AGENTS.md
git commit -m "port: add <cmd-name> BOF to <NAME>-BOF"
git push -u origin port/<cmd-name>
```

**Monitor GitHub Actions:**
```bash
gh run list --branch port/<cmd-name>
gh run watch <run-id>
gh run view <run-id> --log-failed
```

**Iterate until green:**
1. `gh run list --branch port/<cmd-name>` — find the latest run ID.
2. `gh run watch <run-id>` — stream progress; all matrix jobs must show ✓ (ubuntu, kali, arch, macos-14).
3. On failure: `gh run view <run-id> --log-failed` → read compiler errors → fix source or `defs.h` → commit and push.
4. Repeat until all jobs pass.

---

## Checklist

- [ ] Source `.c` compiles locally with `make` (no fatal errors)
- [ ] `_bin/*.x64.o` and `_bin/*.x32.o` produced for every command
- [ ] Upstream `beacon.h` compared with `_include/beacon.h`; not copied into module
- [ ] Upstream `defs.h` deduplicated against `_include/bofdefs.h`; new API declarations kept in local `defs.h`
- [ ] Module directory added to root `Makefile` `SUBDIRS`
- [ ] `.axs` file created with correct `bof_pack` format string
- [ ] `.axs` loaded in `bof-collection.axs`
- [ ] `AGENTS.md` updated with new module entry
- [ ] Module `README.md` written: descriptive paragraphs + all flags documented per command
- [ ] Root `README.md` Modules section updated with new entry row + link; credit added to `## Credits`
- [ ] Test tasks added to `.github/ci/tasks.yaml` with PowerShell preamble for any required fixtures
- [ ] CI build verification counts updated in `test.yaml`
- [ ] CI passes on all matrix targets
