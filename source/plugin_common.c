#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <GoldHEN/Common.h>

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