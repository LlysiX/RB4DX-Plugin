/*
    DTAFuncs.c - RB4DX
    code for creating DTA Functions for RB4DX.
    Licensed under the GNU Lesser General Public License version 2.1, or later.
*/

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include "plugin_common.h"
#include "rb4/data.h"

const char* DXFolder = "/data/GoldHEN/RB4DX/";
Symbol(*Symbol_Ctor)(Symbol*, const char*);
void(*DataInitFuncs)();
void(*DataRegisterFunc)(Symbol, DataFunc);
DataNode* (*DataArrayEvaluate)(DataNode*, DataArray*, size_t);
Symbol(*DataNodeForceSym)(DataNode*, DataArray*);
float (*DataNodeFloat)(DataNode*, DataArray*);
SystemOptions* options;

DataNode* DataFileRename(DataNode* ret, DataArray* args) {
    final_printf("renaming file!\n");
    DataNode _firstArg = *(args->mNodes + 1);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    char* firstArg = firstArgsym.sym;
    DataNode _secondArg = *(args->mNodes + 2);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    char* secondArg = secondArgsym.sym;
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
    strcpy(__firstArg, DXFolder);
    strcat(__firstArg, firstArgsym.sym);
    char* firstArg = __firstArg;

    // set up second arg
    char __secondArg[2048];
    DataNode _secondArg = *(args->mNodes + 2);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    strcpy(__secondArg, DXFolder);
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
    strcpy(__Arg, DXFolder);
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
    strcpy(__Arg, DXFolder);
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
    strcpy(__Arg, DXFolder);
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
    strcpy(__Arg, DXFolder);
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
    DataNode _firstArg = *(args->mNodes + 1);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    char* firstArg = firstArgsym.sym;
    DataNode _secondArg = *(args->mNodes + 2);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    char* secondArg = secondArgsym.sym;
    if (file_exists(firstArg)) {
        size_t bytesRead;
        char buffer[4096];
        FILE* iptr = fopen(firstArg, "r");
        FILE* optr = fopen(secondArg, "w");
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), iptr)) > 0) {
            fwrite(buffer, 1, bytesRead, optr);
        }
        fclose(iptr);
        fclose(optr);
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
    char __firstArg[2048];
    DataNode _firstArg = *(args->mNodes + 1);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    strcpy(__firstArg, DXFolder);
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
    char __secondArg[2048];
    DataNode _secondArg = *(args->mNodes + 2);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    strcpy(__secondArg, DXFolder);
    strcat(__secondArg, secondArgsym.sym);
    char* secondArg = __secondArg;
    if (file_exists(firstArg)) {
        size_t bytesRead;
        char buffer[4096];
        FILE* iptr = fopen(firstArg, "r");
        FILE* optr = fopen(secondArg, "w");
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), iptr)) > 0) {
            fwrite(buffer, 1, bytesRead, optr);
        }
        fclose(iptr);
        fclose(optr);
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
    DataNode _Arg = *(args->mNodes + 1);
    float Argfloat = DataNodeFloat(&_Arg, args);
    if (options != NULL) {
        options->mAudioOffset = Argfloat;
    }
    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* SetVideoCalibration(DataNode* ret, DataArray* args) {
    DataNode _Arg = *(args->mNodes + 1);
    float Argfloat = DataNodeFloat(&_Arg, args);
    if (options != NULL) {
        options->mVideoOffset = Argfloat;
    }
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
    // DX: file_delete using "/data/GoldHEN/RB4DX/" as root path
    Symbol_Ctor(&funcsym, "dx_file_delete");
    DataRegisterFunc(funcsym, DxFileDelete);
    // DX: file_mkdir using "/data/GoldHEN/RB4DX/" as root path
    Symbol_Ctor(&funcsym, "dx_file_mkdir");
    DataRegisterFunc(funcsym, DxFileMkdir);
    // DX: write_file using "/data/GoldHEN/RB4DX/" as root path (always creates blank file)
    Symbol_Ctor(&funcsym, "dx_write_null_file");
    DataRegisterFunc(funcsym, DxWriteNullFile);

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
    if (file_exists("/data/GoldHEN/RB4DX/videocalib.ini")) {
        options->mVideoOffset = read_file_as_float("/data/GoldHEN/RB4DX/videocalib.ini");
    }
    if (file_exists("/data/GoldHEN/RB4DX/audiocalib.ini")) {
        options->mAudioOffset = read_file_as_float("/data/GoldHEN/RB4DX/audiocalib.ini");
    }
    //DoNotificationa("Audio Offset Set: %.0f", thisoptions->mAudioOffset); // 260
    //DoNotificationa("Video Offset Set: %.0f", thisoptions->mVideoOffset); // 260
    return;
}

void (*RBSystemOptionsSave)(void*, void*);

HOOK_INIT(RBSystemOptionsSave);

void RBSystemOptionsSave_hook(void* thisoptions, void* binstream) {
    //DoNotificationStatica("systemoptions::save");
    //options = thisoptions;
    HOOK_CONTINUE(RBSystemOptionsSave, void (*)(void*, void*), thisoptions, binstream);
    //DoNotificationa("Audio Offset Set: %.0f", thisoptions->mAudioOffset);
    // delete force override files when this is called
    if (file_exists("/data/GoldHEN/RB4DX/videocalib.ini")) {
        remove("/data/GoldHEN/RB4DX/videocalib.ini");
    }
    if (file_exists("/data/GoldHEN/RB4DX/audiocalib.ini")) {
        remove("/data/GoldHEN/RB4DX/audiocalib.ini");
    }
    return;
}

void InitDTAHooks() {
    sys_sdk_proc_info(&procInfo);

    DataInitFuncs = (void*)(procInfo.base_address + 0x00222350);
    DataRegisterFunc = (void*)(procInfo.base_address + 0x002221f0);
    DataArrayEvaluate = (void*)(procInfo.base_address + 0x000c7d30);
    DataNodeForceSym = (void*)(procInfo.base_address + 0x0000e850);
    DataNodeFloat = (void*)(procInfo.base_address + 0x0000ee30);
    Symbol_Ctor = (void*)(procInfo.base_address + 0x00256fd0);
    SystemOptionsLoad = (void*)(procInfo.base_address + 0x011b2310);
    RBSystemOptionsSave = (void*)(procInfo.base_address + 0x00d667c0);

    HOOK(DataInitFuncs);
    HOOK(SystemOptionsLoad);
    HOOK(RBSystemOptionsSave);
}

void DestroyDTAHooks() {
    UNHOOK(DataInitFuncs);
    UNHOOK(SystemOptionsLoad);
    UNHOOK(RBSystemOptionsSave);
}