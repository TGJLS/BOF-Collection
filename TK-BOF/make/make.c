#include <windows.h>
#include "beacon.h"
#include "bofdefs.h"
#include "../tkerror.h"

VOID go(IN PCHAR Buffer, IN ULONG Length)
{
    datap parser;
    WCHAR *username = NULL;
    WCHAR *password = NULL;
    WCHAR *domain   = NULL;
    BOOL  no_apply = FALSE;
    int   logon_type = 0;
    HANDLE hToken  = NULL;
    DWORD  dwError = 0;
    char   errMsg[256];

    BeaconDataParse(&parser, Buffer, Length);
    username   = (WCHAR*) BeaconDataExtract(&parser, NULL);
    password   = (WCHAR*) BeaconDataExtract(&parser, NULL);
    domain     = (WCHAR*) BeaconDataExtract(&parser, NULL);
    no_apply   = (BOOL) BeaconDataInt(&parser);
    logon_type = (int)  BeaconDataInt(&parser);

    if (!username || !password)
    {
        BeaconPrintf(CALLBACK_ERROR, "[-] make: missing username or password argument\n");
        return;
    }

    if (logon_type == 0) logon_type = 9;
    const WCHAR *dom = (domain && domain[0]) ? domain : L".";

    if (!ADVAPI32$LogonUserW(username, dom, password, (DWORD)logon_type, LOGON32_PROVIDER_DEFAULT, &hToken))
    {
        dwError = KERNEL32$GetLastError();
        TkErrorMessage(dwError, errMsg, sizeof(errMsg));
        BeaconPrintf(CALLBACK_ERROR, "[-] make: LogonUserW failed: %s\n", errMsg);
        return;
    }

    if (!no_apply)
    {
        if (!ADVAPI32$ImpersonateLoggedOnUser(hToken))
        {
            dwError = KERNEL32$GetLastError();
            TkErrorMessage(dwError, errMsg, sizeof(errMsg));
            BeaconPrintf(CALLBACK_ERROR, "[-] make: ImpersonateLoggedOnUser failed: %s\n", errMsg);
            NTDLL$NtClose(hToken);
            return;
        }
        BeaconPrintf(CALLBACK_OUTPUT, "[+] Handle: 0x%llx\n", (unsigned long long) hToken);
    }
    else
    {
        BeaconPrintf(CALLBACK_OUTPUT, "[+] Handle: 0x%llx (impersonation not applied)\n", (unsigned long long) hToken);
    }
}
