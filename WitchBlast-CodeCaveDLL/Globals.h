#pragma once
#include <windows.h>
#include <stdio.h>

extern HANDLE process_base;
extern unsigned char* hook_location;
extern DWORD old_protect;
extern DWORD trampoline;
extern DWORD original_call;

extern DWORD* player_base_addr;
extern DWORD* entity_base_addr;
extern DWORD* entity_health_addr;

extern int max_entities;
extern DWORD* entities_base_addr[30];
extern char entities_healths_str[30][5];
extern bool is_reset_entities_base_addr;
extern int i;

void reset_entities_base_addr();
void Install_CodeCave_Jump_Hook(IN HANDLE process_base, IN int hook_location_offset, IN int original_opcode_size, IN void* codecave_function);
void Globals_Initialization();