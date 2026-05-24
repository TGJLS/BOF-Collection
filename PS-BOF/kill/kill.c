#include <windows.h>
#include "bofdefs.h"
#include "beacon.h"

void go(char *args, int len)
{
    datap  data_parser    = {0};
    INT32  process_id;
    INT32  process_exitcode;
    HANDLE h;
    BOOL   ok;

    BeaconDataParse(&data_parser, args, len);
    process_id       = BeaconDataInt(&data_parser);
    process_exitcode = BeaconDataInt(&data_parser);

    h = KERNEL32$OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)process_id);
    if (!h) {
        BeaconPrintf(CALLBACK_ERROR, "OpenProcess failed: %d\n", KERNEL32$GetLastError());
        return;
    }

    ok = KERNEL32$TerminateProcess(h, (UINT)process_exitcode);
    KERNEL32$CloseHandle(h);

    if (!ok) {
        BeaconPrintf(CALLBACK_ERROR, "TerminateProcess failed: %d\n", KERNEL32$GetLastError());
        return;
    }

    BeaconPrintf(CALLBACK_OUTPUT, "Process killed\n");
}
