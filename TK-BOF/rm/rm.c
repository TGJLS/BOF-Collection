#include <windows.h>
#include "beacon.h"
#include "bofdefs.h"

VOID go(IN PCHAR Buffer, IN ULONG Length)
{
    datap parser;
    HANDLE token_handle = NULL;

    BeaconDataParse(&parser, Buffer, Length);
    token_handle = (HANDLE)(ULONG_PTR)(DWORD) BeaconDataInt(&parser);

    NTSTATUS status;
    status = NTDLL$NtClose(token_handle);
    if (status < 0)
    {
        BeaconPrintf(CALLBACK_ERROR, "[-] rm: NtClose failed: 0x%lx\n", (ULONG) status);
        return;
    }

    BeaconPrintf(CALLBACK_OUTPUT, "[+] Handle 0x%lx closed.\n", (ULONG_PTR) token_handle);
}
