#include "Globals.h"

// Global variables for any Game Hack.
HANDLE process_base = GetModuleHandle(L"Witch Blast.exe");
unsigned char* hook_location;
DWORD old_protect;
DWORD trampoline;
DWORD original_call;

// Global variables shared between some Witch Blast Hacks.
DWORD* player_base_addr;
DWORD* entity_base_addr;
DWORD* entity_health_addr;

// arrays' sizes must be fixed at compilation. using hard-coded allows to keep consistency.
int max_entities = 30;
// addresses of each known enemies of the current room.
DWORD* entities_base_addr[30] = { NULL };
// Stores a maximum number of enemies' healths (30 should be enough for any room), where health is 4 chars max + NULL byte each
// When initialized to NULL, it's actually just length*4 NULL bytes in memory.
char entities_healths_str[30][5] = { NULL };

// true only when the battle ends (e.g. the gates are opening). 
// Used to restart the enemies' entities. 
// The corresponding opcode is: 
// -> 004AFDF9 | E8 22CFFBFF             | call witch blast.46CD20            | display_opening_gates:1.0
bool is_reset_entities_base_addr = false;

int i;
bool known_entity;

__declspec(naked) void codecave_move_entity() {
	// This codecave hooks the move_entity function, where may be any entity (player or enemy) in the current room, at each frame.
	// Used to initialize an array of entities in the current room.
	// When hooking, ebx is the moving entity's address base. We save it in our local array (for every entity in the map)
	__asm {
		pushf
		pushad
		mov entity_base_addr, ebx;
	}

	if (is_reset_entities_base_addr) {
		reset_entities_base_addr();
	}

	// We don't perform the StatHack for the player, and already known enemies healths in the list.
	// That an ugly but fancy way to implement a set / vector list.
	known_entity = false;
	i = 0;
	while (i < max_entities && !known_entity && entities_base_addr[i] != NULL) {
		if (entities_base_addr[i] == entity_base_addr) {
			known_entity = true;
		}
		i++;
	}

	// if the moving entity triggering the breakpoint is unknown and not player, add it to the array.
	// player_base_addr is a POINTER to our pointer to our player. The entity_base_addr is the final pointer directly.
	if (!known_entity && entity_base_addr != (DWORD*)*player_base_addr) {
		entities_base_addr[i] = entity_base_addr;
	}

	trampoline = (DWORD)process_base + 0x2505;
	__asm {
		popad
		popf
		mov eax, dword ptr ds : [ebx + 0x170]
		jmp trampoline
	}
}

// we reset the array of addresses of enemies to NULL (the first one is the player, we can just skip it) if the battle ends
__declspec(naked) void codecave_open_gates() {
	__asm {
		pushad
	}

	is_reset_entities_base_addr = true;

	trampoline = (DWORD)process_base + 0xAFDFE;
	original_call = (DWORD)process_base + 0x6CD20;
	__asm {
		popad
		call original_call
		jmp trampoline
	}
}

// We reset the enemies' addresses array to NULL if an entity died (either the player or enemy, that's a shared function).
// -> EDGE CASE: if the player dies, then at least an enemy is in the room, which doesn't reset the array. To avoid crashes, we reset the array.
__declspec(naked) void codecave_kill_entity() {
	__asm {
		pushad
	}

	is_reset_entities_base_addr = true;

	trampoline = (DWORD)process_base + 0x4E5F;
	__asm {
		popad
		mov dword ptr ds : [edx + 0x344] , 0
		jmp trampoline
	}
}

// One condition to reset the minimap is to check if a game starts.
__declspec(naked) void codecave_load_level() {
	__asm {
		pushad
	}

	is_reset_entities_base_addr = true;

	trampoline = (DWORD)process_base + 0x9A19F;
	original_call = (DWORD)process_base + 0x97B60;
	__asm {
		popad
		call original_call
		jmp trampoline
	}
}



// N.B.: This is a local asm code, it doesn't have a ret instruction !! we must add it manually when we call it (otherwise, the game crashes when called with INT3).
__declspec(naked) void reset_entities_base_addr() {
	__asm {
		pushad
	}
	// We first reset the array of addresses to enemies to NULL using the other code caves triggers.
	// This must be done first.
	// If we use the upper limit sizeof(entities_base_addr), it goes from 0 to 4*max_entities..., while using sizeof((DWORD*)entities_base_addr) goes from 0 to 4.
	// Then, let's just reset it byte by byte, less readable but consistent...
	for (i = 0; i < sizeof(entities_base_addr); i++) {
		*((BYTE*)entities_base_addr + i) = NULL;
	}
	__asm {
		popad
		ret
	}
}


void WINAPI Globals_Initialization(DWORD fdwReason) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		// Get the player's health value based off the base pointer
		player_base_addr = (DWORD*)((DWORD)process_base + 0x341E90);
		player_base_addr = (DWORD*)(*player_base_addr + 0x74);
		player_base_addr = (DWORD*)(*player_base_addr + 0x16C);

		// First hook: write the enemies' healths in the level text.
		hook_location = (unsigned char*)((DWORD)process_base + 0x24FF);
		VirtualProtect((void*)hook_location, 6, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9;
		// don't forget to PRECISE THE EXACT CASTING SIZE when overwriting the opcode to avoid writing more than needed (DWORD*, BYTE* ...)
		*(DWORD*)(hook_location + 1) = (DWORD)&codecave_move_entity - ((DWORD)hook_location + 5);
		// original opcode is 6 bytes, we NOP-pad.
		*(BYTE*)(hook_location + 5) = { 0x90 };

		// Second hook: check if the gates are opening to reset the known enemies to NULL, as they're no more enemies in the map.
		hook_location = (unsigned char*)((DWORD)process_base + 0xAFDF9);
		VirtualProtect((void*)hook_location, 5, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9;
		*(DWORD*)(hook_location + 1) = (DWORD)&codecave_open_gates - ((DWORD)hook_location + 5);

		// Third hook: check if the player died to reset the known enemies to NULL, as we're no longer looking for the StatHack.
		hook_location = (unsigned char*)((DWORD)process_base + 0x4E55);
		VirtualProtect((void*)hook_location, 10, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9; // 1 byte written
		*(DWORD*)(hook_location + 1) = (DWORD)&codecave_kill_entity - ((DWORD)hook_location + 5); // 4 bytes written
		// original opcode is 10 (1+4+5) bytes, we NOP-pad it.
		for (i = 0; i < 4; i++) {
			*(BYTE*)(hook_location + 5 + i) = { 0x90 }; // 5 byte written
		}

		// Fourth hook: check if a new game is loaded to reset the known enemies to NULL, as there might be enemies left in the map if we die ("Use-After-Reallocated" pointer crash).
		hook_location = (unsigned char*)((DWORD)process_base + 0x9A19A);
		VirtualProtect((void*)hook_location, 5, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9;
		*(DWORD*)(hook_location + 1) = (DWORD)&codecave_load_level - ((DWORD)hook_location + 5);
	}
}