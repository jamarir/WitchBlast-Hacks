#include "Gold_Hack.h"
#include "Invicibility_Hack.h"
#include "Stat_Hack.h"
#include "FarmBot_Hack.h"

/*
Just another Game Hack project.
*/

// N.B.: CPP files MUST be configured with "Not Using Precompiled Headers".
BOOL WINAPI DllMain(HMODULE hModule, DWORD  fdwReason, LPVOID lpReserved)
{
    Globals_Initialization(fdwReason);

    Gold_Hack(fdwReason);
    Invicibility_Hack(fdwReason);
    Stat_Hack(fdwReason);
    FarmBot_Hack(fdwReason);

    return true;
}