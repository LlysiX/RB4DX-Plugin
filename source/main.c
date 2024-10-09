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
#define PLUGIN_CONFIG_PATH RB4DX_PATH "/RB4DX.ini"
#define PLUGIN_NAME "RB4DX"
#define final_printf(a, args...) klog("[" PLUGIN_NAME "] " a, ##args)

#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include <orbis/Pad.h>
#include "rb4/songmetadata.h"
#include "rb4/data.h"

attr_public const char *g_pluginName = PLUGIN_NAME;
attr_public const char *g_pluginDesc = "Plugin for loading Rock Band 4 Deluxe files, among other enhancements.";
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

// ARKless file loading hook
const char* RawfilesFolder = "/data/GoldHEN/RB4DX/";
const char* GameRawfilesFolder = "data:/GoldHEN/RB4DX/";
bool USTitleID = true;
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
    char rawpath[256];
    strcpy(rawpath, RawfilesFolder);
    /*if (rawpath[strlen(rawpath) - 1] != '/') {
        strcat(rawpath, "/");
    }*/
    strcat(rawpath, path);
    char gamepath[256];
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
    char speedtitleint[256];
    char speedtxt[256];
    char* speedtitle;
    strcpy(speedtitleint, thisMetadata->mTitle);

    bool autoplay = file_exists("/data/GoldHEN/RB4DX/autoplay.ini");
    char aptitleint[256];
    strcpy(aptitleint, thisMetadata->mTitle);
    strcat(aptitleint, autoplaytitle);
    char* aptitle = aptitleint;

    bool drunkmode = file_exists("/data/GoldHEN/RB4DX/drunkmode.ini");
    char dmtitleint[256];
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

//TODDO: force the readfile function to look for text format instead of binary

//void(*datareadfile)(long, const char*, bool);
//
//hook_init(datareadfile);
//
//void datareadfile_hook(long l, const char* filename, bool b){
//    final_printf("attempting to read dta %s\n", filename);
//    if (strcmp(filename, "dx/ui/dx_init.dta") == 0) {
//        final_printf("dx.dta read detected!\n");
//        final_printf("long = %ld\n", l);
//        final_printf("bool = %d\n", b);
//        hook_continue(datareadfile, void(*)(long, const char*, bool), l, filename, 0);
//        //if (b == 1)
//        //    hook_continue(datareadfile, void(*)(const char*, bool), filename, 0);
//        //else
//        //    hook_continue(datareadfile, void(*)(const char*, bool), filename, 1);
//    } else
//        hook_continue(datareadfile, void(*)(long, const char*, bool), l, filename, b);
//    return;
//}

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

// dta functions

Symbol(*Symbol_Ctor)(Symbol*, const char*);
void(*DataInitFuncs)();
void(*DataRegisterFunc)(Symbol, DataFunc);
DataNode* (*DataArrayEvaluate)(DataNode*, DataArray*, int);

DataNode DataFileRename(DataArray* args) {
    final_printf("renaming file!\n");
    DataNode firstArg;
    DataArrayEvaluate(&firstArg, args, 1);
    //DataNode secondArg;
    //DataArrayEvaluate(&secondArg, args, 2);
    //if(firstArg.type == kDataSymbol & secondArg.type == kDataSymbol)
    //rename(firstArg.value.string, secondArg.value.string);
    final_printf("from %s\n", firstArg.mValue.symbol);
    //final_printf("to %s\n", secondArg.mValue.symbol);
    final_printf("type: %u\n", firstArg.mType);
    final_printf("int: %zu\n", firstArg.mValue.value);
    DataNode ret;
    ret.mType = kDataInt;
    ret.mValue.value = 1;
    return ret;
}

HOOK_INIT(DataInitFuncs);

void DataInitFuncs_hook() {
    // New dta functions go here
    Symbol funcsym;
    // Rename/Move file
    Symbol_Ctor(&funcsym, "file_rename");
    DataRegisterFunc(funcsym, DataFileRename);

    //add original dta functions
    HOOK_CONTINUE(DataInitFuncs, void (*)());
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

    //configuration GameConfig; //TODO: implement game configuration

    //DataReadFile = (void*)(procInfo.base_address + 0x002205e0);
    NewFile = (void*)(procInfo.base_address + 0x00376d40);
    GameRestart = (void*)(procInfo.base_address + 0x00a46710);
    GetTitle = (void*)(procInfo.base_address + 0x00f28d20);
    SetMusicSpeed = (void*)(procInfo.base_address + 0x00a470e0);
    DataInitFuncs = (void*)(procInfo.base_address + 0x00222350);
    DataRegisterFunc = (void*)(procInfo.base_address + 0x002221f0);
    DataArrayEvaluate = (void*)(procInfo.base_address + 0x000c7d30);
    Symbol_Ctor = (void*)(procInfo.base_address + 0x00256fd0);
    //SetGameOver = (void*)(procInfo.base_address + 0x00a48790);
    //SetCheating = (void*)(procInfo.base_address + 0x0122dfc0);
    //SetAutoplay = (void*)(procInfo.base_address + 0x00a65680);
    //RBVocalPlayerRestart = (void*)(procInfo.base_address + 0x00a622f0);
    //ExportGameEnded = (void*)(procInfo.base_address + 0x009648d0);
    TscePadSetLightBar = (void*)(procInfo.base_address + 0x012450d0);

    // apply all hooks
    HOOK(GameRestart);
    //HOOK(SetGameOver);
    //HOOK(SetCheating);
    HOOK(GetTitle);
    //HOOK(RBVocalPlayerRestart);
    //HOOK(ExportGameEnded);
    HOOK(NewFile);
    HOOK(TscePadSetLightBar);
    HOOK(DataInitFuncs);
    //HOOK(DataReadFile);

    return 0;
}

int32_t attr_public module_stop(size_t argc, const void *args)
{
    final_printf("Stopping plugin...\n");
    // unhook everything just in case
    UNHOOK(GameRestart);
    //UNHOOK(SetGameOver);
    //UNHOOK(SetCheating);
    UNHOOK(GetTitle);
    //UNHOOK(RBVocalPlayerRestart);
    //UNHOOK(ExportGameEnded);
    UNHOOK(NewFile);
    UNHOOK(TscePadSetLightBar);
    UNHOOK(DataInitFuncs);
    //UNHOOK(DataReadFile);
    return 0;
}
