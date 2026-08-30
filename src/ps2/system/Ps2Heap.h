// Ps2Heap.h -- allocation-failure diagnostics for the Emotion Engine.
//
// The PS2 counterpart of src/wii/WiiHeap.h, and deliberately the same shape:
// platform/Diagnostics.h is the seam both consoles are reached through, and
// GuiErrorScreen renders exactly five short lines from it. Before this file
// that seam returned empty strings on PS2, so the out-of-memory screen came up
// blank on the console with the smallest heap of the two.
//
// What this does NOT catch
// ------------------------
// Only allocation FAILURE. A heap whose structures have been corrupted -- by
// anything writing past the end of a buffer, gsKit's unchecked draw queue being
// the documented offender here -- does not fail: malloc walks a broken free
// list and never returns. That looks like a hang, not an OOM, and no amount of
// bad_alloc reporting will describe it. ps2_gs_queue_check_overflow() in
// Ps2GsQueue.cpp exists for that half of the problem.
#pragma once

#ifdef PS2_PLATFORM

// Fill the diagnostic lines from the last throwing allocation plus the current
// heap state. Called from platformCaptureBadAlloc() when bad_alloc is caught,
// while the heap is already under pressure -- so it allocates nothing.
void ps2HeapCaptureBadAlloc();

// One captured line, index 0..4. Never null; an uncaptured slot is "".
const char* ps2HeapLastBadAllocLine(unsigned index);

#endif // PS2_PLATFORM
