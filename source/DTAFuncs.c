/*
    DTAFuncs.c - RB4DX
    code for creating DTA Functions for RB4DX.
    Licensed under the GNU Lesser General Public License version 2.1, or later.
*/

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/stat.h>
#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include "plugin_common.h"
#include "rb4/data.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int cp(const char* to, const char* from)
{
    int fd_to, fd_from;
    char buf[4096] = { 0 };
    ssize_t nread;
    int saved_errno;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char* out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return 0;
    }

out_error:
    return -1;
}

const char* DXFolder = "/data/GoldHEN/RB4DX-1.08/";
Symbol(*Symbol_Ctor)(Symbol*, const char*);
void(*DataInitFuncs)();
void(*DataRegisterFunc)(Symbol, DataFunc);
Symbol(*DataNodeForceSym)(DataNode*, DataArray*);
float (*DataNodeFloat)(DataNode*, DataArray*);
int (*DataNodeInt)(DataNode*, DataArray*);
DataNode* (*DataExecuteString)(DataNode* __return_storage_ptr__, char* param_1);
SystemOptions* options;

DataNode* DataFileRename(DataNode* ret, DataArray* args) {
    final_printf("renaming file!\n");
    DataNode _firstArg = (args->mNodes->n[1]);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    char* firstArg = firstArgsym.sym;
    DataNode _secondArg = (args->mNodes->n[2]);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    char* secondArg = secondArgsym.sym;
    sceKernelRename(firstArg, secondArg);
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
    char __firstArg[2048] = { 0 };
    DataNode _firstArg = (args->mNodes->n[1]);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    strcat(__firstArg, DXFolder);
    strcat(__firstArg, firstArgsym.sym);
    char* firstArg = __firstArg;

    // set up second arg
    char __secondArg[2048] = { 0 };
    DataNode _secondArg = (args->mNodes->n[2]);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    strcat(__secondArg, DXFolder);
    strcat(__secondArg, secondArgsym.sym);
    char* secondArg = __secondArg;

    sceKernelRename(firstArg, secondArg);
    final_printf("from %s\n", firstArg);
    final_printf("to %s\n", secondArg);
    //final_printf("type: %x\n", firstArg.mType);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxFileExists(DataNode* ret, DataArray* args) {
    char __Arg[2048] = { 0 };
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcat(__Arg, DXFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    ret->mType = kDataInt;
    ret->mValue.value = file_exists(Arg);
    return ret;
}

DataNode* DxFileDelete(DataNode* ret, DataArray* args) {
    char __Arg[2048] = { 0 };
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcat(__Arg, DXFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    sceKernelUnlink(Arg);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxFileMkdir(DataNode* ret, DataArray* args) {
    char __Arg[2048] = { 0 };
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcat(__Arg, DXFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    sceKernelMkdir(Arg, 0777);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* FileRmdir(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    char* Arg = Argsym.sym;
    sceKernelRmdir(Arg);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxFileRmdir(DataNode* ret, DataArray* args) {
    char __Arg[2048] = { 0 };
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcat(__Arg, DXFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    sceKernelRmdir(Arg);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxWriteNullFile(DataNode* ret, DataArray* args) {
    char __Arg[2048] = { 0 };
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcat(__Arg, DXFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    FILE* fptr = fopen(Arg, "w");
    fclose(fptr);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataFileCopy(DataNode* ret, DataArray* args) {
    final_printf("Copying file!\n");
    DataNode _firstArg = (args->mNodes->n[1]);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    char* firstArg = firstArgsym.sym;
    DataNode _secondArg = (args->mNodes->n[2]);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    char* secondArg = secondArgsym.sym;
    if (file_exists(firstArg)) {
        cp(secondArg, firstArg);
        final_printf("from %s\n", firstArg);
        final_printf("to %s\n", secondArg);
    }
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxFileCopy(DataNode* ret, DataArray* args) {
    final_printf("Copying file!\n");
    // set up first arg
    char __firstArg[2048] = { 0 };
    DataNode _firstArg = (args->mNodes->n[1]);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    strcat(__firstArg, DXFolder);
    strcat(__firstArg, firstArgsym.sym);
    for (int i = 0, j; __firstArg[i] != '\0'; ++i) { // fix sprinted files with spaces (TODO: repeat this in the other funcs)

        // enter the loop if the character is '
        // and not the null character
        while ((__firstArg[i] == '\'') && !(__firstArg[i] == '\0')) {
            for (j = i; __firstArg[j] != '\0'; ++j) {

                // if jth element of __firstArg is ',
                // assign the value of (j+1)th element to the jth element
                __firstArg[j] = __firstArg[j + 1];
            }
            __firstArg[j] = '\0';
        }
    }
    char* firstArg = __firstArg;

    // set up second arg
    char __secondArg[2048] = { 0 };
    DataNode _secondArg = (args->mNodes->n[2]);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    strcat(__secondArg, DXFolder);
    strcat(__secondArg, secondArgsym.sym);
    char* secondArg = __secondArg;
    if (file_exists(firstArg)) {
        cp(secondArg, firstArg);
        final_printf("from %s\n", firstArg);
        final_printf("to %s\n", secondArg);
    }
    else {
        final_printf("error finding %s\n", firstArg);
    }
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

// get calibration offset in dta in ms

DataNode* GetAudioCalibration(DataNode* ret, DataArray* args) {
    ret->mType = kDataInt;
    ret->mValue.value = options->mAudioOffset;
    return ret;
}

DataNode* GetVideoCalibration(DataNode* ret, DataArray* args) {
    ret->mType = kDataInt;
    ret->mValue.value = options->mVideoOffset;
    return ret;
}

// set calibration offset in dta in ms 

DataNode* SetAudioCalibration(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    float Argfloat = DataNodeFloat(&_Arg, args);
    if (options != NULL) {
        options->mAudioOffset = Argfloat;
    }
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* SetVideoCalibration(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    float Argfloat = DataNodeFloat(&_Arg, args);
    if (options != NULL) {
        options->mVideoOffset = Argfloat;
    }
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* SetSongSpeed(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    float Argfloat = DataNodeFloat(&_Arg, args);
    songspeed = Argfloat;
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

#ifndef FILESYSTEM_PREFIX_LEN
#define FILESYSTEM_PREFIX_LEN(f) 0
#endif

#ifndef ISSLASH
#define ISSLASH(c) ((c) == '/')
#endif

char*
base_name(char const* name)
{
    char const* base = name += FILESYSTEM_PREFIX_LEN(name);

    for (; *name; name++)
        if (ISSLASH(*name))
            base = name + 1;

    return (char*)base;
}

DataNode* DataBaseNameExt(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    char* Arg = Argsym.sym;
    char* filebase = base_name(Arg);
    ret->mType = kDataSymbol;
    ret->mValue.symbol = filebase;
    return ret;
}

DataNode* EmuFileDelete(DataNode* ret, DataArray* args) {
    char buf[2048] = { 0 };

    DataNode _firstArg = (args->mNodes->n[1]);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    char* firstArg = firstArgsym.sym;

    strcat(buf, "{set $path \"");
    strcat(buf, firstArg);
    strcat(buf, "\"}");
    DataExecuteString(ret, buf);
    buf[0] = '\0';

    memcpy(firstArg, "data:/", 6);
    strcat(buf, "{set $path_ingame \"");
    strcat(buf, firstArg);
    strcat(buf, "\"}");
    DataExecuteString(ret, buf);
    buf[0] = '\0';

    DataNode _secondArg = (args->mNodes->n[2]);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    char* secondArg = secondArgsym.sym;
    strcat(buf, "{set $todel \"");
    strcat(buf, secondArg);
    strcat(buf, "\"}");
    DataExecuteString(ret, buf);

    char fullpath[2048] = { 0 };
    memcpy(firstArg, "/data/", 6);
    strcat(fullpath, firstArg);
    strcat(fullpath, "/");
    strcat(fullpath, secondArg);

    if (!file_exists(fullpath)) {
        DataExecuteString(ret, "{set $path 0}");
        DataExecuteString(ret, "{set $path_ingame 0}");
        DataExecuteString(ret, "{set $todel 0}");
        ret->mType = kDataInt;
        ret->mValue.value = 1;
        return ret;
    }

    DataExecuteString(ret, "{dx_file_mkdir 'temp'}");
    DataExecuteString(ret, "{set $copylist {file_list_paths {sprint $path_ingame '/*'}}}");
    DataExecuteString(ret, "{foreach $entry $copylist {unless {== {basename_ext $entry} $todel} {file_copy {sprint $path \"/\" {basename_ext $entry}} {sprint \"/data/GoldHEN/RB4DX-1.08/temp/\" {basename_ext $entry}}}}}");
    DataExecuteString(ret, "{file_rmdir $path}");
    DataExecuteString(ret, "{file_mkdir $path_ingame}");
    DataExecuteString(ret, "{file_copy \"/data/GoldHEN/RB4DX-1.08/temp\" $path}");
    DataExecuteString(ret, "{dx_file_rmdir \"temp\"}");

    DataExecuteString(ret, "{set $path 0}");
    DataExecuteString(ret, "{set $path_ingame 0}");
    DataExecuteString(ret, "{set $todel 0}");
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxEmuFileDelete(DataNode* ret, DataArray* args) {
    char buf[2048] = { 0 };

    DataNode _firstArg = (args->mNodes->n[1]);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    char* firstArg = firstArgsym.sym;
    strcat(buf, "{set $dx_path \"");
    strcat(buf, firstArg);
    strcat(buf, "\"}");
    DataExecuteString(ret, buf);
    buf[0] = '\0';

    DataNode _secondArg = (args->mNodes->n[2]);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    char* secondArg = secondArgsym.sym;
    strcat(buf, "{set $dx_file \"");
    strcat(buf, secondArg);
    strcat(buf, "\"}");
    DataExecuteString(ret, buf);

    DataExecuteString(ret, "{emu_file_delete {sprint \"/data/GoldHEN/RB4DX-1.08/\" $dx_path} $dx_file}");

    DataExecuteString(ret, "{set $dx_path 0}");
    DataExecuteString(ret, "{set $dx_file 0}");

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DxForceSym(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Arg = DataNodeForceSym(&_Arg, args);
    ret->mType = kDataSymbol;
    ret->mValue.symbol = Arg.sym;
    return ret;
}

DataNode* DataIsEmulator(DataNode* ret, DataArray* args) {
    ret->mType = kDataInt;
    if (sys_sdk_proc_info(&procInfo) != 0) {
        ret->mValue.value = 1;
    }
    else {
        ret->mValue.value = 0;
    }
    return ret;
}

DataNode* DataSetPluginVar(DataNode* ret, DataArray* args) {
    DataNode _Arg1 = (args->mNodes->n[1]);
    DataNode _Arg2 = (args->mNodes->n[2]);
    Symbol Argsym = DataNodeForceSym(&_Arg1, args);
    char* Arg1 = Argsym.sym;
    int Arg2 = DataNodeInt(&_Arg2, args);
    set_plugin_var(Arg1, Arg2);
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataGetPluginVar(DataNode* ret, DataArray* args) {
    DataNode _Arg = (args->mNodes->n[1]);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    char* Arg = Argsym.sym;
    ret->mType = kDataInt;
    ret->mValue.value = get_plugin_var(Arg);
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
    // Copy file
    Symbol_Ctor(&funcsym, "file_copy");
    DataRegisterFunc(funcsym, DataFileCopy);
    // DX: file_copy using "/data/GoldHEN/RB4DX/" as root path
    Symbol_Ctor(&funcsym, "dx_file_copy");
    DataRegisterFunc(funcsym, DxFileCopy);
    // DX: file_rename using "/data/GoldHEN/RB4DX/" as root path
    Symbol_Ctor(&funcsym, "dx_file_rename");
    DataRegisterFunc(funcsym, DxFileRename);
    // DX: file_exists using "/data/GoldHEN/RB4DX/" as root path
    Symbol_Ctor(&funcsym, "dx_file_exists");
    DataRegisterFunc(funcsym, DxFileExists);

    if (sys_sdk_proc_info(&procInfo) != 0) {
        // syscall failed, probably shadPS4
        // janky file_delete method needed for emu
        Symbol_Ctor(&funcsym, "emu_file_delete");
        DataRegisterFunc(funcsym, EmuFileDelete);
        // DX: emu_file_delete using "/data/GoldHEN/RB4DX/" as root path
        Symbol_Ctor(&funcsym, "dx_emu_file_delete");
        DataRegisterFunc(funcsym, DxEmuFileDelete);
    }
    
    // DX: file_delete using "/data/GoldHEN/RB4DX/" as root path
    Symbol_Ctor(&funcsym, "dx_file_delete");
    DataRegisterFunc(funcsym, DxFileDelete);
    // DX: file_mkdir using "/data/GoldHEN/RB4DX/" as root path
    Symbol_Ctor(&funcsym, "dx_file_mkdir");
    DataRegisterFunc(funcsym, DxFileMkdir);
    // Delete Directory
    Symbol_Ctor(&funcsym, "file_rmdir");
    DataRegisterFunc(funcsym, FileRmdir);
    // DX: file_rmdir using "/data/GoldHEN/RB4DX/" as root path
    Symbol_Ctor(&funcsym, "dx_file_rmdir");
    DataRegisterFunc(funcsym, DxFileRmdir);
    // DX: write_file using "/data/GoldHEN/RB4DX/" as root path (always creates blank file)
    Symbol_Ctor(&funcsym, "dx_write_null_file");
    DataRegisterFunc(funcsym, DxWriteNullFile);
    // DX: base name with file extentions
    Symbol_Ctor(&funcsym, "basename_ext");
    DataRegisterFunc(funcsym, DataBaseNameExt);
    // Allow DataForceSym to be called from DTA
    Symbol_Ctor(&funcsym, "force_sym");
    DataRegisterFunc(funcsym, DxForceSym);
    // Emulation check
    Symbol_Ctor(&funcsym, "is_emu");
    DataRegisterFunc(funcsym, DataIsEmulator);

    // get calibration offset in dta in ms
    Symbol_Ctor(&funcsym, "get_audio_calibration");
    DataRegisterFunc(funcsym, GetAudioCalibration);

    Symbol_Ctor(&funcsym, "get_video_calibration");
    DataRegisterFunc(funcsym, GetVideoCalibration);

    // set calibration offset in dta in ms 
    Symbol_Ctor(&funcsym, "set_audio_calibration");
    DataRegisterFunc(funcsym, SetAudioCalibration);

    Symbol_Ctor(&funcsym, "set_video_calibration");
    DataRegisterFunc(funcsym, SetVideoCalibration);

    //updated song speed functionality
    Symbol_Ctor(&funcsym, "set_song_speed");
    DataRegisterFunc(funcsym, SetSongSpeed);

    //Plugin variables, accessible in C and DTA
    Symbol_Ctor(&funcsym, "set_plugin_var");
    DataRegisterFunc(funcsym, DataSetPluginVar);

    Symbol_Ctor(&funcsym, "get_plugin_var");
    DataRegisterFunc(funcsym, DataGetPluginVar);

    //add original dta functions
    HOOK_CONTINUE(DataInitFuncs, void (*)());
}

void (*SystemOptionsLoad)(SystemOptions*, void*);

HOOK_INIT(SystemOptionsLoad);

void SystemOptionsLoad_hook(SystemOptions* thisoptions, void* binstream) {
    //DoNotificationStatica("systemoptions::load");
    options = thisoptions;
    HOOK_CONTINUE(SystemOptionsLoad, void (*)(SystemOptions*, void*), thisoptions, binstream);
    //force override calibration if not saved in the save file
    //DoNotificationa("Audio Offset Set: %.0f", thisoptions->mAudioOffset); // 260
    //DoNotificationa("Video Offset Set: %.0f", thisoptions->mVideoOffset); // 260
    return;
}

void (*RBSystemOptionsSave)(void*, void*);

HOOK_INIT(RBSystemOptionsSave);

void RBSystemOptionsSave_hook(void* thisoptions, void* binstream) {
    DataNode ret;
    //DoNotificationStatica("systemoptions::save");
    //options = thisoptions;
    if (file_exists("/data/GoldHEN/RB4DX-1.08/plugin/calibration.ini")) {
        DataExecuteString(&ret, "{set $calibration {read_file 'data:/GoldHEN/RB4DX-1.08/plugin/calibration.ini'}}");
        DataExecuteString(&ret, "{foreach $entry $calibration {if {== {elem $entry 0} {basename video}} {set_video_calibration {elem {find $entry video} 1}} } {if {== {elem $entry 0} {basename audio}} {set_audio_calibration {elem {find $entry audio} 1}} } }");
    }

    HOOK_CONTINUE(RBSystemOptionsSave, void (*)(void*, void*), thisoptions, binstream);
    //DoNotificationa("Audio Offset Set: %.0f", thisoptions->mAudioOffset);
    
    // delete force override files when this is called
    if (file_exists("/data/GoldHEN/RB4DX-1.08/plugin/calibration.ini")) {
        sceKernelUnlink("/data/GoldHEN/RB4DX-1.08/plugin/calibration.ini");
        DataExecuteString(&ret, "{dx_emu_file_delete 'plugin' 'calibration.ini'}");
    }
    return;
}

void InitDTAHooks() {
    uint64_t base_address = get_base_address();

    DataInitFuncs = (void*)(base_address + 0x006d2720);
    DataRegisterFunc = (void*)(base_address + 0x006d25d0);
    DataNodeForceSym = (void*)(base_address + 0x006ed810);
    DataNodeFloat = (void*)(base_address + 0x00bef6a0);
    DataNodeInt = (void*)(base_address + 0x00be8c50);
    Symbol_Ctor = (void*)(base_address + 0x00708d40);
    SystemOptionsLoad = (void*)(base_address + 0x00801e50);
    RBSystemOptionsSave = (void*)(base_address + 0x0046a760);
    DataExecuteString = (void*)(base_address + 0x006cd2c0);

    HOOK(DataInitFuncs);
    HOOK(SystemOptionsLoad);
    HOOK(RBSystemOptionsSave);
}

void DestroyDTAHooks() {
    UNHOOK(DataInitFuncs);
    UNHOOK(SystemOptionsLoad);
    UNHOOK(RBSystemOptionsSave);
}