#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include "plugin_common.h"
#include "rb4/songmetadata.h"
#include "rb4/data.h"

bool sortingbyartist = false;

int(*SortByArtist)(void*, void*);
HOOK_INIT(SortByArtist);
int SortByArtist_hook(void* song1, void* song2) {
    sortingbyartist = true;
    //final_printf("SortByArtist\n");
    return HOOK_CONTINUE(SortByArtist, int (*)(void*, void*), song1, song2);
}

int(*SortByTitle)(void*, void*);
HOOK_INIT(SortByTitle);
int SortByTitle_hook(void* song1, void* song2) {
    sortingbyartist = false;
    //final_printf("SortByTitle\n");
    return HOOK_CONTINUE(SortByTitle, int (*)(void*, void*), song1, song2);
}

//for some reason this never gets hit, while the rest of the sort functions that arent title/artist crash
int(*SortByStars)(void*, void*);
HOOK_INIT(SortByStars);
int SortByStars_hook(void* song1, void* song2) {
    sortingbyartist = false;
    //final_printf("SortByStars\n");
    return HOOK_CONTINUE(SortByStars, int (*)(void*, void*), song1, song2);
}
/*
SongMetadata*(*GetSongMetadata)(void*, void*, int);
void(*TheSongMgr);
int(*CompareStrings)(char*, char*, bool);
void(*SongMgrLoadSongCache)(void*, void*);
HOOK_INIT(SongMgrLoadSongCache);
void SongMgrLoadSongCache_hook(void* SongMgr, void* param_2) {
    if (TheSongMgr == NULL)
        TheSongMgr = SongMgr;
    final_printf("loadsongcache, %X\n", SongMgr);
    HOOK_CONTINUE(SongMgrLoadSongCache, void (*)(void*, void*), SongMgr, param_2);
    return;
}
HOOK_INIT(GetSongMetadata);
SongMetadata* GetSongMetadata_hook(void* param1, void* param2, int param3) {
    //final_printf("GetSongMetadata, %X\n", param1);
    return HOOK_CONTINUE(GetSongMetadata, SongMetadata* (*)(void*, void*, int), param1, param2, param3);
}
*/

/*
long(*SortByDateAcquired)(void*, void*);
void(*sortbysourcepatchloc1);
void(*sortbysourcepatchloc2);
HOOK_INIT(SortByDateAcquired);
long SortByDateAcquired_hook(void* song1, void* song2) {
    char sortbysourcepatch1[] = { 0xe8, 0x95, 0x2e, 0x28, 0x00, 0x48, 0x89, 0xdf, 0x49, 0x89, 0xc4, 0xe8, 0x8a, 0x2e, 0x28, 0x00 };
    sceKernelMprotect(sortbysourcepatchloc1, sizeof(sortbysourcepatch1), VM_PROT_ALL);
    memcpy(sortbysourcepatchloc1, sortbysourcepatch1, sizeof(sortbysourcepatch1));
    /*char sortbysourcepatch2[] = { 0x66, 0xe9, 0xac, 0xfe };
    sceKernelMprotect(sortbysourcepatchloc2, sizeof(sortbysourcepatch2), VM_PROT_ALL);
    memcpy(sortbysourcepatchloc2, sortbysourcepatch2, sizeof(sortbysourcepatch2));* /
    int ret = HOOK_CONTINUE(SortByArtist, int (*)(void*, void*), song1, song2);
    char sortbysourceunpatch1[] = { 0xe8, 0xb5, 0x2e, 0x28, 0x00, 0x48, 0x89, 0xdf, 0x49, 0x89, 0xc4, 0xe8, 0xaa, 0x2e, 0x28, 0x00 };
    sceKernelMprotect(sortbysourcepatchloc1, sizeof(sortbysourceunpatch1), VM_PROT_ALL);
    memcpy(sortbysourcepatchloc1, sortbysourceunpatch1, sizeof(sortbysourceunpatch1));
    /*char sortbysourceunpatch2[] = { 0x49, 0x8b, 0x7d, 0x00 };
    sceKernelMprotect(sortbysourcepatchloc2, sizeof(sortbysourceunpatch2), VM_PROT_ALL);
    memcpy(sortbysourcepatchloc2, sortbysourceunpatch2, sizeof(sortbysourceunpatch2));* /
    //final_printf("SortByDateAcquired\n");
    sortingbyartist = false;
    return ret;
}
*/


