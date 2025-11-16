#include "FarmBot_Hack.h"
#include <cmath>

/* == OBJECTIVE ==
This is just an ugly but funny PoC of a FarmBot in Witch Blast.
In a nutshell, our player goes towards an enemy, and fires at him if the player has enough range.
Many edge cases are not considered, so the path-finding towards enemies is definitely not optimized (and sometimes not solvable !).

FarmBot cheat can be toggled (enabled / disabled) if RIGHT-CTRL is pressed.
*/

float* player_attack_range_addr;
float* player_X_addr;
float* player_Y_addr;

DWORD* target_base_addr = NULL;
DWORD* target_health_addr = NULL;
float* target_X_addr = NULL;
float* target_Y_addr = NULL;
float deltaX = 0;
float deltaY = 0;
int move_delay = 1000;
int attack_delay = 10;
bool farmbot_enabled = false;


// Fixing a bug that sometimes crashes the game when the target is dead but we're doing calculation on its position.
bool is_target_alive() {
    return target_base_addr && target_health_addr && target_X_addr && target_Y_addr;
}

// this codecave hooks the move_entity function, so it may be any entity in the current room every frame.
__declspec(naked) void codecave_farmbot_hack_move_entity() {
    __asm {
        pushad
        pushf
    }

    // The player's position allows us to know where we are from our target.
    player_X_addr = (float*)((DWORD)*player_base_addr + 0x14);
    player_Y_addr = (float*)((DWORD)*player_base_addr + 0x18);

    // The player's attack range allows to keep some distance to the enemy
    player_attack_range_addr = (float*)((DWORD)*player_base_addr + 0x390);

    // If our target died, we set it to NULL.
    if (target_health_addr != NULL && *target_health_addr <= 0) {
        target_base_addr = NULL;
        target_health_addr = NULL;
        target_X_addr = NULL;
        target_Y_addr = NULL;
    }

    // we loop through all known enemies
    i = 0;
    while (target_base_addr == NULL && i < max_entities && entities_base_addr[i] != NULL) {
        __asm {
            mov esi, i // we need registers. memory to memory impossible
            mov eax, [entities_base_addr + esi * 4] // size of a DWORD, i.e. a slot in the array, is 4.
            add eax, 0x344 // the health is 0x344 ahead from the entity's base address
            mov target_health_addr, eax // Getting the entity's health
        }
        if (target_health_addr != NULL && *target_health_addr > 0) {
            // The target's position allows us to go towards it (seriously, you are writing a farmbot, ya know ??)
            target_base_addr = entities_base_addr[i];
            target_X_addr = (float*)((DWORD)target_base_addr + 0x14);
            target_Y_addr = (float*)((DWORD)target_base_addr + 0x18);
        }
        i++;
    }

    trampoline = (DWORD)process_base + 0x250F;
    __asm {
        popf
        popad
        add eax, edx
        sub esp, 0x04
        jmp trampoline
    }
}

void pressKey(char key) {
    INPUT input = { NULL };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    input.ki.dwFlags = KEYEVENTF_UNICODE; // Key Down
    SendInput(1, &input, sizeof(INPUT));
}
void releaseKey(char key) {
    INPUT input = { NULL };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    input.ki.dwFlags = KEYEVENTF_KEYUP; // Key Up
    SendInput(1, &input, sizeof(INPUT));
}


void sendKey(char key, int delay) {
    pressKey(key);
    Sleep(delay);
    releaseKey(key);
}

void approach_target() {
    // If player is close horizontally, approach vertically.
    if (deltaX < deltaY) {
        // approach if not so close
        if (deltaY > *player_attack_range_addr / 5) {
            if (is_target_alive() && *player_Y_addr < *target_Y_addr) {
                sendKey('S', move_delay);
            }
            else {
                sendKey('Z', move_delay);
            }
        }
    }
    // Otherwise, approach horizontally.
    else {
        // approach if not so close
        if (deltaX > *player_attack_range_addr / 5) {
            if (is_target_alive() && *player_X_addr < *target_X_addr) {
                sendKey('D', move_delay);
            }
            else {
                sendKey('Q', move_delay);
            }
        }
    }
}

/* A player atk range of :
- 700 goes from position 70 to 330 => 260 in distance.
- 350 goes from position 70 to 210 => 140 in distance.
- 1000 goes from position 70 to 430 => 350 in distance.

This isn't proportional, but we may approximate that 100 atk range <=> 35 distance. Said differently, 1/3 of the range covers positional distance.
Below functions allows to attack target.
*/
bool player_has_enough_attack_range_horizontally() {
    return *player_attack_range_addr / 3 > deltaX;
}

bool player_has_enough_attack_range_vertically() {
    return *player_attack_range_addr / 3 > deltaY;
}

void attack_target() {
    // If the X-delta between the player and the target is small, fight vertically
    if (deltaX < deltaY) {
        //if (player_has_enough_attack_range_vertically()) {
        if (is_target_alive() && *player_Y_addr < *target_Y_addr) {
            sendKey(VK_DOWN, attack_delay);
        }
        else {
            sendKey(VK_UP, attack_delay);
        }
    }
    // Otherwise, fight horizontally
    else {
        //if (player_has_enough_attack_range_horizontally()) {
        if (is_target_alive() && *player_X_addr < *target_X_addr) {
            sendKey(VK_RIGHT, attack_delay);
        }
        else {
            sendKey(VK_LEFT, attack_delay);
        }
        //}
    }
}

void farmbot_thread() {
    // Used to send keystrokes with the Windows's API.
    while (true) {
        // Activate the cheat if RIGHT-CTRL (VK_RCONTROL) is pressed.
        if (GetAsyncKeyState(VK_RCONTROL)) {
            Sleep(500); // avoid multiple toggles while pressing
            farmbot_enabled = !farmbot_enabled;
        }

        // If the cheat is enabled, and we have a target.
        if (farmbot_enabled && is_target_alive()) {
            deltaX = std::abs(*player_X_addr - *target_X_addr);
            deltaY = std::abs(*player_Y_addr - *target_Y_addr);
            approach_target();
            attack_target();
        }
    }
    Sleep(10); // avoid swallow the whole CPU
}

void FarmBot_Hack() {
    Install_CodeCave_Jump_Hook(process_base, 0x250A, 5, codecave_farmbot_hack_move_entity);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)farmbot_thread, NULL, 0, NULL);
}