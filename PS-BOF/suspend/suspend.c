#include <windows.h>
#include "bofdefs.h"
#include "beacon.h"

void go(char *args, int len) {
    datap data_parser = {0};
    BeaconDataParse(&data_parser, args, len);
    INT32 pid = BeaconDataInt(&data_parser);

    HANDLE h = KERNEL32$OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, (DWORD)pid);
    if (!h) {
        BeaconPrintf(CALLBACK_ERROR, "OpenProcess failed: %d\n", KERNEL32$GetLastError());
        return;
    }

    NTSTATUS status = NTDLL$NtSuspendProcess(h);
    KERNEL32$CloseHandle(h);

    if (!NT_SUCCESS(status)) {
        BeaconPrintf(CALLBACK_ERROR, "NtSuspendProcess failed: 0x%08x\n", status);
        return;
    }
    BeaconPrintf(CALLBACK_OUTPUT, "Process suspended\n");
}