/*
long(*SortByDifficulty)(void*, void*);
HOOK_INIT(SortByDifficulty);
long SortByDifficulty_hook(void* song1, void* song2) {
    sortingbyartist = false;
    //final_printf("SortByDifficulty\n");
    return HOOK_CONTINUE(SortByDifficulty, long (*)(void*, void*), song1, song2);
}

long(*SortByRating)(void*, void*);
HOOK_INIT(SortByRating);
long SortByRating_hook(void* song1, void* song2) {
    sortingbyartist = false;
    //final_printf("SortByRating\n");
    return HOOK_CONTINUE(SortByRating, long (*)(void*, void*), song1, song2);
}

long(*SortByPlayCount)(void*, void*);
HOOK_INIT(SortByPlayCount);
long SortByPlayCount_hook(void* song1, void* song2) {
    sortingbyartist = false;
    //final_printf("SortByPlayCount\n");
    return HOOK_CONTINUE(SortByPlayCount, long (*)(void*, void*), song1, song2);
}
*/

/*
bool filteringbysource = false;
Symbol(*GetDecade)(SongMetadata*);
HOOK_INIT(GetDecade);
Symbol GetDecade_hook(SongMetadata* thisMetaData) {
    Symbol ret;
    if (filteringbysource)
        Symbol_Ctor(&ret, thisMetaData->mGameOrigin);
    else {
        char decade[10] = { 0 };
        sprintf(decade, "the%is", (thisMetaData->mOriginalYear / 10) * 10);
        Symbol_Ctor(&ret, decade);
    }
    return ret;
}
void(*FilterByDecade)(void*, void*);
void(*FilterBySourcePatchLoc1);
void(*FilterBySourcePatchLoc2);
void(*FilterBySourcePatchLoc3);
void(*FilterBySourcePatchLoc4);
void(*FilterBySourcePatchLoc5);
void(*FilterBySourcePatchLoc6);
void(*FilterBySourcePatchLoc7);
void(*FilterBySourcePatchLoc8);
void(*FilterBySourcePatchLoc9);
HOOK_INIT(FilterByDecade);
void FilterByDecade_hook(void* param1, void* param2) {
    if (filteringbysource) {
        char FilterBySourcePatch1[] = { 0x49, 0x8b, 0x5f, 0x60, 0x4d, 0x8b, 0x77, 0x68 };
        sceKernelMprotect(FilterBySourcePatchLoc1, sizeof(FilterBySourcePatch1), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc1, FilterBySourcePatch1, sizeof(FilterBySourcePatch1));
        char FilterBySourcePatch2[] = { 0x49, 0x8b, 0x5f, 0x68, 0x49, 0x3b, 0x5f, 0x70 };
        sceKernelMprotect(FilterBySourcePatchLoc2, sizeof(FilterBySourcePatch2), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc2, FilterBySourcePatch2, sizeof(FilterBySourcePatch2));
        char FilterBySourcePatch3[] = { 0x49, 0x89, 0x47, 0x68 };
        sceKernelMprotect(FilterBySourcePatchLoc3, sizeof(FilterBySourcePatch3), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc3, FilterBySourcePatch3, sizeof(FilterBySourcePatch3));
        char FilterBySourcePatch4[] = { 0x49, 0x8b, 0x77, 0x60 };
        sceKernelMprotect(FilterBySourcePatchLoc4, sizeof(FilterBySourcePatch4), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc4, FilterBySourcePatch4, sizeof(FilterBySourcePatch4));
        char FilterBySourcePatch5[] = { 0x49, 0x8d, 0x7f, 0x78 };
        sceKernelMprotect(FilterBySourcePatchLoc5, sizeof(FilterBySourcePatch5), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc5, FilterBySourcePatch5, sizeof(FilterBySourcePatch5));
        char FilterBySourcePatch6[] = { 0x49, 0x8b, 0x77, 0x60, 0x49, 0x8b, 0x5f, 0x68 };
        sceKernelMprotect(FilterBySourcePatchLoc6, sizeof(FilterBySourcePatch6), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc6, FilterBySourcePatch6, sizeof(FilterBySourcePatch6));
        char FilterBySourcePatch7[] = { 0x49, 0x8b, 0x77, 0x60 };
        sceKernelMprotect(FilterBySourcePatchLoc7, sizeof(FilterBySourcePatch7), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc7, FilterBySourcePatch7, sizeof(FilterBySourcePatch7));
        char FilterBySourcePatch8[] = { 0x49, 0x8b, 0x57, 0x70, 0x49, 0x8d, 0x7f, 0x78 };
        sceKernelMprotect(FilterBySourcePatchLoc8, sizeof(FilterBySourcePatch8), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc8, FilterBySourcePatch8, sizeof(FilterBySourcePatch8));
        char FilterBySourcePatch9[] = { 0x4d, 0x89, 0x67, 0x60, 0x49, 0x89, 0x5f, 0x68, 0x49, 0x89, 0x47, 0x70 };
        sceKernelMprotect(FilterBySourcePatchLoc9, sizeof(FilterBySourcePatch9), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc9, FilterBySourcePatch9, sizeof(FilterBySourcePatch9));
    }
    else {
        char FilterBySourceUnpatch1[] = { 0x49, 0x8b, 0x5f, 0x20, 0x4d, 0x8b, 0x77, 0x28 };
        sceKernelMprotect(FilterBySourcePatchLoc1, sizeof(FilterBySourceUnpatch1), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc1, FilterBySourceUnpatch1, sizeof(FilterBySourceUnpatch1));
        char FilterBySourceUnpatch2[] = { 0x49, 0x8b, 0x5f, 0x28, 0x49, 0x3b, 0x5f, 0x30 };
        sceKernelMprotect(FilterBySourcePatchLoc2, sizeof(FilterBySourceUnpatch2), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc2, FilterBySourceUnpatch2, sizeof(FilterBySourceUnpatch2));
        char FilterBySourceUnpatch3[] = { 0x49, 0x89, 0x47, 0x28 };
        sceKernelMprotect(FilterBySourcePatchLoc3, sizeof(FilterBySourceUnpatch3), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc3, FilterBySourceUnpatch3, sizeof(FilterBySourceUnpatch3));
        char FilterBySourceUnpatch4[] = { 0x49, 0x8b, 0x77, 0x20 };
        sceKernelMprotect(FilterBySourcePatchLoc4, sizeof(FilterBySourceUnpatch4), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc4, FilterBySourceUnpatch4, sizeof(FilterBySourceUnpatch4));
        char FilterBySourceUnpatch5[] = { 0x49, 0x8d, 0x7f, 0x38 };
        sceKernelMprotect(FilterBySourcePatchLoc5, sizeof(FilterBySourceUnpatch5), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc5, FilterBySourceUnpatch5, sizeof(FilterBySourceUnpatch5));
        char FilterBySourceUnpatch6[] = { 0x49, 0x8b, 0x77, 0x20, 0x49, 0x8b, 0x5f, 0x28 };
        sceKernelMprotect(FilterBySourcePatchLoc6, sizeof(FilterBySourceUnpatch6), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc6, FilterBySourceUnpatch6, sizeof(FilterBySourceUnpatch6));
        char FilterBySourceUnpatch7[] = { 0x49, 0x8b, 0x77, 0x20 };
        sceKernelMprotect(FilterBySourcePatchLoc7, sizeof(FilterBySourceUnpatch7), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc7, FilterBySourceUnpatch7, sizeof(FilterBySourceUnpatch7));
        char FilterBySourceUnpatch8[] = { 0x49, 0x8b, 0x57, 0x30, 0x49, 0x8d, 0x7f, 0x38 };
        sceKernelMprotect(FilterBySourcePatchLoc8, sizeof(FilterBySourceUnpatch8), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc8, FilterBySourceUnpatch8, sizeof(FilterBySourceUnpatch8));
        char FilterBySourceUnpatch9[] = { 0x4d, 0x89, 0x67, 0x20, 0x49, 0x89, 0x5f, 0x28, 0x49, 0x89, 0x47, 0x30 };
        sceKernelMprotect(FilterBySourcePatchLoc9, sizeof(FilterBySourceUnpatch9), VM_PROT_ALL);
        memcpy(FilterBySourcePatchLoc9, FilterBySourceUnpatch9, sizeof(FilterBySourceUnpatch9));
    }
    HOOK_CONTINUE(FilterByDecade, void (*)(void*, void*), param1, param2);
    return;
}
void(*FilterBySource)(void*, void*);
HOOK_INIT(FilterBySource);
void FilterBySource_hook(void* param1, void* param2) {
    filteringbysource = true;
    FilterByDecade(param1, param2);
    filteringbysource = false;
    return;
}

void(*HeaderTextPatchLoc);
Symbol(*GetHeaderText)(int, void*);
HOOK_INIT(GetHeaderText);
Symbol GetHeaderText_hook(int sortingmethod, void* param_2) {
    //final_printf("ght");
    Symbol ret;
    if (sortingmethod == 2) {
        Symbol_Ctor(&ret, "SORTING BY CUSTOM SOURCE IS POSSIBLE");
        return ret;
    }
    return HOOK_CONTINUE(GetHeaderText, Symbol (*)(int, void*), sortingmethod, param_2);
}
*/

