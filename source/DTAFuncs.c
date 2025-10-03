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
int (*DataNodeInt)(DataNode*, DataArray*);
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

typedef union _FloatInt {
    float f;
    unsigned int i;
} FloatInt;

uint32_t swap_endian(uint32_t value) {
    return ((value >> 24) & 0x000000FF) | // Move byte 3 to position 0
        ((value >> 8) & 0x0000FF00) | // Move byte 2 to position 1
        ((value << 8) & 0x00FF0000) | // Move byte 1 to position 2
        ((value << 24) & 0xFF000000); // Move byte 0 to position 3
}

DataNode* DataFloatToInt(DataNode* ret, DataArray* args) {
    DataNode _Arg = *(args->mNodes + 1);
    float Argfloat = DataNodeFloat(&_Arg, args);
    FloatInt convert;
    convert.f = Argfloat;
    ret->mType = kDataInt;
    ret->mValue.value = swap_endian(convert.i);
    return ret;
}

// replace floats, used in HUD files for positioning
void replace_floats(const char* filename, long offset, const float* values, size_t count) {
    FILE* fp = fopen(filename, "rb+");
    fseek(fp, offset, SEEK_SET);
    fwrite(values, sizeof(float), count, fp);
    fclose(fp);
    return;
}

const char* score_file_path = "/data/GoldHEN/RB4DX/ps4/ui/game/hud.scene_ps4";
const char* countdown_file_path = "/data/GoldHEN/RB4DX/ps4/ui/game/break_countdown.entity_ps4";
const char* solo_file_path = "/data/GoldHEN/RB4DX/ps4/ui/game/solo_percentage.entity_ps4";
const char* mtv_file_path = "/data/GoldHEN/RB4DX/ps4/ui/game/song_artist_overlay.entity_ps4";
const char* practice_speed_file_path = "/data/GoldHEN/RB4DX/ps4/dx/track/practice_speed.dta_dta_ps4";

