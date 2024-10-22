#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <GoldHEN/Common.h>

bool file_exists(const char* filename) {
    struct stat buff;
    return stat(filename, &buff) == 0 ? true : false;
}