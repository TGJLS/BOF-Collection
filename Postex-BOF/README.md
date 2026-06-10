# Postex-BOF

Post-exploitation BOFs for credential harvesting and sensitive data collection.

## veeam-dumper

Extracts and decrypts credentials stored in a Veeam Backup & Replication (VBR) or VeeamOne database. Requires an **admin beacon** to read the relevant registry keys.

The BOF works in four stages: (1) detect the database type and instance from the registry, (2) locate the DB client executable (`sqlcmd.exe` or `psql.exe`), (3) read the encryption salt or entropy value from the registry under `HKLM\SOFTWARE\Veeam\...`, (4) spawn the DB client as a child process, capture its output, and decrypt each credential row. Two encryption schemes are handled: **A-type** (raw DPAPI blob — `CryptUnprotectData` without entropy, or with entropy for VeeamOne) and **V-type** (base64-encoded blob with a 37-byte header prefix, DPAPI-decrypted using the `EncryptionSalt` from the registry as entropy).

Output is split into two sections: a **Credentials** block listing decrypted username/password/description for each row, and a **Username → Host** block mapping each credential to the managed hosts it is assigned to.

```
veeam-dumper <auto|mssql|psql> [--dbname <name>] [--exepath <path>] [--debug] [--veeamone]
```

- `auto|mssql|psql` — Database type (required). `auto` inspects the registry to determine whether VBR uses MSSQL or PostgreSQL, or whether VeeamOne is present (always MSSQL). Pass `mssql` or `psql` to skip registry detection and force the type.
- `--dbname <name>` — Database name to query. If omitted, read from the registry (`SqlDatabaseName` under the appropriate VBR or VeeamOne key). Pass this if the registry read fails or you need to target a non-default database name.
- `--exepath <path>` — Full path to `sqlcmd.exe` (for MSSQL) or `psql.exe` (for PostgreSQL). If omitted, the BOF searches `%PATH%` first via `SearchPathA`, then falls back to a list of common install locations (SQL Server Client SDK paths for MSSQL; PostgreSQL 13–17 under `C:\Program Files\PostgreSQL\` for PSQL).
- `--debug` — Print verbose diagnostic output: detected database type, located executable path, DB name from registry, encryption salt value, raw SQL output, and each credential line as it is processed.
- `--veeamone` — Target a VeeamOne installation instead of VBR. Changes the registry paths used for the database name (`SOFTWARE\Veeam\Veeam ONE`), the entropy key (`SOFTWARE\Veeam\Veeam ONE\Private\Entropy`), the SQL instance (`SOFTWARE\Veeam\Veeam One\DatabaseServer`), and the credential/host queries to use the `[monitor]` schema. VeeamOne always uses MSSQL. Also set automatically by `auto` detection when the VeeamOne registry key is found.

**Examples:**
```
veeam-dumper auto
veeam-dumper mssql --debug --veeamone
veeam-dumper psql --dbname VeeamBackup --exepath "C:\Program Files\PostgreSQL\17\bin\psql.exe"
veeam-dumper mssql --dbname VeeamBackup --exepath "C:\Program Files\Microsoft SQL Server\150\Tools\Binn\sqlcmd.exe" --debug
```

**Requirements:**
- Admin beacon (registry keys for salt and DB config are under `HKLM\SOFTWARE\Veeam` and require elevated access).
- `sqlcmd.exe` or `psql.exe` must be present on the target (installed with SQL Server tools / PostgreSQL client respectively) or the path must be supplied via `--exepath`.
