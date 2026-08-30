#pragma once
#ifdef PS2_PLATFORM

enum SaveLocation {
    SAVE_LOC_MC   = 0,
    SAVE_LOC_MASS = 1,
    SAVE_LOC_NONE = 2
};

SaveLocation ps2_show_saves_prompt();
bool ps2_show_format_prompt();
bool ps2_do_format_mc();

#endif
