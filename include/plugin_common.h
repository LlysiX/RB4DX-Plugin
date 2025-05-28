#define PLUGIN_NAME "RB4DX"
#define final_printf(a, args...) klog("[" PLUGIN_NAME "] " a, ##args)

static struct proc_info procInfo;

bool file_exists(const char*);

float read_file_as_float(const char*);

uint64_t get_base_address();
