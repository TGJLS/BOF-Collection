# PS-BOF

Process management operations: process list, process kill, process run, process grep, process suspend, process resume.

## process list

List all running processes. Output columns: PID, PPID, session ID, owner (domain\user), architecture.

```
process list
```

## process kill

Terminate a process by PID. Optional exit code argument (defaults to 1 if omitted).

```
process kill <PID> [exit_code]
```

## process run

Launch a new process. Supports default CreateProcess, credential-based launch (WithLogon), and token-based launch (WithToken), with optional PPID spoofing and stdout/stderr pipe capture.

```
process run --command <cmd> [--pipe] [--ppid <PID>] [--state suspended] [--domain <domain> --username <user> --password <pass>] [--token <handle>]
```

- `--command <cmd>` — Command line to execute (required)
- `--pipe` — Capture stdout/stderr via anonymous pipe
- `--ppid <PID>` — Spoof parent PID
- `--state suspended` — Launch process in suspended state
- `--domain <domain>` — Domain for CreateProcessWithLogon
- `--username <user>` — Username for CreateProcessWithLogon
- `--password <pass>` — Password for CreateProcessWithLogon
- `--token <handle>` — Token handle for CreateProcessWithToken

## process grep

Inspect a process by PID. Output sections: token (owner, elevated flag, integrity level), modules (name, base address, entry point, size), command line, threads (TIDs).

```
process grep <PID>
```

## process suspend

Suspend a process by PID.

```
process suspend <PID>
```

## process resume

Resume a suspended process by PID.

```
process resume <PID>
```
