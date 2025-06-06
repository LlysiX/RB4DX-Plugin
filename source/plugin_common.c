#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <GoldHEN/Common.h>
#include "plugin_common.h"

static uint64_t base_address = 0;

// shadPS4 base address (old)
#define SHADPS4_BASE 0x8ffffc000

// shadPS4 base address (new)
//#define SHADPS4_BASE 0x800000000 

uint64_t get_base_address() {
    if (base_address != 0) {
        return base_address;
    }
    if (sys_sdk_proc_info(&procInfo) != 0) {
        // syscall failed, probably shadPS4
        base_address = SHADPS4_BASE;
    } else {
        base_address = procInfo.base_address;
    }
    return base_address;
}

bool file_exists(const char* filename) {
    struct stat buff;
    return stat(filename, &buff) == 0 ? true : false;
}

float read_file_as_float(const char* filename) {
    float out;
    FILE* fptr;
    fptr = fopen(filename, "r"); 
    char string[100];
    fgets(string, 100, fptr);
    const char* stringptr = string;
    out = atof(stringptr);
    fclose(fptr);
    return out;
}