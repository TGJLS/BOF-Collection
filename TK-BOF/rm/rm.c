#include <windows.h>
#include "beacon.h"
#include "bofdefs.h"

VOID go(IN PCHAR Buffer, IN ULONG Length)
{
    datap parser;
    DWORD token_handle = 0;

    BeaconDataParse(&parser, Buffer, Length);
    token_handle = (DWORD) BeaconDataInt(&parser);
}
