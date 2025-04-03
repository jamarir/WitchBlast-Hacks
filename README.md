# WitchBlast-Hacks

## TLDR

Witch Blast 0.7.5 Dll Injection Hacks PoC
```
Inject-x86.exe WitchBlast-CodeCaveDLL.dll 'Witch Blast.exe'
```

## Not-TLDR

This repo is nothing more than an implementation of the [Game Hacking Academy "Debugging & Reversing", "Programming", and "RTS/RPG Hacks" courses](https://gamehacking.academy/).

[Witch Blast](https://github.com/Cirrus-Minor/witchblast) is one of [many awesome open-source games](https://github.com/michelpereira/awesome-open-source-games), defined as a roguelike. This game is highly inspired by the "[The Binding of Isaac](https://en.wikipedia.org/wiki/The_Binding_of_Isaac_%28video_game%29)" game.

In a nutshell, the player spawns in the first floor of a multi-level labyrinth, procedurally-generated at each restart. The purpose is to find as much resources in every floor (perks, weapons, pills, money, items), and beat their respective boss.

These hacks are based upon the 0.7.5 version of the game, and includes:
- A gold hack, which makes every gathered coin valuable 10 golds (instead of 1).
- An invincibility hack, which makes the player (only) invincible, when hit by an enemy.
- A stat hack, which prints the current enemies' healths on top of the minimap. That way, the player can see how many health the enemies have left.
- A farm bot hack, which (hardly...) makes the player move toward enemies, and shoot at them if the player has enough range. This hack can be toggled (enabled / disabled) in-game pressing the RIGHT-CTRL key.

To inject this cheat in the game, you must first launch `Witch Blast.exe`, downloadable [here](https://github.com/Cirrus-Minor/witchblast/releases/tag/v0.7.5), and inject [the release DLL](https://github.com/jamarir/WitchBlast-Hacks/releases/tag/Debug) into the process using a [Dll Injector](https://github.com/adamhlt/DLL-Injector/releases/tag/DLL-Injector):
```
PS D:\WitchBlast-Hacks> Inject-x86.exe .\WitchBlast-CodeCaveDLL.dll 'Witch Blast.exe'
[DLL Injector]
Process : Witch Blast.exe
Process ID : 18548

[PROCESS INJECTION]
Process opened successfully.
Memory allocate at 0xE3E0000
DLL path writen successfully.
LoadLibraryA address at 0x76AD0E40
DLL Injected !
```
