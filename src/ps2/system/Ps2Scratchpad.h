// Ps2Scratchpad.h — allocation map for the Emotion Engine scratchpad (SPR).
//
// The EE has 16 KB of SRAM at 0x70000000 that is addressed like ordinary memory
// but answers with no wait states and never goes through the data cache. It is
// the only fast memory on this console that nothing else was using: before this
// file there was not a single reference to 0x70000000 in the tree.
//
// Why it is worth using at all
// ----------------------------
// The R5900's data cache is 8 KB. The GIF packet staging buffers in
// Ps2Vu0Draw3D are 6 KB and 9 KB, so the triangle buffer alone is larger than
// the whole cache: filling it evicts its own start before the flush reads it
// back, every batch, all frame. Moving both into SPR removes that traffic
// entirely rather than merely reducing it.
//
// Why a central map instead of each user picking an address
// ---------------------------------------------------------
// There is no allocator here and no hardware protection. Two users that both
// "just take the start of the scratchpad" produce silent corruption of GS
// packets, which surfaces as scrambled geometry a long way from the cause. The
// offsets below are the single authority; each region is claimed here, and its
// user static_asserts that what it wants to store actually fits.
//
// The rules a new region has to respect
// -------------------------------------
//   * CPU access only. A region may be read or written by the EE core, and may
//     be the SOURCE of a CPU-side copy (memcpy, or gsKit's own copy loops).
//     It must NOT be handed to a DMA channel as a plain address: DMA to or from
//     scratchpad needs the SPR bit in MADR and the fromSPR/toSPR channels, and
//     nothing here sets that up. Both current users were checked against the
//     linked ELF -- gsKit_prim_list_triangle_goraud_texture_stq_3d tail-calls
//     memcpy from the caller's array, and flushStrip memcpy's explicitly -- so
//     neither buffer is ever a DMA source.
//
//   * Main thread only. The scratchpad is NOT part of a thread context and is
//     not saved or restored across a switch. src/ps2/input/Ps2Input.cpp starts
//     one EE thread for pad initialisation; it does not render and does not
//     touch SPR, which is what makes single-thread use safe rather than lucky.
//     A second renderer thread would need this map revisited.
//
//   * No static initialisation. Contents are undefined at boot and after any
//     code that was not written with this map in hand.
//
// PS2_ENABLE_SCRATCHPAD exists so this can be bisected: with it at 0 every user
// falls back to an ordinary static array in main RAM, which is exactly the code
// that ran before. If GS packets ever come out scrambled, flip it and see.
#pragma once

#ifdef PS2_PLATFORM

#define PS2_ENABLE_SCRATCHPAD 1

// Hardware constants, not choices.
#define PS2_SPR_BASE  0x70000000u
#define PS2_SPR_BYTES (16u * 1024u)

// ---- Region map ----------------------------------------------------------
// Offsets are in bytes from PS2_SPR_BASE and must stay 16-byte aligned: every
// current user stores quadword GS registers.
//
//   0x0000  6144  Ps2Vu0Draw3D s_stripBatch   (128 GSPRIMSTQPOINT)
//   0x1800  9216  Ps2Vu0Draw3D batch          (192 GSPRIMSTQPOINT)
//   0x3C00  1024  free
//
// The two draw buffers are live at the same time -- a single draw call can
// flush a triangle batch and a quad strip -- so they cannot share a region.
#define PS2_SPR_STRIP_BATCH_OFFSET 0x0000u
#define PS2_SPR_STRIP_BATCH_BYTES  6144u

#define PS2_SPR_TRI_BATCH_OFFSET   0x1800u
#define PS2_SPR_TRI_BATCH_BYTES    9216u

static_assert(PS2_SPR_STRIP_BATCH_OFFSET + PS2_SPR_STRIP_BATCH_BYTES
              <= PS2_SPR_TRI_BATCH_OFFSET,
              "PS2 scratchpad: strip batch region overlaps the triangle batch");
static_assert(PS2_SPR_TRI_BATCH_OFFSET + PS2_SPR_TRI_BATCH_BYTES <= PS2_SPR_BYTES,
              "PS2 scratchpad: region map exceeds the 16KB scratchpad");

// Typed pointer to a region. The cast is the whole implementation: scratchpad
// is directly addressable, so there is nothing to map or reserve.
template <typename T>
static inline T *ps2_spr_region(unsigned int offset)
{
	return reinterpret_cast<T *>(PS2_SPR_BASE + offset);
}

#endif // PS2_PLATFORM
