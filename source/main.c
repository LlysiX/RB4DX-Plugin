/*
    main.c - RB4DX
    Initialisation code for the RB4DX plugin.
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

#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include <orbis/Pad.h>
#include "plugin_common.h"
#include "DTAFuncs.h"
#include "rb4/songmetadata.h"
#include "rb4/gemsmasher.h"

attr_public const char *g_pluginName = PLUGIN_NAME;
attr_public const char *g_pluginDesc = "Plugin for loading Rock Band 4 Deluxe files, among other enhancements.";
attr_public const char *g_pluginAuth = "LysiX";
attr_public uint32_t g_pluginVersion = 0x00000100; // 1.00

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

bool USTitleID = true;

// ARKless file loading hook
const char* RawfilesFolder = "/data/GoldHEN/RB4DX/";
const char* GameRawfilesFolder = "data:/GoldHEN/RB4DX/";
bool PrintRawfiles = false;
bool PrintArkfiles = false;

typedef enum {
    kRead = 0x0,
    kReadNoArk = 0x1,
    kReadNoBuffer = 0x2,
    kAppend = 0x3,
    kWrite = 0x4,
    kWriteNoBuffer = 0x5
} FileMode;

void* (*NewFile)(const char*, FileMode);

HOOK_INIT(NewFile);

void NewFile_hook(const char* path, FileMode mode) {
    char rawpath[2048];
    strcpy(rawpath, RawfilesFolder);
    /*if (rawpath[strlen(rawpath) - 1] != '/') {
        strcat(rawpath, "/");
    }*/
    strcat(rawpath, path);
    char gamepath[2048];
    strcpy(gamepath, GameRawfilesFolder);
    strcat(gamepath, path);
    const char* newpath = rawpath;
    if (file_exists(newpath)) {
        if (PrintRawfiles) final_printf("Loading rawfile: %s\n", newpath);
        HOOK_CONTINUE(NewFile, void (*)(const char*, FileMode), gamepath, kReadNoArk);
        return;
    }
    if (PrintArkfiles || (PrintRawfiles && mode == kReadNoArk)) final_printf("Loading file: %s\n", path);
    HOOK_CONTINUE(NewFile, void (*)(const char*, FileMode), path, mode);
    return;
}

//speed hack
void(*GameRestart)(void*, bool);
void(*SetMusicSpeed)(void*, float);

HOOK_INIT(GameRestart);

void GameRestart_hook(void* thisGame, bool restart) {
    HOOK_CONTINUE(GameRestart, void (*)(void*, bool), thisGame, restart);
    float speed = 1.00;
    bool autoplay = file_exists("/data/GoldHEN/RB4DX/autoplay.ini");
    bool drunkmode = file_exists("/data/GoldHEN/RB4DX/drunkmode.ini");
    bool insong = file_exists("/data/GoldHEN/RB4DX/insong.dta");
    bool speedfile = file_exists("/data/GoldHEN/RB4DX/speedmod.ini");

    if (speedfile) {
        FILE* spdfptr;
        spdfptr = fopen("/data/GoldHEN/RB4DX/speedmod.ini", "r");
        char spdstring[100];
        fgets(spdstring, 100, spdfptr);
        const char* spdstringptr = spdstring;
        speed = atof(spdstringptr);
        fclose(spdfptr);
    }


    if (speed > 0.00 && speed != 1.00){
        SetMusicSpeed(thisGame, speed);
        final_printf("Music speed: %.2f\n", speed);
        if (!insong || autoplay || drunkmode)
            DoNotification("Music Speed Set: %.2f", speed);
    }
    if (autoplay) {
        final_printf("Autoplay Enabled!\n");
        if (!insong)
            DoNotificationStatic("Autoplay Enabled!");
    }
    return;
}

//song title hook for reporting speedhack/autoplay
char* (*GetTitle)(SongMetadata*);
const char* autoplaytitle = " (AUTOPLAY)";
const char* drunkmodetitle = " (DRUNK MODE)";

HOOK_INIT(GetTitle);

