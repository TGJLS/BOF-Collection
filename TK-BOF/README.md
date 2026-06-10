# TK-BOF

Token management: steal, use, make, rm, revert, privget

|Commands|Usage|Notes|
|--------|-----|-----|
|steal|`tk_steal <pid>`|Duplicate a process token; optionally skip impersonation with `--no-apply`|
|use|`tk_use <token_handle>`|Impersonate a previously obtained token handle|
|make|`tk_make <username> <password>`|Create a token via LogonUserW; supports `--domain`, `--logon-type`, `--no-apply`|
|rm|`tk_rm <token_handle>`|Close a token handle and free the kernel object|
|revert|`tk_revert`|Drop impersonation and revert to process token|
|privget|`tk_privget`|Enable all privileges on the current token|

## Handle Lifecycle

tk_rm <handle> closes the kernel object — the handle is gone and cannot be reused. tk_revert drops impersonation but keeps handles alive — the token can be re-activated with tk_use. Always tk_rm handles you no longer need to avoid leaking kernel objects in the beacon process.

## steal

Duplicate a process token by PID via OpenProcessToken + DuplicateTokenEx. Impersonation is applied immediately via ImpersonateLoggedOnUser unless --no-apply is passed. The duplicated handle is printed for later reuse with `tk_use`.

```
tk_steal <pid>
tk_steal <pid> --no-apply
```

## use

Impersonate a previously obtained token handle via ImpersonateLoggedOnUser.

```
tk_use <token_handle>
```

## make

Create a token from plaintext credentials via LogonUserW. Impersonation is applied immediately unless --no-apply is passed. The token handle is printed for later reuse with `tk_use`.

```
tk_make <username> <password>
tk_make <username> <password> --domain <domain>
tk_make <username> <password> --logon-type <type>
tk_make <username> <password> --no-apply
```

## rm

Close a token handle and free the kernel object. The handle is gone after this call and cannot be reused with `tk_use`.

```
tk_rm <token_handle>
```

## revert

Drop impersonation and revert to the process token. Open token handles are not closed; they remain valid and can be reused with `tk_use`.

```
tk_revert
```

## privget

Enable all privileges on the current token by iterating the token's privilege set and calling AdjustTokenPrivileges.

```
tk_privget
```

## Credits

- [Kharon](https://github.com/entropy-z/Kharon): TK-BOF command implementations
