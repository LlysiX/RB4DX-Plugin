#define PLUGIN_NAME "RB4DX"
#define final_printf(a, args...) klog("[" PLUGIN_NAME "] " a, ##args)

#include "rb4/data.h"

static struct proc_info procInfo;

bool file_exists(const char*);

float read_file_as_float(const char*);

uint64_t get_base_address();

typedef struct {
    char name[50];
    int value;
} variable;

typedef struct {
    char name[50];
    Symbol value;
} symvariable;

extern bool is_emu;
extern variable variables[100];
extern int num_vars;
extern symvariable symvariables[100];
extern int num_symvars;

void set_plugin_var(const char*, int);
int get_plugin_var(const char*);
void set_plugin_symvar(const char*, Symbol);
Symbol get_plugin_symvar(const char*);