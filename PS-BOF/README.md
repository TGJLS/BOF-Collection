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
ps run --command "cmd.exe /c whoami" --pipe
```

```
ps run --command "cmd.exe" --domain CORP --username admin --password Secret
```

```
ps run --command "cmd.exe" --token <handle>
```

Note: `--state suspended` launches suspended; `--ppid <PID>` spoofs parent PID.

## ps grep

Inspect a process by PID. Output sections: token (owner, elevation type, integrity level), modules (name, base address, entry point, size), command line, threads (TIDs).

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
