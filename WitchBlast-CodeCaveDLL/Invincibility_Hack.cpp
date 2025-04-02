#include "Invicibility_Hack.h"

/* == OBJECTIVE ==
This code makes the player invincible, while keeping the enemy vulnerable.
N.B.: The function hitting an entity is shared between the player and the enemies.
*/


/* Original opcode decreasing p1's health AND ENEMY is :
	- 0040439F | 8982 44030000           | mov dword ptr ds:[edx+344],eax   | hit_player:0.0

NOP'ing it out will make enemy invicible as well
Then, we first need to make sure the address edx+344 is NOT our p1's health address found in cheat engine.
*/

__declspec(naked) void codecave_invicibility_hack_hit_entity() {
	// we save all the registers (pushad) AND FLAGS (pushf) to prevents crashes when we alter the flags with our codecave's conditions below !
	// when hooked, edx is the base of the hit entity
	__asm {
		pushad
		pushf
		mov entity_base_addr, edx
	}

	trampoline = (DWORD)process_base + 0x43A5;
	// If the hit entity is the player's health address, jump directly to trampoline
	// BUT this alters original EFLAGS and get us killed by in-game health check. Thus, we first restore the original flags (popf)
	if (entity_base_addr == (DWORD*)*player_base_addr) {
		__asm {
			popf
			popad
			jmp trampoline
		}
	}
	// Otherwise, execute the original opcode we overwrote (decrease entity's health) and jump to trampoline.
	else {
		__asm {
			popf
			popad
			mov dword ptr ds : [edx + 0x344] , eax
			jmp trampoline
		}
	}
}

// We'll hook a bit after the move entity of StatHack, because StatHack uses it to print the enemies' healths in minimap.
void WINAPI Invicibility_Hack(DWORD fdwReason) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		hook_location = (unsigned char*)((DWORD)process_base + 0x439F);
		VirtualProtect((void*)hook_location, 6, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9;
		*(DWORD*)(hook_location + 1) = (DWORD)&codecave_invicibility_hack_hit_entity - ((DWORD)hook_location + 5);
		// original opcode is 6 bytes, we fill 1 NOP.
		*(BYTE*)(hook_location + 5) = { 0x90 };
	}
}