#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/storage/save/McSavePS2.h"
#include <libmc.h>
#include <stdio.h>

bool ps2_mc_is_formatted() {
    int type = 0, free_space = 0, format = 0;
    mcGetInfo(0, 0, &type, &free_space, &format);
    int cmd = 0, result = -1;
    mcSync(0, &cmd, &result);
    if (result < 0) return false;
    return (format != 0);
}

bool ps2_mc_format_start() {
    int ret = mcFormat(0, 0);
    if (ret < 0) {
        MC_LOG_ERROR("save", "[PS2] mcFormat start failed: %d\n", ret);
        return false;
    }
    return true;
}

int ps2_mc_format_poll(int* result_out) {
    int cmd = 0, result = -1;
    int sync = mcSync(MC_NOWAIT, &cmd, &result);
    if (sync == 1) {
        if (result_out) *result_out = result;
        MC_LOG_INFO("save", "[PS2] mcFormat done: result=%d\n", result);
        return 1;
    }
    if (sync < 0) {
        MC_LOG_ERROR("save", "[PS2] mcSync error: %d\n", sync);
        if (result_out) *result_out = -1;
        return -1;
    }
    return 0; // still running
}

#endif // PS2_PLATFORM
