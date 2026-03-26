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

int(*SortByDateAcquired)(void*, void*);
HOOK_INIT(SortByDateAcquired);
int SortByDateAcquired_hook(void* song1, void* song2) {
    sortingbyartist = false;
    //final_printf("SortByDateAcquired\n");
    return HOOK_CONTINUE(SortByDateAcquired, int (*)(void*, void*), song1, song2);
}

int(*SortByStars)(void*, void*);
HOOK_INIT(SortByStars);
int SortByStars_hook(void* song1, void* song2) {
    sortingbyartist = false;
    //final_printf("SortByStars\n");
    return HOOK_CONTINUE(SortByStars, int (*)(void*, void*), song1, song2);
}

int(*SortByDifficulty)(void*, void*);
HOOK_INIT(SortByDifficulty);
int SortByDifficulty_hook(void* song1, void* song2) {
    sortingbyartist = false;
    //final_printf("SortByDifficulty\n");
    return HOOK_CONTINUE(SortByDifficulty, int (*)(void*, void*), song1, song2);
}

int(*SortByRating)(void*, void*);
HOOK_INIT(SortByRating);
int SortByRating_hook(void* song1, void* song2) {
    sortingbyartist = false;
    //final_printf("SortByRating\n");
    return HOOK_CONTINUE(SortByRating, int (*)(void*, void*), song1, song2);
}

int(*SortByPlayCount)(void*, void*);
HOOK_INIT(SortByPlayCount);
int SortByPlayCount_hook(void* song1, void* song2) {
    sortingbyartist = false;
    //final_printf("SortByPlayCount\n");
    return HOOK_CONTINUE(SortByPlayCount, int (*)(void*, void*), song1, song2);
}

void InitSortHooks()
{
    uint64_t base_address = get_base_address();

    SortByArtist = (void*)(base_address + 0x00ca5e30);
    SortByTitle = (void*)(base_address + 0x00ca5d60);
    SortByDateAcquired = (void*)(base_address + 0x00ca5fd0);
    SortByStars = (void*)(base_address + 0x00ca5f80);
    SortByDifficulty = (void*)(base_address + 0x00ca60b0);
    SortByRating = (void*)(base_address + 0x00ca6100);
    SortByPlayCount = (void*)(base_address + 0x00ca6120);

    HOOK(SortByArtist);
    HOOK(SortByTitle);
    HOOK(SortByDateAcquired);
    HOOK(SortByStars);
    HOOK(SortByDifficulty);
    HOOK(SortByRating);
    HOOK(SortByPlayCount);
}

void DestroySortHooks()
{
    UNHOOK(SortByArtist);
    UNHOOK(SortByTitle);
    UNHOOK(SortByDateAcquired);
    UNHOOK(SortByStars);
    UNHOOK(SortByDifficulty);
    UNHOOK(SortByRating);
    UNHOOK(SortByPlayCount);
}