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
#include "rb4/data.h"
#include "DTAFuncs.h"
//#include "Autoplay.h"
#include "rb4/songmetadata.h"
//#include "rb4/gemsmasher.h"

attr_public const char *g_pluginName = PLUGIN_NAME;
attr_public const char *g_pluginDesc = "Plugin for loading Rock Band 4 Deluxe files, among other enhancements.";
attr_public const char *g_pluginAuth = "LysiX";
attr_public uint32_t g_pluginVersion = 0x00000100; // 1.00

void DoNotificationStatic(const char* text) {
    bool NotifyColored = file_exists("/data/GoldHEN/RB4DX-1.08/plugin/notifycolored.ini");
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
    bool NotifyColored = file_exists("/data/GoldHEN/RB4DX-1.08/plugin/notifycolored.ini");
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

void reverse(char str[], int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void intToStr(int num, char str[]) {
    int i = 0;
    int isNegative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0) {
        isNegative = 1;
        num = -num;
    }

    while (num != 0) {
        str[i++] = (num % 10) + '0';
        num = num / 10;
    }

    if (isNegative)
        str[i++] = '-';

    str[i] = '\0';
    reverse(str, i);
}

bool refreshrp = false;
bool USTitleID = true;

// ARKless file loading hook
const char* RawfilesFolder = "/data/GoldHEN/RB4DX-1.08/";
const char* GameRawfilesFolder = "data:/GoldHEN/RB4DX-1.08/";
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
    char rawpath[2048] = {0};
    strcat(rawpath, RawfilesFolder);
    strcat(rawpath, path);
    char gamepath[2048] = {0};
    strcat(gamepath, GameRawfilesFolder);
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
void(*RBMetaStateGoto)(void*, int);

HOOK_INIT(GameRestart);

void GameRestart_hook(void* thisGame, bool restart) {
    HOOK_CONTINUE(GameRestart, void (*)(void*, bool), thisGame, restart);
    //bool autoplay = file_exists("/data/GoldHEN/RB4DX-1.08/plugin/autoplay.ini");
    //bool drunkmode = file_exists("/data/GoldHEN/RB4DX-1.08/plugin/drunkmode.ini");
    set_plugin_var("insong", 1);
    refreshrp = true;
    int speedmod = get_plugin_var("speedmod");
    float songspeed = (float)speedmod / 100;

    if (songspeed > 0.00 && songspeed != 1.00){
        SetMusicSpeed(thisGame, songspeed);
        final_printf("Music speed: %.2f\n", songspeed);
    }
    return;
}

HOOK_INIT(RBMetaStateGoto);

void RBMetaStateGoto_hook(void* thisMetaState, int state) {
    HOOK_CONTINUE(RBMetaStateGoto, void (*)(void*, int), thisMetaState, state);
    DataNode ret;
    if (state != 3 && state != 44 && get_plugin_var("insong") != 0)
        set_plugin_var("insong", 0);
    if (state != 3 && state != 44)
        DataExecuteString(&ret, "{write_file 'data:/GoldHEN/RB4DX-1.08/plugin/discordrp.json' {array (\"{\\qGame mode\\q:\\qdefaults\\q}\")}}");
    return;
}

//song title hook for reporting speedhack/autoplay
char* (*GetTitle)(SongMetadata*);
const char* autoplaytitle = " (AUTOPLAY)";
const char* drunkmodetitle = " (DRUNK MODE)";

HOOK_INIT(GetTitle);

char* GetTitle_hook(SongMetadata* thisMetadata) {
    bool insong = (get_plugin_var("insong") != 0);
    if (!insong)
        return  thisMetadata->mTitle;

    int speed = get_plugin_var("speedmod");
    char speedtitleint[512] = {0};
    char speedtxt[512] = { 0 };
    char* speedtitle;
    strcat(speedtitleint, thisMetadata->mTitle);

    //bool autoplay = file_exists("/data/GoldHEN/RB4DX-1.08/plugin/autoplay.ini");
    char aptitleint[512] = { 0 };
    strcat(aptitleint, thisMetadata->mTitle);
    strcat(aptitleint, autoplaytitle);
    char* aptitle = aptitleint;

    //bool drunkmode = file_exists("/data/GoldHEN/RB4DX-1.08/plugin/drunkmode.ini");
    char dmtitleint[512] = { 0 };
    strcat(dmtitleint, thisMetadata->mTitle);
    strcat(dmtitleint, drunkmodetitle);
    char* dmtitle = dmtitleint;

    //final_printf("songtitle: %s\n", thisMetadata->mTitle);
    //final_printf("apsongtitle: %s\n", aptitle);

    //if (insong && autoplay)
        //include " (AUTOPLAY)" at the end of the song title
    //    return aptitle;
    //else if (insong && drunkmode)
        //include " (DRUNK MODE)" at the end of the song title
    //    return dmtitle;
    if (insong && speed > 0 && speed != 100) {
        // include " (x% Speed)" at the end of the song title
        // manually convert speed % to string, since shad doesn't support sprintf
        char temp[20];
        int temp_num = speed;
        int digit_count = 0;
        int i;
        // Count digits
        if (temp_num == 0) {
            digit_count = 1;
        }
        else {
            while (temp_num > 0) {
                temp_num /= 10;
                digit_count++;
            }
        }
        // Convert to string manually
        temp[digit_count] = '\0';
        for (i = digit_count - 1; i >= 0; i--) {
            temp[i] = '0' + (speed % 10);
            speed /= 10;
        }

        strcat(speedtxt, " (");
        strcat(speedtxt, temp);
        strcat(speedtxt, "% Speed)");
        //sprintf(speedtxt, " (%.0f%% Speed)", songspeed);
        strcat(speedtitleint, speedtxt);
        speedtitle = speedtitleint;
        return speedtitle;
    }
    else
        return thisMetadata->mTitle;
}

//artist hook for detailed song stats
//void* thelocale;
char* (*GetArtist)(SongMetadata*);
//char* (*Localize)(Symbol, bool*, void*);
const char* famousby = "As Made Famous By ";

HOOK_INIT(GetArtist);

//convert double quotes to single quotes to prevent crashing when creating the discordrp.json file

void fix_quotes(const char* input, char* output) {
    while (*input) {
        if (*input == '"') {
            *output++ = '\'';
        }
        else {
            *output++ = *input;
        }
        input++;
    }
    *output = '\0';
}

char* GetArtist_hook(SongMetadata* thisMetadata) {
    DataNode ret;
    bool insong = (get_plugin_var("insong") != 0);
    if (!insong)
        return  thisMetadata->mArtist;
    bool showartist = (get_plugin_var("noartisttxt") == 0);
    bool showcover = (get_plugin_var("covertxt") != 0);
    bool showalbum = (get_plugin_var("albumtxt") != 0);
    bool showyear = (get_plugin_var("yeartxt") != 0);
    bool showgenre = (get_plugin_var("genretxt") != 0);
    bool showorigin = (get_plugin_var("origintxt") != 0);
    bool fake = false;

    char year[4];
    intToStr(thisMetadata->mAlbumYear, year);

    char rpexec[4096] = { 0 };
    strcat(rpexec, "{write_file 'data:/GoldHEN/RB4DX-1.08/plugin/discordrp.json' {array (");

    char richprescence[4096] = { 0 };
    strcat(richprescence, "\"{\\qGame mode\\q:\\qqp_coop\\q,\\qLoaded Song\\q:\\q");

    char fixedtitle[512] = { 0 };
    char fixedartist[512] = { 0 };
    char fixedalbum[512] = { 0 };

    if (refreshrp) {
        refreshrp = false;

        fix_quotes(thisMetadata->mTitle, fixedtitle);
        strcat(richprescence, fixedtitle);
        strcat(richprescence, " - ");
        if (thisMetadata->mIsCoverRecording)
            strcat(richprescence, famousby);
        fix_quotes(thisMetadata->mArtist, fixedartist);
        strcat(richprescence, fixedartist);
        strcat(richprescence, ", ");
        strcat(richprescence, year);
        strcat(richprescence, "\\q,\\qSongname\\q:\\q");
        strcat(richprescence, fixedtitle);
        strcat(richprescence, "\\q,\\qArtist\\q:\\q");
        if (thisMetadata->mIsCoverRecording)
            strcat(richprescence, famousby);
        strcat(richprescence, fixedartist);
        strcat(richprescence, "\\q,\\qYear\\q:\\q");
        strcat(richprescence, year);
        strcat(richprescence, "\\q,\\qAlbum\\q:\\q");
        fix_quotes(thisMetadata->mAlbum, fixedartist);
        strcat(richprescence, fixedalbum);
        strcat(richprescence, "\\q,\\qGenre\\q:\\q");
        strcat(richprescence, thisMetadata->mGenre);
        strcat(richprescence, "\\q,\\qSource\\q:\\q");
        strcat(richprescence, thisMetadata->mGameOrigin);
        strcat(richprescence, "\\q");
        strcat(rpexec, richprescence);
        strcat(rpexec, "}\")}}");

        DataExecuteString(&ret, rpexec);
    }

    char detailedint[1024] = { 0 };
    //famous by/cover
    if (showartist && showcover && thisMetadata->mIsCoverRecording)
        strcat(detailedint, famousby);

    //artist
    if (showartist)
        strcat(detailedint, thisMetadata->mArtist);
    
    //album
    if (showalbum && strcmp(thisMetadata->mAlbum, "") != 0) {
        strcat(detailedint, "\n");
        strcat(detailedint, thisMetadata->mAlbum);
    }

    if (showyear) {
        strcat(detailedint, ", ");
        strcat(detailedint, year);
    }

    //genre
    if (showgenre) {
        strcat(detailedint, "\n");
        strcat(detailedint, thisMetadata->mGenre);
    }

    if (showorigin) {
        if (!showgenre || !showalbum || strcmp(thisMetadata->mAlbum, "") == 0)
            strcat(detailedint, "\n");
        else
            strcat(detailedint, " | ");
        strcat(detailedint, thisMetadata->mGameOrigin);
    }
    char* detailed = detailedint;
    return detailed;
}

// Custom gem colors from RBVREnhanced, updated to set gems individually instead of all at once
// TODO: FIND SUSTAIN COLOR

/* bool(*UpdateColors)(RBGemSmasherCom* thiscom);

HOOK_INIT(UpdateColors);

bool UpdateColors_hook(RBGemSmasherCom* thiscom) {

    bool enabled = file_exists("/data/GoldHEN/RB4DX/settings/theme/gem/colors/enabled.dta");
    bool r = HOOK_CONTINUE(UpdateColors, bool(*)(RBGemSmasherCom*), thiscom);
    if (!enabled)
        return r;

    //final_printf("Smasher color detected!\n");
    //final_printf("R: %f\n", thiscom->mColor.r);
    //final_printf("G: %f\n", thiscom->mColor.g);
    //final_printf("B: %f\n", thiscom->mColor.b);

    char coltest[9];
    sprintf(coltest, "%f", thiscom->mColor.r);

    //green
    if (strcmp(coltest, "0.130136") == 0) {
        thiscom->mColor.r = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/greenr.dta");
        thiscom->mColor.g = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/greeng.dta");
        thiscom->mColor.b = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/greenb.dta");
        thiscom->mColor.a = 1.0;
    }
    //red
    if (strcmp(coltest, "0.896269") == 0) {
        thiscom->mColor.r = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/redr.dta");
        thiscom->mColor.g = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/redg.dta");
        thiscom->mColor.b = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/redb.dta");
        thiscom->mColor.a = 1.0;
    }
    //yellow
    if (strcmp(coltest, "0.791298") == 0) {
        thiscom->mColor.r = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/yellowr.dta");
        thiscom->mColor.g = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/yellowg.dta");
        thiscom->mColor.b = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/yellowb.dta");
        thiscom->mColor.a = 1.0;
    }
    //blue
    if (strcmp(coltest, "0.011612") == 0) {
        thiscom->mColor.r = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/bluer.dta");
        thiscom->mColor.g = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/blueg.dta");
        thiscom->mColor.b = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/blueb.dta");
        thiscom->mColor.a = 1.0;
    }
    //orange
    if (strcmp(coltest, "0.745404") == 0) {
        thiscom->mColor.r = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/oranger.dta");
        thiscom->mColor.g = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/orangeg.dta");
        thiscom->mColor.b = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/orangeb.dta");
        thiscom->mColor.a = 1.0;
    }

    return r;
}

bool (*DoSetColor)(void* component, void* proppath, void* propinfo, Color* color, Color* toset, bool param_6);

HOOK_INIT(DoSetColor);

bool DoSetColor_hook(void* component, void* proppath, void* propinfo, Color* color, Color* toset, bool param_6) {
    bool insong = file_exists("/data/GoldHEN/RB4DX/insong.dta");
    bool enabled = file_exists("/data/GoldHEN/RB4DX/settings/theme/gem/colors/enabled.dta");
    if (!enabled || !insong)
        return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, toset, param_6);
    
    //final_printf("Color detected\n");
    //final_printf("R: %f\n", toset->r);
    //final_printf("G: %f\n", toset->g);
    //final_printf("B: %f\n", toset->b);

    Color newcolorg;
    Color newcolorr;
    Color newcolory;
    Color newcolorb;
    Color newcoloro;

    char coltest[9] = { 0 };
    sprintf(coltest, "%f", toset->r);
    //none
    if ((strcmp(coltest, "0.130136") != 0) && (strcmp(coltest, "0.242281") != 0) && (strcmp(coltest, "0.896269") != 0) && (strcmp(coltest, "0.791298") != 0) && (strcmp(coltest, "0.011612") != 0) && (strcmp(coltest, "0.004025") != 0) && (strcmp(coltest, "0.745404") != 0) && (strcmp(coltest, "0.814847") != 0))
        return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, toset, param_6);
    //green
    if ((strcmp(coltest, "0.130136") == 0) || (strcmp(coltest, "0.242281") == 0)) {
        newcolorg.r = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/greenr.dta");
        newcolorg.g = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/greeng.dta");
        newcolorg.b = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/greenb.dta");
        newcolorg.a = 1.0;
        return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, &newcolorg, param_6);
    }
    //red
    if (strcmp(coltest, "0.896269") == 0) {
        newcolorr.r = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/redr.dta");
        newcolorr.g = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/redg.dta");
        newcolorr.b = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/redb.dta");
        newcolorr.a = 1.0;
        return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, &newcolorr, param_6);
    }
    //yellow
    if (strcmp(coltest, "0.791298") == 0) {
        newcolory.r = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/yellowr.dta");
        newcolory.g = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/yellowg.dta");
        newcolory.b = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/yellowb.dta");
        newcolory.a = 1.0;
        return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, &newcolory, param_6);
    }
    //blue
    if ((strcmp(coltest, "0.011612") == 0) || (strcmp(coltest, "0.004025") == 0)) {
        newcolorb.r = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/bluer.dta");
        newcolorb.g = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/blueg.dta");
        newcolorb.b = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/blueb.dta");
        newcolorb.a = 1.0;
        return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, &newcolorb, param_6);
    }
    //orange
    if ((strcmp(coltest, "0.745404") == 0) || (strcmp(coltest, "0.814847") == 0)) {
        newcoloro.r = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/oranger.dta");
        newcoloro.g = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/orangeg.dta");
        newcoloro.b = read_file_as_float("/data/GoldHEN/RB4DX/settings/theme/gem/colors/orangeb.dta");
        newcoloro.a = 1.0;
        return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, &newcoloro, param_6);
    }
    return HOOK_CONTINUE(DoSetColor, bool(*)(void*, void*, void*, Color*, Color*, bool), component, proppath, propinfo, color, toset, param_6);
}*/

int32_t attr_public module_start(size_t argc, const void *args)
{
    if (sys_sdk_proc_info(&procInfo) != 0) {
        final_printf("shadPS4? assuming we're 01.08\n");
        // TODO: figure out version check and USTitleID check for shadPS4
    } else {
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
        
        if (strcmp(procInfo.version, "01.08") != 0) {
            final_printf("This plugin is only compatible with version 01.08 of Rock Band 4.\n");
            return 0;
        }
    }

    uint64_t base_address = get_base_address();

    final_printf("Applying RB4DX hooks...\n");
    DoNotificationStatic("RB4DX Plugin loaded!");

    NewFile = (void*)(base_address + 0x007acef0);
    GameRestart = (void*)(base_address + 0x0008b930);
    GetTitle = (void*)(base_address + 0x004035e0);
    GetArtist = (void*)(base_address + 0x004035f0);
    //Localize = (void*)(base_address + 0x007b9d30);
    //thelocale = (void*)(base_address + 0x01c80390);
    SetMusicSpeed = (void*)(base_address + 0x0008c3c0);
    RBMetaStateGoto = (void*)(base_address + 0x003163f0);
    //UpdateColors = (void*)(base_address + 0x00f94a70);
    //DoSetColor = (void*)(base_address + 0x001a7320);

    // apply all hooks
    InitDTAHooks();
    //InitAutoplayHooks();
    HOOK(GameRestart);
    HOOK(RBMetaStateGoto);
    HOOK(GetTitle);
    HOOK(GetArtist);
    HOOK(NewFile);
    //HOOK(UpdateColors);
    //HOOK(DoSetColor);

    return 0;
}

int32_t attr_public module_stop(size_t argc, const void *args)
{
    final_printf("Stopping plugin...\n");
    // unhook everything just in case
    DestroyDTAHooks();
    //DestroyAutoplayHooks();
    UNHOOK(GameRestart);
    UNHOOK(RBMetaStateGoto);
    UNHOOK(GetTitle);
    UNHOOK(GetArtist);
    UNHOOK(NewFile);
    //UNHOOK(UpdateColors);
    //UNHOOK(DoSetColor);
    return 0;
}
