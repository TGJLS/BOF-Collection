#include <windows.h>
#include "beacon.h"
#include "bofdefs.h"

VOID go(IN PCHAR Buffer, IN ULONG Length)
{
    datap parser;
    DWORD pid      = 0;
    BOOL  no_apply = FALSE;

    BeaconDataParse(&parser, Buffer, Length);
    pid      = (DWORD) BeaconDataInt(&parser);
    no_apply = (BOOL)  BeaconDataInt(&parser);
}
