/*
    main.c - RB4DX-Autoplay
    Initialisation code for the RB4DX autoplay plugin.
    Licensed under the GNU Lesser General Public License version 2.1, or later.
*/

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include "plugin_common.h"

static struct proc_info procInfo;

//autoplay hack
void(*SetGameOver)(void*, bool);

HOOK_INIT(SetGameOver);

void SetGameOver_hook(void* thisGame,  bool won) {

    if (file_exists("/data/GoldHEN/RB4DX/saving.ini")) {
        //for some reason checking if a file exists will make saving work
        HOOK_CONTINUE(SetGameOver, void (*)(void*, bool), thisGame, won);
        return;
    }
    //not checking if a file exists breaks saving, used when autoplay or other cheats are enabled
    HOOK_CONTINUE(SetGameOver, void (*)(void*, bool), thisGame, true);
    return;
}

void(*SetCheating)(void*, bool);

HOOK_INIT(SetCheating);

void SetCheating_hook(void* thisTrackWatcher, bool cheating) {
    bool autoplay = file_exists("/data/GoldHEN/RB4DX/autoplay.ini");

    HOOK_CONTINUE(SetCheating, void (*)(void*, bool), thisTrackWatcher, autoplay);
    return;
}

void(*RBVocalPlayerRestart)(void*, float, void*);
void(*SetAutoplay)(void*, bool);

HOOK_INIT(RBVocalPlayerRestart);

void RBVocalPlayerRestart_hook(void* thisRBVocalPlayer, float time, void* song) {
    bool autoplay = file_exists("/data/GoldHEN/RB4DX/autoplay.ini");

    SetAutoplay(thisRBVocalPlayer, autoplay);

    HOOK_CONTINUE(RBVocalPlayerRestart, void (*)(void*, float, void*), thisRBVocalPlayer, time, song);
    return;
}

void InitAutoplayHooks()
{
    sys_sdk_proc_info(&procInfo);

    SetGameOver = (void*)(procInfo.base_address + 0x00a48790);
    SetCheating = (void*)(procInfo.base_address + 0x0122dfc0);
    SetAutoplay = (void*)(procInfo.base_address + 0x00a65680);
    RBVocalPlayerRestart = (void*)(procInfo.base_address + 0x00a622f0);

    HOOK(SetGameOver);
    HOOK(SetCheating);
    HOOK(RBVocalPlayerRestart);
}

void DestroyAutoplayHooks()
{
    UNHOOK(SetGameOver);
    UNHOOK(SetCheating);
    UNHOOK(RBVocalPlayerRestart);
}
