#include <windows.h>
#include "beacon.h"
#include "bofdefs.h"

VOID go(IN PCHAR Buffer, IN ULONG Length)
{
    datap parser;
    HANDLE token_handle = NULL;

    BeaconDataParse(&parser, Buffer, Length);
    token_handle = (HANDLE)(ULONG_PTR)(DWORD) BeaconDataInt(&parser);
}
