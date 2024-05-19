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
#include "songmetadata.h"

attr_public const char *g_pluginName = PLUGIN_NAME;
attr_public const char *g_pluginDesc = "Plugin for loading Rock Band 4 Deluxe files, among other enhancements.";
attr_public const char *g_pluginAuth = "LysiX";
attr_public uint32_t g_pluginVersion = 0x00000100; // 1.00

//notification ini reader
typedef struct
{
    bool NotifyColored;
} NotifyConfiguration;
bool ini_exists;
static int NotifyHandler(void* user, const char* section, const char* name,
    const char* value)
{
    NotifyConfiguration* pconfig = (NotifyConfiguration*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("Settings", "NotifyColored")) {
        if (strcmp(value, "true") == 0) {
            pconfig->NotifyColored = true;
        }
        else {
            pconfig->NotifyColored = false;
        }
    }
    else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

bool file_exists(const char* filename) {
    struct stat buff;
    return stat(filename, &buff) == 0 ? true : false;
}

void DoNotificationStatic(const char* text) {
    NotifyConfiguration NotifyConfig;
    bool NotifyColored = false;
    bool NotifyColoredfile = file_exists("/data/GoldHEN/RB4DX/notifycolored.ini");
    //read ini for notify color
    if (ini_parse(PLUGIN_CONFIG_PATH, NotifyHandler, &NotifyConfig) < 0) {
        final_printf("Can't load 'RB4DX.ini'\n");
    }
    if (ini_exists) {
        NotifyColored = NotifyConfig.NotifyColored;
    }
    OrbisNotificationRequest Buffer = { 0 };
    Buffer.useIconImageUri = 1;
    Buffer.targetId = -1;
    strcpy(Buffer.message, text);
    if (NotifyColored || NotifyColoredfile)
        strcpy(Buffer.iconUri, "https://raw.githubusercontent.com/hmxmilohax/rock-band-3-deluxe/100069d1c2293424a659ecb4a5ddacc3b91c4f9b/dependencies/media/dx.png");
    else
        strcpy(Buffer.iconUri, "https://raw.githubusercontent.com/hmxmilohax/RB2DX-Site/7d707e0d8e6f1c47c9e1eb187237ad934f254f92/docs/apple-touch-icon.png");
    sceKernelSendNotificationRequest(0, &Buffer, sizeof(Buffer), 0);
}

void DoNotification(const char *FMT, ...) {
    NotifyConfiguration NotifyConfig;
    bool NotifyColored = false;
    bool NotifyColoredfile = file_exists("/data/GoldHEN/RB4DX/notifycolored.ini");
    //read ini for notify color
    if (ini_parse(PLUGIN_CONFIG_PATH, NotifyHandler, &NotifyConfig) < 0) {
        final_printf("Can't load 'RB4DX.ini'\n");
    }
    if (ini_exists) {
        NotifyColored = NotifyConfig.NotifyColored;
    }
    OrbisNotificationRequest Buffer = { 0 };
    va_list args;
    va_start(args, FMT);
    vsprintf(Buffer.message, FMT, args);
    va_end(args);
    Buffer.type = NotificationRequest;
    Buffer.unk3 = 0;
    Buffer.useIconImageUri = 1;
    Buffer.targetId = -1;
    if (NotifyColored || NotifyColoredfile)
        strcpy(Buffer.iconUri, "https://raw.githubusercontent.com/hmxmilohax/rock-band-3-deluxe/100069d1c2293424a659ecb4a5ddacc3b91c4f9b/dependencies/media/dx.png");
    else
        strcpy(Buffer.iconUri, "https://raw.githubusercontent.com/hmxmilohax/RB2DX-Site/7d707e0d8e6f1c47c9e1eb187237ad934f254f92/docs/apple-touch-icon.png");
    sceKernelSendNotificationRequest(0, &Buffer, sizeof(Buffer), 0);
}

//speedhack ini reader
typedef struct
{
    float speed;
} SpeedConfiguration;
static int SpeedHandler(void* user, const char* section, const char* name,
    const char* value)
{
    SpeedConfiguration* pconfig = (SpeedConfiguration*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("Settings", "SongSpeedMultiplier")) {
        pconfig->speed = atof(value);
    }
    else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

//autoplay ini reader
typedef struct
{
    bool autoplay;
} AutoplayConfiguration;
static int AutoplayHandler(void* user, const char* section, const char* name,
    const char* value)
{
    AutoplayConfiguration* pconfig = (AutoplayConfiguration*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("Settings", "Autoplay")) {
        if (strcmp(value, "true") == 0) {
            pconfig->autoplay = true;
        }
        else {
            pconfig->autoplay = false;
        }
    }
    else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
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
void(*GameRestart)(void*, bool);
void(*SetMusicSpeed)(void*, float);

HOOK_INIT(GameRestart);

void GameRestart_hook(void* thisGame, bool restart) {
    HOOK_CONTINUE(GameRestart, void (*)(void*, bool), thisGame, restart);
    SpeedConfiguration SpeedConfig;
    float speed = 1.00;
    AutoplayConfiguration AutoplayConfig;
    bool autoplay = false;

    bool insong = file_exists("/data/GoldHEN/RB4DX/insong.dta");
    bool autoplayfile = file_exists("/data/GoldHEN/RB4DX/autoplay.ini");
    bool speedfile = file_exists("/data/GoldHEN/RB4DX/speedmod.ini");

    //read ini for autoplay
    if (ini_parse(PLUGIN_CONFIG_PATH, AutoplayHandler, &AutoplayConfig) < 0) {
        final_printf("Can't load 'RB4DX.ini'\n");
    }

    //read ini for speedhack
    if (ini_parse(PLUGIN_CONFIG_PATH, SpeedHandler, &SpeedConfig) < 0) {
        final_printf("Can't load 'RB4DX.ini'\n");
    }

    if (SpeedConfig.speed == 0.00)
        SpeedConfig.speed = 1.00;
    if (ini_exists) {
        speed = SpeedConfig.speed;
        autoplay = AutoplayConfig.autoplay;
    }

    if (autoplayfile)
        autoplay = true;

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
        if (!insong || autoplay)
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

HOOK_INIT(GetTitle);

char* GetTitle_hook(SongMetadata* thisMetadata) {
    AutoplayConfiguration AutoplayConfig;
    SpeedConfiguration SpeedConfig;
    float speed = 1.00;
    bool autoplay = false;
    char aptitleint[256];
    strcpy(aptitleint, thisMetadata->mTitle);
    strcat(aptitleint, autoplaytitle);
    char* aptitle = aptitleint;
    char speedtitleint[256];
    char speedtxt[256];
    char* speedtitle;
    strcpy(speedtitleint, thisMetadata->mTitle);

    bool insong = file_exists("/data/GoldHEN/RB4DX/insong.dta");
    bool autoplayfile = file_exists("/data/GoldHEN/RB4DX/autoplay.ini");
    bool speedfile = file_exists("/data/GoldHEN/RB4DX/speedmod.ini");

    //read ini for autoplay
    if (ini_parse(PLUGIN_CONFIG_PATH, AutoplayHandler, &AutoplayConfig) < 0) {
        //final_printf("Can't load 'RB4DX.ini'\n");
    }
    //read ini for speedhack
    if (ini_parse(PLUGIN_CONFIG_PATH, SpeedHandler, &SpeedConfig) < 0) {
        //final_printf("Can't load 'RB4DX.ini'\n");
    }

    if (SpeedConfig.speed == 0.00)
        SpeedConfig.speed = 1.00;

    if (ini_exists) {
        speed = SpeedConfig.speed * 100;
        autoplay = AutoplayConfig.autoplay;
    }

    if (autoplayfile)
        autoplay = true;

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
    else if (insong && speed > 0 && speed != 100) {
        sprintf(speedtxt, " (%.0f%% Speed)", speed);
        strcat(speedtitleint, speedtxt);
        speedtitle = speedtitleint;
        return speedtitle;
    }
    else
        return thisMetadata->mTitle;
}

//autoplay hack
void(*SetGameOver)(void*, bool);

HOOK_INIT(SetGameOver);

void SetGameOver_hook(void* thisGame,  bool won) {
    AutoplayConfiguration AutoplayConfig;
    bool autoplay = false;
    bool autoplayfile = file_exists("/data/GoldHEN/RB4DX/autoplay.ini");

    //read ini for autoplay
    if (ini_parse(PLUGIN_CONFIG_PATH, AutoplayHandler, &AutoplayConfig) < 0) {
        final_printf("Can't load 'RB4DX.ini'\n");
    }

    if (ini_exists)
        autoplay = AutoplayConfig.autoplay;

    if (autoplayfile)
        autoplay = true;

    if (autoplay)
        //no winning for you, cheater
        HOOK_CONTINUE(SetGameOver, bool (*)(void*, bool), thisGame, false);
    else
        HOOK_CONTINUE(SetGameOver, bool (*)(void*, bool), thisGame, won);
    return;
}

void(*ExportGameEnded)(void*, void*, void*, bool);

HOOK_INIT(ExportGameEnded);

void ExportGameEnded_hook(void* thisRBGameData, void* Game, void* RBSong, bool won) {
    AutoplayConfiguration AutoplayConfig;
    bool autoplay = false;

    bool insong = file_exists("/data/GoldHEN/RB4DX/insong.dta");
    bool autoplayfile = file_exists("/data/GoldHEN/RB4DX/autoplay.ini");

    //read ini for autoplay
    if (ini_parse(PLUGIN_CONFIG_PATH, AutoplayHandler, &AutoplayConfig) < 0) {
        final_printf("Can't load 'RB4DX.ini'\n");
    }

    if (ini_exists)
        autoplay = AutoplayConfig.autoplay;

    if (autoplayfile)
        autoplay = true;

    if (insong && autoplay)
        //show end screen but ignore score
        HOOK_CONTINUE(ExportGameEnded, void (*)(void*, void*, void*, bool), thisRBGameData, Game, RBSong, true);
    else if (autoplay)
        HOOK_CONTINUE(ExportGameEnded, void (*)(void*, void*, void*, bool), thisRBGameData, Game, RBSong, false);
    else
        HOOK_CONTINUE(ExportGameEnded, void (*)(void*, void*, void*, bool), thisRBGameData, Game, RBSong, won);
    
    return;
}

void(*SetCheating)(void*, bool);

HOOK_INIT(SetCheating);

void SetCheating_hook(void* thisTrackWatcher, bool cheating) {
    AutoplayConfiguration AutoplayConfig;
    bool autoplay = false;
    bool autoplayfile = file_exists("/data/GoldHEN/RB4DX/autoplay.ini");

    //read ini for autoplay
        if (ini_parse(PLUGIN_CONFIG_PATH, AutoplayHandler, &AutoplayConfig) < 0) {
            final_printf("Can't load 'RB4DX.ini'\n");
        }

    if (ini_exists)
        autoplay = AutoplayConfig.autoplay;

    if (autoplayfile)
        autoplay = true;

    HOOK_CONTINUE(SetCheating, void (*)(void*, bool), thisTrackWatcher, autoplay);
    return;
}

void(*RBVocalPlayerRestart)(void*, float, void*);
void(*SetAutoplay)(void*, bool);

HOOK_INIT(RBVocalPlayerRestart);

void RBVocalPlayerRestart_hook(void* thisRBVocalPlayer, float time, void* song) {
    AutoplayConfiguration AutoplayConfig;
    bool autoplay = false;
    bool autoplayfile = file_exists("/data/GoldHEN/RB4DX/autoplay.ini");

    //read ini for autoplay
    if (ini_parse(PLUGIN_CONFIG_PATH, AutoplayHandler, &AutoplayConfig) < 0) {
        final_printf("Can't load 'RB4DX.ini'\n");
    }

    if (ini_exists)
        autoplay = AutoplayConfig.autoplay;

    if (autoplayfile)
        autoplay = true;

    SetAutoplay(thisRBVocalPlayer, autoplay);

    HOOK_CONTINUE(RBVocalPlayerRestart, void (*)(void*, float, void*), thisRBVocalPlayer, time, song);
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
    GameRestart = (void*)(procInfo.base_address + 0x00a46710);
    GetTitle = (void*)(procInfo.base_address + 0x00f28d20);
    SetMusicSpeed = (void*)(procInfo.base_address + 0x00a470e0);
    SetGameOver = (void*)(procInfo.base_address + 0x00a48790);
    SetCheating = (void*)(procInfo.base_address + 0x0122dfc0);
    SetAutoplay = (void*)(procInfo.base_address + 0x00a65680);
    RBVocalPlayerRestart = (void*)(procInfo.base_address + 0x00a622f0);
    ExportGameEnded = (void*)(procInfo.base_address + 0x009648d0);
    TscePadSetLightBar = (void*)(procInfo.base_address + 0x012450d0);

    // apply all hooks
    HOOK(GameRestart);
    HOOK(SetGameOver);
    HOOK(SetCheating);
    HOOK(GetTitle);
    HOOK(RBVocalPlayerRestart);
    HOOK(ExportGameEnded);
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
    UNHOOK(SetGameOver);
    UNHOOK(SetCheating);
    UNHOOK(GetTitle);
    UNHOOK(RBVocalPlayerRestart);
    UNHOOK(ExportGameEnded);
    UNHOOK(NewFile);
    UNHOOK(TscePadSetLightBar);
    //UNHOOK(DataReadFile);
    return 0;
}
