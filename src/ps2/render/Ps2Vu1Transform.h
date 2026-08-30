#pragma once

#if defined(PS2_PLATFORM) && defined(PS2_ENABLE_VU1_TRANSFORM)

struct Ps2Vu1ClipVertex {
    float x;
    float y;
    float z;
    float w;
};

enum {
    PS2_VU1_TRANSFORM_MAX_VERTICES = 120
};

// Uploads the draw MVP to VU1. Any previous transform packet is completed
// first, so callers can safely begin a new draw without carrying VIF1 state.
bool ps2_vu1_transform_begin(const float* mvp);

// Starts one contiguous vertex batch. The source stream is XYZ float data;
// stride is in bytes. bank must be 0 or 1 so the EE can consume one output
// bank while VU1 fills the other.
bool ps2_vu1_transform_submit(const void* vertices, int stride,
                              int first, int count, int bank);

// Waits for the current VIF1 packet. Each transform packet ends in FLUSHE, so
// DMA completion also means the VU1 microprogram has finished writing output.
bool ps2_vu1_transform_wait();

// Reads one clip-space vertex from a completed VU1 output bank.
void ps2_vu1_transform_read(int bank, int index, Ps2Vu1ClipVertex& out);

#endif
