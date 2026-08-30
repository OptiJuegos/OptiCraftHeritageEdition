#pragma once
#ifdef PS2_PLATFORM

// Returns true if mc0: slot 0 is formatted and ready.
bool ps2_mc_is_formatted();

// Kicks off an async mcFormat(0,0). Returns false if it couldn't start.
bool ps2_mc_format_start();

// Polls the in-progress format. Returns:
//   0  = still running
//   1  = done, *result >= 0 means success
//  -1  = error
int  ps2_mc_format_poll(int* result_out);

#endif
