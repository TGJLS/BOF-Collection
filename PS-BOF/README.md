# PS-BOF

Process management operations: ps list, ps kill, ps run, ps grep, ps suspend, ps resume.

## ps list

List all running processes. Output columns: PID, PPID, session ID, owner (domain\user), architecture.

```
ps list
```

## ps kill

Terminate a process by PID. Optional exit code argument (defaults to 1 if omitted).

```
ps kill <PID> [exit_code]
```

## ps run

Launch a new process. Supports default CreateProcess, credential-based launch (WithLogon), and token-based launch (WithToken), with optional PPID spoofing and stdout/stderr pipe capture.

```
ps run --command <cmd> [--pipe] [--ppid <PID>] [--state suspended] [--domain <domain> --username <user> --password <pass>] [--token <handle>]
```

- `--command <cmd>` — Command line to execute (required)
- `--pipe` — Capture stdout/stderr via anonymous pipe
- `--ppid <PID>` — Spoof parent PID
- `--state suspended` — Launch process in suspended state
- `--domain <domain>` — Domain for CreateProcessWithLogon
- `--username <user>` — Username for CreateProcessWithLogon
- `--password <pass>` — Password for CreateProcessWithLogon
- `--token <handle>` — Token handle for CreateProcessWithToken

## ps grep

Inspect a process by PID. Output sections: token (owner, elevated flag, integrity level), modules (name, base address, entry point, size), command line, threads (TIDs).

```
ps grep <PID>
```

## ps suspend

Suspend a process by PID.

```
ps suspend <PID>
```

## ps resume

Resume a suspended process by PID.

```
ps resume <PID>
```
