#define PLUGIN_NAME "RB4DX"
#define final_printf(a, args...) klog("[" PLUGIN_NAME "] " a, ##args)

static struct proc_info procInfo;

bool file_exists(const char*);