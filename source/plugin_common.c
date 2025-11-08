#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <GoldHEN/Common.h>
#include "plugin_common.h"
#include <fcntl.h>
#include <unistd.h>

static uint64_t base_address = 0;

// shadPS4 base address (old)
//#define SHADPS4_BASE 0x8ffffc000

// shadPS4 base address (new)
#define SHADPS4_BASE 0x800000000 

uint64_t get_base_address() {
    if (base_address != 0) {
        return base_address;
    }
    if (is_emu) {
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
    int fd = open(filename, O_RDONLY);
    //rewritten FILE* with ps4 specific file functions to work on shadps4
    if (fd == -1) {
        return 0.0f; // or handle error appropriately
    }

    char string[100];
    ssize_t bytes_read = read(fd, string, sizeof(string) - 1);
    if (bytes_read > 0) {
        string[bytes_read] = '\0'; // null-terminate the string

        // Find first newline and terminate there (mimics fgets behavior)
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (string[i] == '\n') {
                string[i] = '\0';
                break;
            }
        }

        out = atof(string);
    }
    else {
        out = 0.0f;
    }

    close(fd);
    return out;
}

variable variables[100];
int num_vars = 0;

void set_plugin_var(const char* name, int value) {
    // Check if the variable already exists
    for (int i = 0; i < num_vars; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }
    // Create new variable if it doesn't
    strcpy(variables[num_vars].name, name);
    variables[num_vars].value = value;
    num_vars++;
}

int get_plugin_var(const char* name) {
    for (int i = 0; i < num_vars; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    return 0; // Not found, return 0
}