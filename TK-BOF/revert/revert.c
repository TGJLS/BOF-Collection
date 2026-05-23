#include <windows.h>
#include "beacon.h"
#include "bofdefs.h"

VOID go(IN PCHAR Buffer, IN ULONG Length)
{
    ADVAPI32$RevertToSelf();
    BeaconPrintf(CALLBACK_OUTPUT, "[+] Reverted to process token.\n");
}
