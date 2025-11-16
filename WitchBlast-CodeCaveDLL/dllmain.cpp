#include "Gold_Hack.h"
#include "Invicibility_Hack.h"
#include "Stat_Hack.h"
#include "FarmBot_Hack.h"

/*
Just another Game Hacking project.
*/

// N.B.: 
//      CPP files MUST be configured with "Not Using Precompiled Headers".
//      The following Post-Build Event can be used to automatically hook the process: "powershell -c "Start-Process -WorkingDir 'D:\Witch Blast v0.7.5\' -FilePath '.\Witch Blast.exe'; Start-Sleep 2; Inject-x86.exe $(TargetPath) 'Witch Blast.exe'""
BOOL WINAPI DllMain(HMODULE hModule, DWORD  fdwReason, LPVOID lpReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        Globals_Initialization();
        Gold_Hack();
        Invicibility_Hack();
        Stat_Hack();
        FarmBot_Hack();
    }
    return true;
}