char* GetTitle_hook(SongMetadata* thisMetadata) {
    bool insong = file_exists("/data/GoldHEN/RB4DX/insong.dta");
    if (!insong)
        return  thisMetadata->mTitle;

    bool speedfile = file_exists("/data/GoldHEN/RB4DX/speedmod.ini");
    float speed = 1.00;
    char speedtitleint[512];
    char speedtxt[512];
    char* speedtitle;
    strcpy(speedtitleint, thisMetadata->mTitle);

    bool autoplay = file_exists("/data/GoldHEN/RB4DX/autoplay.ini");
    char aptitleint[512];
    strcpy(aptitleint, thisMetadata->mTitle);
    strcat(aptitleint, autoplaytitle);
    char* aptitle = aptitleint;

    bool drunkmode = file_exists("/data/GoldHEN/RB4DX/drunkmode.ini");
    char dmtitleint[512];
    strcpy(dmtitleint, thisMetadata->mTitle);
    strcat(dmtitleint, drunkmodetitle);
    char* dmtitle = dmtitleint;


    if (speedfile) {
        FILE* spdfptr;
        spdfptr = fopen("/data/GoldHEN/RB4DX/speedmod.ini", "r");
        char spdstring[100];
        fgets(spdstring, 100, spdfptr);
        const char* spdstringptr = spdstring;
        speed = atof(spdstringptr) * 100;
        fclose(spdfptr);
    }

    //final_printf("songtitle: %s\n", thisMetadata->mTitle);
    //final_printf("apsongtitle: %s\n", aptitle);

    if (insong && autoplay)
        //include " (AUTOPLAY)" at the end of the song title
        return aptitle;
    else if (insong && drunkmode)
        //include " (DRUNK MODE)" at the end of the song title
        return dmtitle;
    else if (insong && speed > 0 && speed != 100) {
        // include " (x% Speed)" at the end of the song title
        sprintf(speedtxt, " (%.0f%% Speed)", speed);
        strcat(speedtitleint, speedtxt);
        speedtitle = speedtitleint;
        return speedtitle;
    }
    else
        return thisMetadata->mTitle;
}

//read lightbar status

int(*TscePadSetLightBar)(int handle, OrbisPadColor *inputColor);

HOOK_INIT(TscePadSetLightBar);

void TscePadSetLightBar_hook(int handle, OrbisPadColor *inputColor) {
    //final_printf("Set Light Bar Color:\n"); //disabled due to log spam
    //final_printf("R: %d\n", inputColor->r);
    //final_printf("G: %d\n", inputColor->g);
    //final_printf("B: %d\n", inputColor->b);
    scePadSetLightBar(handle, inputColor);
    return;
}

// Custom gem colors from RBVREnhanced, updated to set gems individually instead of all at once

bool(*UpdateColors)(RBGemSmasherCom* thiscom);

HOOK_INIT(UpdateColors);

bool UpdateColors_hook(RBGemSmasherCom* thiscom) {

    bool r = HOOK_CONTINUE(UpdateColors, bool(*)(RBGemSmasherCom*), thiscom);

    //final_printf("Smasher color detected!\n");
    //final_printf("R: %f\n", thiscom->mColor.r);
    //final_printf("G: %f\n", thiscom->mColor.g);
    //final_printf("B: %f\n", thiscom->mColor.b);

    char coltest[9];
    sprintf(coltest, "%f", thiscom->mColor.r);

    //green
    if (strcmp(coltest, "0.130136") == 0) {
        thiscom->mColor.r = 1.0;
        thiscom->mColor.g = 0.1;
        thiscom->mColor.b = 1.0;
        thiscom->mColor.a = 1.0;
    }
    //red
    if (strcmp(coltest, "0.896269") == 0) {
        thiscom->mColor.r = 1.0;
        thiscom->mColor.g = 0.5;
        thiscom->mColor.b = 0.0;
        thiscom->mColor.a = 1.0;
    }
    //yellow
    if (strcmp(coltest, "0.791298") == 0) {
        thiscom->mColor.r = 0.01;
        thiscom->mColor.g = 0.01;
        thiscom->mColor.b = 0.01;
        thiscom->mColor.a = 1.0;
    }
    //blue
    if (strcmp(coltest, "0.011612") == 0) {
        thiscom->mColor.r = 1.0;
        thiscom->mColor.g = 0.5;
        thiscom->mColor.b = 0.0;
        thiscom->mColor.a = 1.0;
    }
    //orange
    if (strcmp(coltest, "0.745404") == 0) {
        thiscom->mColor.r = 1.0;
        thiscom->mColor.g = 0.1;
        thiscom->mColor.b = 1.0;
        thiscom->mColor.a = 1.0;
    }

    return r;
}