DataNode* DataWriteScoreFile(DataNode* ret, DataArray* args) {
    DataNode _firstArg = *(args->mNodes + 1);
    float firstArg;
    firstArg = DataNodeFloat(&_firstArg, args);

    DataNode _secondArg = *(args->mNodes + 2);
    float secondArg;
    secondArg = DataNodeFloat(&_secondArg, args);

    DataNode _thirdArg = *(args->mNodes + 3);
    float thirdArg;
    thirdArg = DataNodeFloat(&_thirdArg, args);

    // No Mic Warning
    DataNode _fourthArg = *(args->mNodes + 4);
    int fourthArg = DataNodeInt(&_fourthArg, args);

    float pos[3] = { firstArg, secondArg, thirdArg };

    replace_floats(score_file_path, 0x31a3, pos, 3);

    FILE* binaryfile = fopen(score_file_path, "rb+");
    fseek(binaryfile, 0x1ec7a, SEEK_SET);
    if (fourthArg == 0)
        fputc('i', binaryfile);
    else
        fputc('n', binaryfile);
    fclose(binaryfile);

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataWriteCountdownFile(DataNode* ret, DataArray* args) {
    DataNode _firstArg = *(args->mNodes + 1);
    float firstArg = DataNodeFloat(&_firstArg, args);

    DataNode _secondArg = *(args->mNodes + 2);
    float secondArg = DataNodeFloat(&_secondArg, args);
    float secondArg2 = secondArg + 10;

    DataNode _thirdArg = *(args->mNodes + 3);
    float thirdArg = DataNodeFloat(&_thirdArg, args);

    float posa[3] = { firstArg, secondArg2, thirdArg };
    float posb[3] = { firstArg, secondArg, thirdArg };

    replace_floats(countdown_file_path, 0xCBA, posa, 3);
    replace_floats(countdown_file_path, 0xE36, posb, 3);

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataWriteMTVFile(DataNode* ret, DataArray* args) {
    DataNode _firstArg = *(args->mNodes + 1);
    float firstArg = DataNodeFloat(&_firstArg, args);
    float firstArg2 = firstArg + 3.04834747314453;

    DataNode _secondArg = *(args->mNodes + 2);
    float secondArg = DataNodeFloat(&_secondArg, args);

    DataNode _thirdArg = *(args->mNodes + 3);
    float thirdArg = DataNodeFloat(&_thirdArg, args);
    float thirdArg2;

    //detailed
    DataNode _fourthArg = *(args->mNodes + 4);
    int fourthArg = DataNodeInt(&_fourthArg, args);

    //always on
    DataNode _fifthArg = *(args->mNodes + 5);
    int fifthArg = DataNodeInt(&_fifthArg, args);

    if (fourthArg == 0)
        thirdArg2 = thirdArg - 38.2138786;
    else
        thirdArg2 = thirdArg - 65.21386337280316;

    float posa[3] = { firstArg, secondArg, thirdArg };
    float posb[3] = { firstArg2, secondArg, thirdArg2 };

    replace_floats(mtv_file_path, 0x631, posa, 3);
    replace_floats(mtv_file_path, 0x821, posb, 3);
    
    FILE* binaryfile = fopen(mtv_file_path, "rb+");
    fseek(binaryfile, 0xc64, SEEK_SET);
    if (fifthArg == 0)
        fputc('o', binaryfile);
    else
        fputc('n', binaryfile);
    fseek(binaryfile, 0xce6, SEEK_SET);
    if (fifthArg == 0)
        fputc('i', binaryfile);
    else
        fputc('n', binaryfile);
    fclose(binaryfile);

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataWriteSoloFile(DataNode* ret, DataArray* args) {
    DataNode _firstArg = *(args->mNodes + 1);
    float firstArg = DataNodeFloat(&_firstArg, args);

    DataNode _secondArg = *(args->mNodes + 2);
    float secondArg = DataNodeFloat(&_secondArg, args);

    DataNode _thirdArg = *(args->mNodes + 3);
    float thirdArg = DataNodeFloat(&_thirdArg, args);

    float pos[3] = { firstArg, secondArg, thirdArg };

    replace_floats(solo_file_path, 0x130C, pos, 3);

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataSetPracticeSpeed(DataNode* ret, DataArray* args) {
    DataNode _firstArg = *(args->mNodes + 1);
    float firstArg = DataNodeFloat(&_firstArg, args);

    replace_floats(practice_speed_file_path, 0x39, &firstArg, 1);

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataWriteBinaryFile(DataNode* ret, DataArray* args) {
    DataNode _firstArg = *(args->mNodes + 1);
    Symbol firstArgsym = DataNodeForceSym(&_firstArg, args);
    char* firstArg = firstArgsym.sym;
    DataNode _secondArg = *(args->mNodes + 2);
    Symbol secondArgsym = DataNodeForceSym(&_secondArg, args);
    char* secondArg = secondArgsym.sym;

    FILE* binaryfile = fopen(firstArg, "w");

    char* p;
    for(p = secondArg; *p != '\0'; ) {
        unsigned int x;
        int n;
        if(sscanf(p, "%2x%n", &x, &n) != 1) break;
        putc(x, binaryfile);
        p += n;
    }
    fclose(binaryfile);

    ret->mType = kDataInt;
    ret->mValue.value = 1;
    return ret;
}

DataNode* DataReadFileAsFloat(DataNode* ret, DataArray* args) {
    DataNode _Arg = *(args->mNodes + 1);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    char* Arg = Argsym.sym;
    ret->mType = kDataFloat;
    ret->mValue.real = read_file_as_float(Arg);
    return ret;
}

DataNode* DxReadFileAsFloat(DataNode* ret, DataArray* args) {
    char __Arg[2048];
    DataNode _Arg = *(args->mNodes + 1);
    Symbol Argsym = DataNodeForceSym(&_Arg, args);
    strcpy(__Arg, DXFolder);
    strcat(__Arg, Argsym.sym);
    char* Arg = __Arg;
    ret->mType = kDataFloat;
    ret->mValue.real = read_file_as_float(Arg);
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

    // Float to Hex
    Symbol_Ctor(&funcsym, "float_to_int");
    DataRegisterFunc(funcsym, DataFloatToInt);

    // Read File as Float
    Symbol_Ctor(&funcsym, "read_file_as_float");
    DataRegisterFunc(funcsym, DataReadFileAsFloat);
    // DX: Read File as Float using "/data/GoldHEN/RB4DX/" as root path
    Symbol_Ctor(&funcsym, "dx_read_file_as_float");
    DataRegisterFunc(funcsym, DxReadFileAsFloat);

    // Write binary file from string of hex characters
    Symbol_Ctor(&funcsym, "write_binary_file");
    DataRegisterFunc(funcsym, DataWriteBinaryFile);

    // Write score file
    Symbol_Ctor(&funcsym, "write_score_file");
    DataRegisterFunc(funcsym, DataWriteScoreFile);
    // Write countdown file
    Symbol_Ctor(&funcsym, "write_countdown_file");
    DataRegisterFunc(funcsym, DataWriteCountdownFile);
    // Write solo file
    Symbol_Ctor(&funcsym, "write_solo_file");
    DataRegisterFunc(funcsym, DataWriteSoloFile);
    // Write MTV file
    Symbol_Ctor(&funcsym, "write_mtv_file");
    DataRegisterFunc(funcsym, DataWriteMTVFile);
    // Set Practice Speed
    Symbol_Ctor(&funcsym, "set_practice_speed");
    DataRegisterFunc(funcsym, DataSetPracticeSpeed);

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
    uint64_t base_address = get_base_address();

    DataInitFuncs = (void*)(base_address + 0x00222350);
    DataRegisterFunc = (void*)(base_address + 0x002221f0);
    DataArrayEvaluate = (void*)(base_address + 0x000c7d30);
    DataNodeForceSym = (void*)(base_address + 0x0000e850);
    DataNodeFloat = (void*)(base_address + 0x0000ee30);
    DataNodeInt = (void*)(base_address + 0x00000eb40);
    Symbol_Ctor = (void*)(base_address + 0x00256fd0);
    SystemOptionsLoad = (void*)(base_address + 0x011b2310);
    RBSystemOptionsSave = (void*)(base_address + 0x00d667c0);

    HOOK(DataInitFuncs);
    HOOK(SystemOptionsLoad);
    HOOK(RBSystemOptionsSave);
}

void DestroyDTAHooks() {
    UNHOOK(DataInitFuncs);
    UNHOOK(SystemOptionsLoad);
    UNHOOK(RBSystemOptionsSave);
}