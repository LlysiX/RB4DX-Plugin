#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include "plugin_common.h"

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
long(*SortByDateAcquired)(void*, void*);
HOOK_INIT(SortByDateAcquired);
long SortByDateAcquired_hook(void* song1, void* song2) {
    sortingbyartist = false;
    //final_printf("SortByDateAcquired\n");
    return HOOK_CONTINUE(SortByDateAcquired, long (*)(void*, void*), song1, song2);
}

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

void InitSortHooks()
{
    uint64_t base_address = get_base_address();

    SortByArtist = (void*)(base_address + 0x00ca5e30);
    SortByTitle = (void*)(base_address + 0x00ca5d60);
    //SortByDateAcquired = (void*)(base_address + 0x00ca5fd0);
    SortByStars = (void*)(base_address + 0x00ca5f8c); //supposed to be 0x00ca5f80
    //SortByDifficulty = (void*)(base_address + 0x00ca60b0);
    //SortByRating = (void*)(base_address + 0x00ca6100);
    //SortByPlayCount = (void*)(base_address + 0x00ca6120);

    //temporary sort by stars patch to fix the sorting by artist check
    //this is not good but it works so /shrug
    char SortByStarsPatch[] = { 0xe9, 0x1f, 0x01, 0x00, 0x00 };
    sceKernelMprotect(SortByStars, sizeof(SortByStarsPatch), VM_PROT_ALL);
    memcpy(SortByStars, SortByStarsPatch, sizeof(SortByStarsPatch));

    HOOK(SortByArtist);
    HOOK(SortByTitle);
    //HOOK(SortByDateAcquired);
    //HOOK(SortByStars);
    //HOOK(SortByDifficulty);
    //HOOK(SortByRating);
    //HOOK(SortByPlayCount);
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
}