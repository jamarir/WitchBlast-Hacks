#include "Gold_Hack.h"

/* == OBJECTIVE ==
When the player gathers a coin, his gold is increased by 1.
This code makes the coin valuable 10 golds.
*/

/*
Original opcode:
- increasing our gold by 1 when gathering a coin: 0044A7F2 | 8381 C4030000 01        | add dword ptr ds:[ecx+3C4],1              | gather_coin:0.0
patching the last byte would increase a coin's value.
*/

void WINAPI Gold_Hack(DWORD fdwReason) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		hook_location = (unsigned char*)((DWORD)process_base + 0x4A7F9);
		VirtualProtect((void*)hook_location, 1, PAGE_EXECUTE_READWRITE, &old_protect);
		// one coin is valuable as 10
		*(BYTE*)(hook_location - 1) = { 0x0a };
	}
}