void InitSortHooks()
{
    uint64_t base_address = get_base_address();

    SortByArtist = (void*)(base_address + 0x00ca5e30);
    SortByTitle = (void*)(base_address + 0x00ca5d60);
    SortByDateAcquired = (void*)(base_address + 0x00ca5fd0);
    //sortbysourcepatchloc1 = (void*)(base_address + 0x00ca5e76);
    //sortbysourcepatchloc2 = (void*)(base_address + 0x00ca5eb0);
    SortByStars = (void*)(base_address + 0x00ca5f8c); //supposed to be 0x00ca5f80
    //SortByDifficulty = (void*)(base_address + 0x00ca60b0);
    //SortByRating = (void*)(base_address + 0x00ca6100);
    //SortByPlayCount = (void*)(base_address + 0x00ca6120);
    //GetSongMetadata = (void*)(base_address + 0x00f2e650);
    //GetSource = (void*)(base_address + 0x00f28d10);
    //CompareStrings = (void*)(base_address + 0x00ca5950);
    //SongMgrLoadSongCache = (void*)(base_address + 0x00f2d1e0);
    //GetDecade = (void*)(base_address + 0x00f29190);
    //FilterByDecade = (void*)(base_address + 0x00c7e120);
    //FilterBySourcePatchLoc1 = (void*)(base_address + 0x00c7e167);
    //FilterBySourcePatchLoc2 = (void*)(base_address + 0x00c7e278);
    //FilterBySourcePatchLoc3 = (void*)(base_address + 0x00c7e286);
    //FilterBySourcePatchLoc4 = (void*)(base_address + 0x00c7e292);
    //FilterBySourcePatchLoc5 = (void*)(base_address + 0x00c7e2b2);
    //FilterBySourcePatchLoc6 = (void*)(base_address + 0x00c7e2c5);
    //FilterBySourcePatchLoc7 = (void*)(base_address + 0x00c7e2f2);
    //FilterBySourcePatchLoc8 = (void*)(base_address + 0x00c7e2fb);
    //FilterBySourcePatchLoc9 = (void*)(base_address + 0x00c7e30f);
    //FilterBySource = (void*)(base_address + 0x00c7e690);
    //HeaderTextPatchLoc = (void*)(base_address + 0x00ca6ff6);
    //GetHeaderText = (void*)(base_address + 0x00ca6fa0);

    //temporary sort by stars patch to fix the sorting by artist check
    //this is not good but it works so /shrug
    char SortByStarsPatch[] = { 0xe9, 0x1f, 0x01, 0x00, 0x00 };
    sceKernelMprotect(SortByStars, sizeof(SortByStarsPatch), VM_PROT_ALL);
    memcpy(SortByStars, SortByStarsPatch, sizeof(SortByStarsPatch));

    HOOK(SortByArtist);
    HOOK(SortByTitle);
    //HOOK(SortByDateAcquired);
    //HOOK(SongMgrLoadSongCache);
    //HOOK(GetSongMetadata);
    //HOOK(SortByStars);
    //HOOK(SortByDifficulty);
    //HOOK(SortByRating);
    //HOOK(SortByPlayCount);
    //HOOK(GetDecade);
    //HOOK(FilterByDecade);
    //HOOK(FilterBySource);
    //HOOK(GetHeaderText);
}

void DestroySortHooks()
{
    UNHOOK(SortByArtist);
    UNHOOK(SortByTitle);
    //UNHOOK(SortByDateAcquired);
    //UNHOOK(SortByStars);
    //UNHOOK(SortByDifficulty);
    //UNHOOK(SortByRating);
    //UNHOOK(SortByPlayCount);
    //UNHOOK(GetDecade);
    //UNHOOK(FilterByDecade);
    //UNHOOK(FilterBySource);
    //UNHOOK(GetHeaderText);
}