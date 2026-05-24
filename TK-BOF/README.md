# TK-BOF

Token management: steal, use, make, rm, revert, privget

## Handle Lifecycle

tk rm <handle> closes the kernel object — the handle is gone and cannot be reused. tk revert drops impersonation but keeps handles alive — the token can be re-activated with tk use. Always tk rm handles you no longer need to avoid leaking kernel objects in the beacon process.

## steal

Duplicate a process token by PID via OpenProcessToken + DuplicateTokenEx. Impersonation is applied immediately via ImpersonateLoggedOnUser unless --no-apply is passed. The duplicated handle is printed for later reuse with `tk use`.

```
tk steal <pid>
tk steal <pid> --no-apply
```

## use

Impersonate a previously obtained token handle via ImpersonateLoggedOnUser.

```
tk use <token_handle>
```

## make

Create a token from plaintext credentials via LogonUserW. Impersonation is applied immediately unless --no-apply is passed. The token handle is printed for later reuse with `tk use`.

```
tk make <username> <password>
tk make <username> <password> --domain <domain>
tk make <username> <password> --logon-type <type>
tk make <username> <password> --no-apply
```

## rm

Close a token handle and free the kernel object. The handle is gone after this call and cannot be reused with `tk use`.

```
tk rm <token_handle>
```

## revert

Drop impersonation and revert to the process token. Open token handles are not closed; they remain valid and can be reused with `tk use`.

```
tk revert
```

## privget

Enable all privileges on the current token by iterating the token's privilege set and calling AdjustTokenPrivileges.

```
tk privget
```
