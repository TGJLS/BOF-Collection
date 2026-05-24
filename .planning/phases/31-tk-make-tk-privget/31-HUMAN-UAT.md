---
status: partial
phase: 31-tk-make-tk-privget
source: [31-VERIFICATION.md]
started: 2026-05-24T10:30:00Z
updated: 2026-05-24T10:30:00Z
---

## Current Test

[awaiting human testing]

## Tests

### 1. tk make — live credential token creation
expected: Handle printed as `[+] Handle: 0x<hex>` and impersonation active (whoami reflects new user)
result: [pending]

### 2. tk privget without active impersonation
expected: Process token path taken silently, N > 0 privileges printed as `[+] Enabled N privileges.`
result: [pending]

### 3. tk privget with impersonation active (after tk make)
expected: Thread token path taken (not process token fallback), privileges enabled on impersonated identity
result: [pending]

## Summary

total: 3
passed: 0
issues: 0
pending: 3
skipped: 0
blocked: 0

## Gaps
