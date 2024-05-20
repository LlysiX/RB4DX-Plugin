/*
    main.c - RB4DX-Autoplay
    Initialisation code for the RB4DX autoplay plugin.
    Licensed under the GNU Lesser General Public License version 2.1, or later.
*/

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define attr_module_hidden __attribute__((weak)) __attribute__((visibility("hidden")))
#define attr_public __attribute__((visibility("default")))

#define GOLDHEN_PATH "/data/GoldHEN"
#define RB4DX_PATH GOLDHEN_PATH "/RB4DX"
#define PLUGIN_CONFIG_PATH RB4DX_PATH "/RB4DX.ini"
#define PLUGIN_NAME "RB4DX"
#define final_printf(a, args...) klog("[" PLUGIN_NAME "] " a, ##args)

#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>

attr_public const char *g_pluginName = PLUGIN_NAME;
attr_public const char *g_pluginDesc = "Plugin for loading Rock Band 4 Deluxe autoplay";
attr_public const char *g_pluginAuth = "LysiX";
attr_public uint32_t g_pluginVersion = 0x00000100; // 1.00

bool file_exists(const char* filename) {
    struct stat buff;
    return stat(filename, &buff) == 0 ? true : false;
}

void DoNotificationStatic(const char* text) {
    bool NotifyColored = file_exists("/data/GoldHEN/RB4DX/notifycolored.ini");
    OrbisNotificationRequest Buffer = { 0 };
    Buffer.useIconImageUri = 1;
    Buffer.targetId = -1;
    strcpy(Buffer.message, text);
    if (NotifyColored)
        strcpy(Buffer.iconUri, "https://raw.githubusercontent.com/hmxmilohax/rock-band-3-deluxe/100069d1c2293424a659ecb4a5ddacc3b91c4f9b/dependencies/media/dx.png");
    else
        strcpy(Buffer.iconUri, "https://raw.githubusercontent.com/hmxmilohax/RB2DX-Site/7d707e0d8e6f1c47c9e1eb187237ad934f254f92/docs/apple-touch-icon.png");
    sceKernelSendNotificationRequest(0, &Buffer, sizeof(Buffer), 0);
}

void DoNotification(const char *FMT, ...) {
    bool NotifyColored = file_exists("/data/GoldHEN/RB4DX/notifycolored.ini");
    OrbisNotificationRequest Buffer = { 0 };
    va_list args;
    va_start(args, FMT);
    vsprintf(Buffer.message, FMT, args);
    va_end(args);
    Buffer.type = NotificationRequest;
    Buffer.unk3 = 0;
    Buffer.useIconImageUri = 1;
    Buffer.targetId = -1;
    if (NotifyColored)
        strcpy(Buffer.iconUri, "https://raw.githubusercontent.com/hmxmilohax/rock-band-3-deluxe/100069d1c2293424a659ecb4a5ddacc3b91c4f9b/dependencies/media/dx.png");
    else
        strcpy(Buffer.iconUri, "https://raw.githubusercontent.com/hmxmilohax/RB2DX-Site/7d707e0d8e6f1c47c9e1eb187237ad934f254f92/docs/apple-touch-icon.png");
    sceKernelSendNotificationRequest(0, &Buffer, sizeof(Buffer), 0);
}

static struct proc_info procInfo;

//autoplay hack
void(*SetGameOver)(void*, bool);

HOOK_INIT(SetGameOver);

void SetGameOver_hook(void* thisGame,  bool won) {

    if (file_exists("/data/GoldHEN/RB4DX/fake.ini")) {
        //for some reason this breaks scoring entirely no matter what file i put in
        HOOK_CONTINUE(SetGameOver, void (*)(void*, bool), thisGame, false);
        return;
    }
    HOOK_CONTINUE(SetGameOver, void (*)(void*, bool), thisGame, true);
    return;
}

//void(*ExportGameEnded)(void*, void*, void*, bool);
//
//HOOK_INIT(ExportGameEnded);
//
//void ExportGameEnded_hook(void* thisRBGameData, void* Game, void* RBSong, bool won) {
//    HOOK_CONTINUE(ExportGameEnded, void (*)(void*, void*, void*, bool), thisRBGameData, Game, RBSong, true);
//    return;
//}

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

#define ADDR_OFFSET 0x00400000
int32_t attr_public module_start(size_t argc, const void *args)
{
    if (sys_sdk_proc_info(&procInfo) != 0) {
        final_printf("Failed to get process info!\n");
        return 0;
    }

    sys_sdk_proc_info(&procInfo);
    final_printf("Started plugin! Title ID: %s\n", procInfo.titleid);
    if (strcmp(procInfo.titleid, "CUSA02084") == 0) {
        final_printf("US Rock Band 4 Detected!\n");
    }
    else if (strcmp(procInfo.titleid, "CUSA02901") == 0) {
        final_printf("EU Rock Band 4 Detected!\n");
    }
    else {
        final_printf("Game loaded is not Rock Band 4!\n");
        return 0;
    }
    
    if (strcmp(procInfo.version, "02.21") != 0) {
        final_printf("This plugin is only compatible with version 02.21 of Rock Band 4.\n");
        return 0;
    }

    final_printf("Applying RB4DX Autoplay hooks...\n");
    DoNotificationStatic("RB4DX Autoplay loaded!");

    SetGameOver = (void*)(procInfo.base_address + 0x00a48790);
    SetCheating = (void*)(procInfo.base_address + 0x0122dfc0);
    SetAutoplay = (void*)(procInfo.base_address + 0x00a65680);
    RBVocalPlayerRestart = (void*)(procInfo.base_address + 0x00a622f0);
    //ExportGameEnded = (void*)(procInfo.base_address + 0x009648d0);

    // apply all hooks
    HOOK(SetGameOver);
    HOOK(SetCheating);
    HOOK(RBVocalPlayerRestart);
    //HOOK(ExportGameEnded);

    return 0;
}

int32_t attr_public module_stop(size_t argc, const void *args)
{
    final_printf("Stopping plugin...\n");
    // unhook everything just in case
    UNHOOK(SetGameOver);
    UNHOOK(SetCheating);
    UNHOOK(RBVocalPlayerRestart);
    //UNHOOK(ExportGameEnded);
    return 0;
}
