#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>
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
    OrbisKernelModuleInfoEx info;
    memset(&info, 0, sizeof(info));
    info.size = sizeof(info);
    OrbisAppInfo appinfo;
    memset(&appinfo, 0, sizeof(appinfo));
    sceKernelGetAppInfo(0, &appinfo);
    for (pid_t moduleId = 0; moduleId <= 1024; moduleId++)
    {
        if (sceKernelGetAppInfo(moduleId, &appinfo) < 0)
            continue;

        if (!(appinfo.TitleId[0] == '\0')) {
            is_emu = false; // if a title id was found, its not shadps4 (getappinfo doesn't work)
            //final_printf("app info: titleid: %s\n", appinfo.TitleId);
            if (strcmp(appinfo.TitleId, "CUSA02084") == 0) {
                final_printf("US Rock Band 4 Detected!\n");
            }
            else if (strcmp(appinfo.TitleId, "CUSA02901") == 0) {
                final_printf("EU Rock Band 4 Detected!\n");
            }
            else {
                final_printf("Game loaded is not Rock Band 4!\n");
                return 0;
            }
            break;
        }
        else {
            is_emu = true;
        }
    }
    for (int moduleId = 0; moduleId < 1024; moduleId++)
    {
        if (sceKernelGetModuleInfoInternal(moduleId, &info) < 0)
            continue;

        if (is_emu) {
            if (strcmp(info.name, "rockband_ps4_s.sprx") == 0) {
                base_address = (uint64_t)info.segmentInfo[0].address;
                break;
            }
        }
        
        if (strcmp(info.name, "eboot.bin") == 0)
        {
            base_address = (uint64_t)info.segmentInfo[0].address;
            break;
        }
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
symvariable symvariables[100];
int num_symvars = 0;

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

void set_plugin_symvar(const char* name, Symbol value) {
    // Check if the variable already exists
    for (int i = 0; i < num_symvars; i++) {
        if (strcmp(symvariables[i].name, name) == 0) {
            symvariables[i].value = value; // direct struct copy
            return;
        }
    }

    // Create new variable
    strcpy(symvariables[num_symvars].name, name);
    symvariables[num_symvars].value = value;
    num_symvars++;
}

Symbol get_plugin_symvar(const char* name) {
    for (int i = 0; i < num_symvars; i++) {
        if (strcmp(symvariables[i].name, name) == 0) {
            return symvariables[i].value;
        }
    }

    // return empty symbol if not found
    Symbol empty = { .sym = "" };
    return empty;
}