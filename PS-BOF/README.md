# PS-BOF

Process management operations: ps_list, ps_kill, ps_run, ps_grep, ps_suspend, ps_resume.

## ps_list

List all running processes. Output columns: PID, PPID, session ID, owner (domain\user), architecture.

```
ps_list
```

## ps_kill

Terminate a process by PID. Optional exit code argument (defaults to 1 if omitted).

```
ps_kill <PID> [exit_code]
```

## ps_run

Launch a new process. Supports default CreateProcess, credential-based launch (WithLogon), and token-based launch (WithToken), with optional PPID spoofing and stdout/stderr pipe capture.

```
ps_run --command <cmd> [--pipe] [--ppid <PID>] [--state suspended] [--domain <domain> --username <user> --password <pass>] [--token <handle>]
```

- `--command <cmd>` — Command line to execute (required)
- `--pipe` — Capture stdout/stderr via anonymous pipe
- `--ppid <PID>` — Spoof parent PID
- `--state suspended` — Launch process in suspended state
- `--domain <domain>` — Domain for CreateProcessWithLogon
- `--username <user>` — Username for CreateProcessWithLogon
- `--password <pass>` — Password for CreateProcessWithLogon
- `--token <handle>` — Token handle for CreateProcessWithToken

## ps_grep

Inspect a process by PID. Output sections: token (owner, elevated flag, integrity level), modules (name, base address, entry point, size), command line, threads (TIDs).

```
ps_grep <PID>
```

## ps_suspend

Suspend a process by PID.

```
ps_suspend <PID>
```

## ps_resume

Resume a suspended process by PID.

```
ps_resume <PID>
```
