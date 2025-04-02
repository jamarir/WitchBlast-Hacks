#include "Stat_Hack.h"

/* == OBJECTIVE ==
This code print the current room's enemies' healths on top of the minimap.
That way, the player can keep track of the enemies' healths while fighting.
*/

/*
Original opcode:
- move an entity in the map (player or monster):
	004024FC | FF50 5C                 | call dword ptr ds:[eax+5C]            | move_entity:0.0=move_left_entity:1.0

The text at the top of minimap is static (i.e. either static or global C/C++ var) !! Stored at "Witch Blast.exe"+002DF8D0

We wanna get the HP of each enemy and put it into the minimap's text (stored at ebx+344 in next opcode):
-> 004024FF | 8B83 70010000           | mov eax,dword ptr ds:[ebx+170]        |
*/

char* minimap_text = (char*)((DWORD)process_base + 0x2DF8D0);
// keep a backup of the THE WHOLE ORIGINAL WRITABLE TEXT.
char* minimap_text_original = (char*)malloc(sizeof(entities_healths_str));
int c, offset;

// this codecave hooks the move_entity function, so it may be any entity in the current room every frame.
__declspec(naked) void codecave_stat_hack_move_entity() {
	// When hooking, ebx is the moving entity addr base. Save it in our local array (for every entity in the map)
	__asm {
		pushf
		pushad
	}

	// Check if the array has been reset to reset the minimap text.
	if (is_reset_entities_base_addr) {
		memcpy(minimap_text, minimap_text_original, sizeof(entities_healths_str));
		is_reset_entities_base_addr = false;
	}

	i = 0;
	while (i < max_entities && entities_base_addr[i] != NULL) {
		__asm {
			mov esi, i // we need registers. memory to memory impossible
			mov eax, [entities_base_addr + esi * 4] // size of a DWORD, i.e. a slot in the array, is 4.
			add eax, 0x344 // the health is 0x344 ahead from the entity's base address
			mov entity_health_addr, eax // getting the entity's health
		}
		// we retrieve the health in text format
		sprintf_s(entities_healths_str[i], sizeof(entities_healths_str[i]), "%04d", *entity_health_addr); // max string is 5 = 4 + NULL.
		// Restoring snprintf's call stack alteration
		__asm {
			popad
			pushad
		}
		for (c = 0; c < sizeof(entities_healths_str[i]); c++) {
			offset = i * sizeof(entities_healths_str[i]); // offset keeps track of the considered entity with i.
			*(minimap_text + c + offset) = ((char)*(entities_healths_str[i] + c));
			// we don't want a NULL byte at the end of the string, otherwise it's not shown in minimap text.
			// we want a separator, e.g. |.
			// we add the NULL byte ONLY for the last known entity.
			if (c + 1 == sizeof(entities_healths_str[i])) {
				if (i + 1 == max_entities || entities_base_addr[i + 1] == NULL) {
					*(minimap_text + c + offset) = NULL;
				}
				else {
					*(minimap_text + c + offset) = '|';
				}
			}
		}
		i++;
	}

	trampoline = (DWORD)process_base + 0x250A;
	__asm {
		popad
		popf
		mov edx, eax
		shr edx, 0x1F
		jmp trampoline
	}
}

void WINAPI Stat_Hack(DWORD fdwReason) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		// we wanna write any characters in the level text. The max amount to be writable is MAX_ENTITY*MAX_STR_LENGTH.
		// first we backup the original text
		VirtualProtect((void*)minimap_text, sizeof(entities_healths_str), PAGE_EXECUTE_READWRITE, &old_protect);
		memcpy(minimap_text_original, minimap_text, sizeof(entities_healths_str));

		// We'll hook a bit after the move entity of Globals, because Globals uses it to get the enemies base addresses already.
		hook_location = (unsigned char*)((DWORD)process_base + 0x2505);
		VirtualProtect((void*)hook_location, 5, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9;
		*(DWORD*)(hook_location + 1) = (DWORD)&codecave_stat_hack_move_entity - ((DWORD)hook_location + 5);
	}
}