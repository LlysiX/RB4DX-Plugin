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
#include "ini.h"

attr_public const char *g_pluginName = PLUGIN_NAME;
attr_public const char *g_pluginDesc = "Plugin for loading Rock Band 4 Deluxe files, among other enhancements.";
attr_public const char *g_pluginAuth = "LysiX";
attr_public uint32_t g_pluginVersion = 0x00000100; // 1.00

void DoNotificationStatic(const char* text) {
    OrbisNotificationRequest Buffer = { 0 };
    Buffer.useIconImageUri = 1;
    Buffer.targetId = -1;
    strcpy(Buffer.message, text);
    strcpy(Buffer.iconUri, "cxml://psnotification/tex_icon_system");
    sceKernelSendNotificationRequest(0, &Buffer, sizeof(Buffer), 0);
}

void DoNotification(const char *FMT, ...) {
    OrbisNotificationRequest Buffer = { 0 };
    va_list args;
    va_start(args, FMT);
    vsprintf(Buffer.message, FMT, args);
    va_end(args);
    Buffer.type = NotificationRequest;
    Buffer.unk3 = 0;
    Buffer.useIconImageUri = 1;
    Buffer.targetId = -1;
    strcpy(Buffer.iconUri, "cxml://psnotification/tex_icon_system");
    sceKernelSendNotificationRequest(0, &Buffer, sizeof(Buffer), 0);
}

bool file_exists(const char* filename) {
    struct stat buff;
    return stat(filename, &buff) == 0 ? true : false;
}

static struct proc_info procInfo;

// ARKless file loading hook
const char* RawfilesFolder = "/data/GoldHEN/RB4DX/";
const char* GameRawfilesFolder = "data:/GoldHEN/RB4DX/";
bool USTitleID = true;
bool PrintRawfiles = true;
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
typedef struct
{
    float speed;
} SpeedConfiguration;
bool ini_exists;
static int SpeedHandler(void* user, const char* section, const char* name,
                   const char* value)
{
    SpeedConfiguration* pconfig = (SpeedConfiguration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("Settings", "SongSpeedMultiplier")) {
        pconfig->speed = atof(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

void(*GameRestart)(void*, bool);
void(*SetMusicSpeed)(void*, float);

HOOK_INIT(GameRestart);

void GameRestart_hook(void* thisGame, bool restart) {
    HOOK_CONTINUE(GameRestart, void (*)(void*, bool), thisGame, restart);
    SpeedConfiguration SpeedConfig;
    float speed = 1.00;

    //read ini
    if (ini_parse(PLUGIN_CONFIG_PATH, SpeedHandler, &SpeedConfig) < 0) {
        final_printf("Can't load 'RB4DX.ini'\n");
    }

    if (SpeedConfig.speed == 0.00)
        SpeedConfig.speed = 1.00;
    if (ini_exists)
        speed = SpeedConfig.speed;

    if (speed > 0.00 && speed != 1.00){
        SetMusicSpeed(thisGame, speed);
        final_printf("Music speed: %.2f\n", speed);
        DoNotification("Music Speed Set: %.2f", speed);
    }
    return;
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

    ini_exists = file_exists(PLUGIN_CONFIG_PATH);

    final_printf("Applying RB4DX hooks...\n");
    DoNotificationStatic("RB4DX Plugin loaded!");

    //configuration GameConfig; //TODO: implement game configuration

    //DataReadFile = (void*)(procInfo.base_address + 0x002205e0);
    NewFile = (void*)(procInfo.base_address + 0x00376d40);
    GameRestart = (void*)(procInfo.base_address + 0x0a46710);
    SetMusicSpeed = (void*)(procInfo.base_address + 0x00a470e0);
    TscePadSetLightBar = (void*)(procInfo.base_address + 0x012450d0);

    // apply all hooks
    HOOK(GameRestart);
    HOOK(NewFile);
    HOOK(TscePadSetLightBar);
    //HOOK(DataReadFile);

    return 0;
}

int32_t attr_public module_stop(size_t argc, const void *args)
{
    final_printf("Stopping plugin...\n");
    // unhook everything just in case
    UNHOOK(GameRestart);
    UNHOOK(NewFile);
    UNHOOK(TscePadSetLightBar);
    //UNHOOK(DataReadFile);
    return 0;
}
