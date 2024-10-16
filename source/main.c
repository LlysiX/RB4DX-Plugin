/*
    main.c - RB4DX
    Initialisation code for the RB4DX plugin.
    Licensed under the GNU Lesser General Public License version 2.1, or later.
*/

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>

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

// dta functions

Symbol(*Symbol_Ctor)(Symbol*, const char*);
void(*DataInitFuncs)();
void(*DataRegisterFunc)(Symbol, DataFunc);
DataNode* (*DataArrayEvaluate)(DataNode*, DataArray*, size_t);
Symbol(*DataNodeForceSym)(DataNode*, DataArray*);

DataNode* DataFileRename(DataNode* ret, DataArray* args) {
    final_printf("renaming file!\n");
    DataNode _firstArg = *(args->mNodes + 1);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    char* firstArg = firstArgsym.sym;
    DataNode _secondArg = *(args->mNodes + 2);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    char* secondArg = secondArgsym.sym;
    //if(firstArg.type == kDataSymbol & secondArg.type == kDataSymbol)
    rename(firstArg, secondArg);
    final_printf("from %s\n", firstArg);
    final_printf("to %s\n", secondArg);
    //final_printf("type: %x\n", firstArg.mType);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxFileRename(DataNode* ret, DataArray* args) {
    final_printf("renaming file!\n");
    // set up first arg
    char __firstArg[2048];
    DataNode _firstArg = *(args->mNodes + 1);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    strcpy(__firstArg, RawfilesFolder);
    strcat(__firstArg, firstArgsym.sym);
    char* firstArg = __firstArg;

    // set up second arg
    char __secondArg[2048];
    DataNode _secondArg = *(args->mNodes + 2);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    strcpy(__secondArg, RawfilesFolder);
    strcat(__secondArg, secondArgsym.sym);
    char* secondArg = __secondArg;

    rename(firstArg, secondArg);
    final_printf("from %s\n", firstArg);
    final_printf("to %s\n", secondArg);
    //final_printf("type: %x\n", firstArg.mType);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxFileExists(DataNode* ret, DataArray* args) {
    char __Arg[2048];
    DataNode _Arg = *(args->mNodes + 1);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcpy(__Arg, RawfilesFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    ret->mType = kDataInt;
    ret->mValue.value = file_exists(Arg);
    return ret;
}

DataNode* DxFileDelete(DataNode* ret, DataArray* args) {
    char __Arg[2048];
    DataNode _Arg = *(args->mNodes + 1);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcpy(__Arg, RawfilesFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    remove(Arg);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxFileMkdir(DataNode* ret, DataArray* args) {
    char __Arg[2048];
    DataNode _Arg = *(args->mNodes + 1);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcpy(__Arg, RawfilesFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    mkdir(Arg, 0777);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxWriteNullFile(DataNode* ret, DataArray* args) {
    char __Arg[2048];
    DataNode _Arg = *(args->mNodes + 1);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcpy(__Arg, RawfilesFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    FILE* fptr = fopen(Arg, "w");
    fclose(fptr);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

HOOK_INIT(DataInitFuncs);

void DataInitFuncs_hook() {
    // New dta functions go here
    Symbol funcsym;
    // Rename/Move file
    Symbol_Ctor(&funcsym, "file_rename");
    DataRegisterFunc(funcsym, DataFileRename);
    Symbol_Ctor(&funcsym, "file_move");
    DataRegisterFunc(funcsym, DataFileRename);
    // DX: file_rename using "/data/GoldHEN/RB4DX/" as root path
    Symbol_Ctor(&funcsym, "dx_file_rename");
    DataRegisterFunc(funcsym, DxFileRename);
    // DX: file_exists using "/data/GoldHEN/RB4DX/" as root path
    Symbol_Ctor(&funcsym, "dx_file_exists");
    DataRegisterFunc(funcsym, DxFileExists);
    // DX: file_delete using "/data/GoldHEN/RB4DX/" as root path
    Symbol_Ctor(&funcsym, "dx_file_delete");
    DataRegisterFunc(funcsym, DxFileDelete);
    // DX: file_mkdir using "/data/GoldHEN/RB4DX/" as root path
    Symbol_Ctor(&funcsym, "dx_file_mkdir");
    DataRegisterFunc(funcsym, DxFileMkdir);
    // DX: write_file using "/data/GoldHEN/RB4DX/" as root path (always creates blank file)
    Symbol_Ctor(&funcsym, "dx_write_null_file");
    DataRegisterFunc(funcsym, DxWriteNullFile);

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

    NewFile = (void*)(procInfo.base_address + 0x00376d40);
    GameRestart = (void*)(procInfo.base_address + 0x00a46710);
    GetTitle = (void*)(procInfo.base_address + 0x00f28d20);
    SetMusicSpeed = (void*)(procInfo.base_address + 0x00a470e0);
    DataInitFuncs = (void*)(procInfo.base_address + 0x00222350);
    DataRegisterFunc = (void*)(procInfo.base_address + 0x002221f0);
    DataArrayEvaluate = (void*)(procInfo.base_address + 0x000c7d30);
    DataNodeForceSym = (void*)(procInfo.base_address + 0x0000e850);
    Symbol_Ctor = (void*)(procInfo.base_address + 0x00256fd0);
    TscePadSetLightBar = (void*)(procInfo.base_address + 0x012450d0);

    // apply all hooks
    HOOK(GameRestart);
    HOOK(GetTitle);
    HOOK(NewFile);
    HOOK(TscePadSetLightBar);
    HOOK(DataInitFuncs);

    return 0;
}

int32_t attr_public module_stop(size_t argc, const void *args)
{
    final_printf("Stopping plugin...\n");
    // unhook everything just in case
    UNHOOK(GameRestart);
    UNHOOK(GetTitle);
    UNHOOK(NewFile);
    UNHOOK(TscePadSetLightBar);
    UNHOOK(DataInitFuncs);
    return 0;
}