bool (*DoSetColor)(void* component, void* proppath, void* propinfo, Color* color, Color* toset, bool param_6);

HOOK_INIT(DoSetColor);

bool DoSetColor_hook(void* component, void* proppath, void* propinfo, Color* color, Color* toset, bool param_6) {
    Color newcolorg;
    newcolorg.r = 1.0;
    newcolorg.g = 0.1;
    newcolorg.b = 1.0;
    newcolorg.a = 1.0;
    Color newcolorr;
    newcolorr.r = 1.0;
    newcolorr.g = 0.5;
    newcolorr.b = 0.0;
    newcolorr.a = 1.0;
    Color newcolory;
    newcolory.r = 0.01;
    newcolory.g = 0.01;
    newcolory.b = 0.01;
    newcolory.a = 1.0;
    Color newcolorb;
    newcolorb.r = 1.0;
    newcolorb.g = 0.5;
    newcolorb.b = 0.0;
    newcolorb.a = 1.0;
    Color newcoloro;
    newcoloro.r = 1.0;
    newcoloro.g = 0.1;
    newcoloro.b = 1.0;
    newcoloro.a = 1.0;
    char coltest[9];
    sprintf(coltest, "%f", toset->r);
    //green
    if (strcmp(coltest, "0.130136") == 0) return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, &newcolorg, param_6);
    if (strcmp(coltest, "0.242281") == 0) return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, &newcolorg, param_6);
    //red
    if (strcmp(coltest, "0.896269") == 0) return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, &newcolorr, param_6);
    //yellow
    if (strcmp(coltest, "0.791298") == 0) return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, &newcolory, param_6);
    //blue
    if (strcmp(coltest, "0.011612") == 0) return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, &newcolorb, param_6);
    if (strcmp(coltest, "0.004025") == 0) return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, &newcolorb, param_6);
    //orange
    if (strcmp(coltest, "0.745404") == 0) return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, &newcoloro, param_6);
    if (strcmp(coltest, "0.814847") == 0) return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, &newcoloro, param_6);
    return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, toset, param_6);
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
        USTitleID = true;
    }
    else if (strcmp(procInfo.titleid, "CUSA02901") == 0) {
        final_printf("EU Rock Band 4 Detected!\n");
        USTitleID = false;
    }
    else {
        final_printf("Game loaded is not Rock Band 4!\n");
        return 0;
    }
    
    if (strcmp(procInfo.version, "02.21") != 0) {
        final_printf("This plugin is only compatible with version 02.21 of Rock Band 4.\n");
        return 0;
    }

    final_printf("Applying RB4DX hooks...\n");
    DoNotificationStatic("RB4DX Plugin loaded!");

    NewFile = (void*)(procInfo.base_address + 0x00376d40);
    GameRestart = (void*)(procInfo.base_address + 0x00a46710);
    GetTitle = (void*)(procInfo.base_address + 0x00f28d20);
    SetMusicSpeed = (void*)(procInfo.base_address + 0x00a470e0);
    TscePadSetLightBar = (void*)(procInfo.base_address + 0x012450d0);
    UpdateColors = (void*)(procInfo.base_address + 0x00f94a70);
    DoSetColor = (void*)(procInfo.base_address + 0x001a7320);

    // apply all hooks
    InitDTAHooks();
    HOOK(GameRestart);
    HOOK(GetTitle);
    HOOK(NewFile);
    HOOK(TscePadSetLightBar);
    HOOK(UpdateColors);
    HOOK(DoSetColor);

    return 0;
}

int32_t attr_public module_stop(size_t argc, const void *args)
{
    final_printf("Stopping plugin...\n");
    // unhook everything just in case
    DestroyDTAHooks();
    UNHOOK(GameRestart);
    UNHOOK(GetTitle);
    UNHOOK(NewFile);
    UNHOOK(TscePadSetLightBar);
    return 0;
}
