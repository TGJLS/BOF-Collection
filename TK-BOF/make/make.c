#include <windows.h>
#include "beacon.h"
#include "bofdefs.h"

VOID go(IN PCHAR Buffer, IN ULONG Length)
{
    datap parser;
    char *username = NULL;
    char *password = NULL;
    char *domain   = NULL;
    BOOL  no_apply = FALSE;

    BeaconDataParse(&parser, Buffer, Length);
    username = BeaconDataExtract(&parser, NULL);
    password = BeaconDataExtract(&parser, NULL);
    domain   = BeaconDataExtract(&parser, NULL);
    no_apply = (BOOL) BeaconDataInt(&parser);
}
