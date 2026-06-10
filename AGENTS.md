# Agents Guide: BOF-Collection

## Project Overview
BOFs for [AdaptixC2](https://github.com/Adaptix-Framework/AdaptixC2) — C source files compiled into Windows `.o` object files, loaded and executed reflectively by a beacon.

## Modules
| Directory | Commands | Description |
|-----------|----------|-------------|
| `FS-BOF` | `type mkdir copy move del rmdir pwd cd` | Filesystem operations |
| `PS-BOF` | `ps_list ps_kill ps_run ps_grep ps_suspend ps_resume` | Process management |
| `TK-BOF` | `tk_steal tk_use tk_make tk_rm tk_revert tk_privget` | Token manipulation |
| `Exit-BOF` | `exit process/thread` | Controlled beacon termination |
| `Postex-BOF` | `veeam-dumper` | Post-exploitation (Veeam credential dump) |

## Build System
- **Toolchain**: `mingw-w64` cross-compiler targeting Windows from Linux/macOS.
- **Local**: `make` from repo root. **Docker**: `docker compose build && docker compose run --rm bof-build`.
- **Output**: `<MODULE>/_bin/<name>.x64.o` and `<MODULE>/_bin/<name>.x32.o` (committed to repo).
- **Install**: Arch: `pacman -Syu mingw-w64-gcc` · Ubuntu/Kali: `apt install gcc-mingw-w64-x86-64-posix gcc-mingw-w64-i686 mingw-w64-tools`.

## File Layout (per module)
Each module: `<cmd>/<cmd>.c` (entry point `void go(char* args, int alen)`), `<cmd>/defs.h` (BOF-specific WinAPI declarations), `_bin/` (compiled objects, committed), `Makefile`, `<mod>.axs`.

## Code Conventions
- **API calls**: `LIB$Function` (e.g. `KERNEL32$HeapAlloc`); `#define` aliases in `defs.h` restore standard names.
- **Shared headers**: `_include/beacon.h` (Beacon API) and `_include/bofdefs.h` (common WinAPI BOF decls) via `-I ../_include`. New BOF-specific API calls go in the local `defs.h`, not `_include/bofdefs.h`.
- **Memory**: `malloc`/`realloc`/`free` via `MSVCRT$` macros; never use CRT globals.
- **Output**: `BeaconPrintf(CALLBACK_OUTPUT, ...)` or `BeaconFormatPrintf` + `BeaconFormatToString` for buffered multi-part output.
- **Strings**: Windows W-APIs receive wide strings packed as `wstr` in `.axs`; ASCII strings use local conversion helpers.

## Gotchas
- 64-bit builds sometimes need `HANDLE _trash = NULL;` in global scope to prevent relocation errors.
- `BeaconDataExtract` reads length-prefixed blobs; `BeaconDataInt` reads 4-byte LE ints — match the `ax.bof_pack` format string exactly.
- Each module's `Makefile` must be listed in root `Makefile` `SUBDIRS` or CI won't build it.
- Register command groups in the module `.axs` **and** load the `.axs` in `bof-collection.axs`.

## Testing & CI
- **Build CI** (`.github/workflows/build.yml`): matrix build on ubuntu, kali, arch, macos-14; passes if at least one `.o` produced.
- **Integration CI** (`.github/workflows/test.yaml`): AdaptixC2 server runs in Docker inside WSL2 on a Windows runner; a beacon is delivered to the Windows host via SSH; the task suite in `.github/ci/tasks.yaml` is run against the live session using [Testing-Kit](https://github.com/TheGr3atJosh/Testing-Kit) (`adaptix-testing -c config.yaml -t tasks.yaml`).
- **PowerShell preamble** (`ssh.preamble` in the CI config): PowerShell one-liners run on the Windows target after SSH connects but before the agent starts — used to create fixture files, disable Defender, and open firewall ports for the C2 callback.
- Credits for upstream BOF implementations belong in root `README.md § Credits`, not per-module READMEs.

## Porting New BOFs
See `docs/BOF-PORTING-GUIDE.md` for step-by-step instructions: source placement, header merging, Makefile, `.axs` authoring, README style, test case patterns, CI wiring, and monitoring runs until green.
