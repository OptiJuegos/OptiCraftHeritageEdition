#include "platform/Log.h"
#include "ps2/render/Ps2TextureGs.h"

#ifdef PS2_PLATFORM

#include <gsKit.h>
#include <gsTexture.h>
#include <gsInline.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cstdint>
#include <limits>

#include "ps2/render/Ps2Graphics.h"
#include "ps2/render/Ps2Tuning.h" // PS2_TEXTURE_PARTIAL_UPLOAD

// The bound name. This is render state rather than a texture property, but it
// belongs to whoever owns the table: the GS draw paths ask this module which
// texture to rasterise with, and the OpenGL compatibility layer forwards
// glBindTexture here instead of keeping a second copy that could drift.
static unsigned int s_boundTex = 0;

// GS PSM_CT16: 16-bit A1B5G5R5 — [15]=A, [14:10]=B, [9:5]=G, [4:0]=R.
// Halves VRAM vs CT32 (the GS only has 4 MB). Alpha drops to 1 bit, fine for
// the alpha-tested terrain cutouts; GUI translucency degrades slightly.
static inline u16 rgba_to_psmct16(const u8* src) {
    u16 r = (u16)(src[0] >> 3);
    u16 g = (u16)(src[1] >> 3);
    u16 b = (u16)(src[2] >> 3);
    u16 a = (u16)(src[3] >= 128 ? 1 : 0);
    return (u16)((a << 15) | (b << 10) | (g << 5) | r);
}

#ifdef PS2_ENABLE_PSMT8
// ---- PSMT8 palettization ----
//
// Game textures are stored as 8-bit palette indices + a 256-entry CT16 CLUT:
// half the VRAM and EE RAM of CT16, and half the DMA per animated-texture
// re-upload. The CLUT keeps the exact same A1B5G5R5 colors CT16 had, so any
// texture with <= 256 unique CT16 colors converts losslessly; richer textures
// (the prerendered gui_blocks atlas) quantize to their 256 most popular CT16
// colors with nearest-match remapping.

// The GS reads an 8-bit CSM1 CLUT with index bits 3 and 4 swapped (entries
// arranged as 8-column blocks). Store entry i at this position so TEX0 lookups
// hit the right color.
static inline int ps2_clut_csm1_pos(int i) {
    return (i & ~0x18) | ((i & 0x08) << 1) | ((i & 0x10) >> 1);
}

// Squared distance between two CT16 colors; alpha-bit mismatch is penalized
// so cutout pixels never snap to an opaque color (and vice versa).
static inline int ps2_ct16_dist(u16 a, u16 b) {
    int dr = (int)(a & 31)         - (int)(b & 31);
    int dg = (int)((a >> 5) & 31)  - (int)((b >> 5) & 31);
    int db = (int)((a >> 10) & 31) - (int)((b >> 10) & 31);
    int da = ((a ^ b) >> 15) & 1;
    return dr*dr + dg*dg + db*db + da * 0x10000;
}

// Build a <=256-color palette for npx CT16 pixels and fill idx[] with the
// palette index of every pixel. palette[] receives the entries in linear
// (unswizzled) order; returns the palette size.
static int ps2_palettize_ct16(const u16* px, u32 npx, u8* idx, u16* palette) {
    // CT16 is only 16 bits, so an exact histogram over all 65536 values is
    // cheap (256 KB, transient) and avoids any hashing.
    u32* hist = (u32*)calloc(65536, sizeof(u32));
    u16* lut  = (u16*)malloc(65536 * sizeof(u16)); // CT16 value -> palette index
    if (!hist || !lut) {
        if (hist) free(hist);
        if (lut) free(lut);
        return 0;
    }
    for (u32 p = 0; p < npx; p++)
        hist[px[p]]++;

    int nPal = 0;
    // First pass: take colors while they fit (covers the lossless case).
    for (int v = 0; v < 65536; v++) {
        if (!hist[v]) continue;
        if (nPal < 256) {
            lut[v] = (u16)nPal;
            palette[nPal++] = (u16)v;
        } else { nPal = 257; break; }
    }

    if (nPal > 256) {
        // Too many colors: keep the 256 most popular, remap the rest to the
        // nearest kept color. Selection sort of the top 256 over the (sparse)
        // histogram is fine for a one-time load cost.
        nPal = 0;
        for (int k = 0; k < 256; k++) {
            u32 bestCount = 0; int bestVal = -1;
            for (int v = 0; v < 65536; v++) {
                if (hist[v] > bestCount) { bestCount = hist[v]; bestVal = v; }
            }
            if (bestVal < 0) break;
            hist[bestVal] = 0;          // consumed
            lut[bestVal] = (u16)nPal;
            palette[nPal++] = (u16)bestVal;
        }
        // Remap every remaining (dropped) color to its nearest palette entry.
        for (int v = 0; v < 65536; v++) {
            if (!hist[v]) continue;     // not present or already in palette
            int best = 0, bestD = 0x7FFFFFFF;
            for (int k = 0; k < nPal; k++) {
                int d = ps2_ct16_dist((u16)v, palette[k]);
                if (d < bestD) { bestD = d; best = k; }
            }
            lut[v] = (u16)best;
        }
    }

    for (u32 p = 0; p < npx; p++)
        idx[p] = (u8)lut[px[p]];

    free(hist);
    free(lut);
    return nPal;
}
#endif // PS2_ENABLE_PSMT8

// ---- Texture table ----

struct PS2Tex {
    GSTEXTURE gs;
    u16*      cpuMem;  // CT16 path: pixel copy for glTexSubImage2D; 64-byte aligned for GS DMA
#ifdef PS2_ENABLE_PSMT8
    u8*       cpuIdx;  // T8 path: 8-bit palette indices (this is gs.Mem at upload time)
    u16*      clut;    // T8 path: 256 CT16 entries, CSM1-swizzled, 64-byte aligned
    u16*      remap;   // T8 path: lazy CT16->index memo for glTexSubImage2D (0xFFFF = empty)
    u32       clutVramSize;
#endif
    u32       vramSize; // bytes reserved in GS VRAM (for the reuse free-list)
    bool      valid;
    bool      uploaded;
    bool      tileAtlas;
    bool      dirtyUpload; // sub-image touched the cpu copy; re-send on next bind
    // Which GS page rows that traffic covered, one bit per 64 texel rows. Zero
    // while dirtyUpload is set means "re-send all of it" -- what an unsupported
    // geometry leaves behind, and the state a fresh texture starts in.
    u32       dirtyPageRows;
};
static PS2Tex s_tex[PS2_MAX_TEX];

// ---- GS VRAM free-list ----
//
// gsKit_vram_alloc() is a one-way bump allocator (gsGlobal->CurrentPointer);
// there is no per-block free, only gsKit_vram_clear() which wipes ALL user
// VRAM. So glDeleteTextures() / texture re-upload used to leak VRAM forever:
// the bump pointer kept climbing until the 4 MB GS ran out and new textures
// (terrain/items/gui_blocks) silently failed to upload = missing hotbar icons.
//
// Fix: recycle freed VRAM blocks here. ps2_vram_alloc() first tries to reuse a
// freed block of equal-or-greater size (first fit), only bumping CurrentPointer
// via gsKit when nothing fits. Game textures use a small fixed set of sizes that
// repeat across world reloads, so reuse keeps VRAM bounded.
#define PS2_MAX_VRAM_FREE 256
struct PS2VramBlock { u32 vram; u32 size; };
static PS2VramBlock s_vramFree[PS2_MAX_VRAM_FREE];
static int          s_vramFreeCount = 0;

static u32 ps2_vram_alloc(u32 size) {
    // Reuse the smallest freed block that still fits (reduces wasted slack).
    int best = -1;
    for (int i = 0; i < s_vramFreeCount; i++) {
        if (s_vramFree[i].size >= size &&
            (best < 0 || s_vramFree[i].size < s_vramFree[best].size))
            best = i;
    }
    if (best >= 0) {
        u32 vram = s_vramFree[best].vram;
        s_vramFree[best] = s_vramFree[--s_vramFreeCount];
        return vram;
    }
    return gsKit_vram_alloc(gsGlobal, size, GSKIT_ALLOC_USERBUFFER);
}

static void ps2_vram_free(u32 vram, u32 size) {
    if (vram == GSKIT_ALLOC_ERROR || size == 0)
        return;
    if (s_vramFreeCount < PS2_MAX_VRAM_FREE) {
        s_vramFree[s_vramFreeCount].vram = vram;
        s_vramFree[s_vramFreeCount].size = size;
        s_vramFreeCount++;
    }
    // If the free-list is full we simply drop the block (leak it); 256 distinct
    // freed sizes is far more than this game ever churns, so this never trips.
}

extern "C" int ps2_dbg_vram_current_kb()
{
    return gsGlobal ? (int)(gsGlobal->CurrentPointer / 1024) : 0;
}

extern "C" int ps2_dbg_vram_free_blocks()
{
    return s_vramFreeCount;
}

extern "C" int ps2_dbg_vram_recycled_kb()
{
    u32 total = 0;
    for (int i = 0; i < s_vramFreeCount; ++i)
        total += s_vramFree[i].size;
    return (int)(total / 1024);
}

bool ps2_texture_valid(unsigned int name) {
    return name > 0 && name < PS2_MAX_TEX &&
           s_tex[name].valid && s_tex[name].uploaded;
}

bool ps2_texture_discard_cpu_mirror(unsigned int name) {
    if (name == 0 || name >= PS2_MAX_TEX)
        return false;
    PS2Tex& te = s_tex[name];
    // If the first upload could not be sent immediately, ps2_texture_resolve()
    // still needs the mirror to retry. Never discard while a deferred upload is
    // pending or before the texture has reached GS VRAM at least once.
    if (!te.valid || !te.uploaded || te.dirtyUpload)
        return false;

    if (te.cpuMem) {
        free(te.cpuMem);
        te.cpuMem = nullptr;
    }
#ifdef PS2_ENABLE_PSMT8
    if (te.cpuIdx) {
        free(te.cpuIdx);
        te.cpuIdx = nullptr;
    }
    if (te.clut) {
        free(te.clut);
        te.clut = nullptr;
        te.gs.Clut = nullptr;
    }
    if (te.remap) {
        free(te.remap);
        te.remap = nullptr;
    }
#endif
    te.gs.Mem = nullptr;
    return true;
}

// Kept as an extern "C" entry point because RenderEngine calls it to retry a
// texture whose load failed (mid-game OOM/VRAM-full) instead of caching a
// permanently broken id -- the "flat white GUI" bug.


void ps2_texture_generate_names(int count, unsigned int* names) {
    if (count <= 0 || names == nullptr)
        return;
    int found = 0;
    for (unsigned int i = 1; i < PS2_MAX_TEX && found < count; i++) {
        if (!s_tex[i].valid) {
            s_tex[i].valid = true; s_tex[i].uploaded = false;
            s_tex[i].tileAtlas = false;
            s_tex[i].cpuMem = nullptr;
            s_tex[i].dirtyUpload = false;
            s_tex[i].dirtyPageRows = 0;
#ifdef PS2_ENABLE_PSMT8
            s_tex[i].cpuIdx = nullptr;
            s_tex[i].clut = nullptr;
            s_tex[i].remap = nullptr;
            s_tex[i].clutVramSize = 0;
#endif
            memset(&s_tex[i].gs, 0, sizeof(GSTEXTURE));
            names[found++] = i;
        }
    }
    while (found < count)
        names[found++] = 0;
}
void ps2_texture_bind(unsigned int name) {
    s_boundTex = name;
}

unsigned int ps2_texture_bound_name() {
    return s_boundTex;
}

void ps2_texture_set_tile_atlas(unsigned int name, bool tileAtlas) {
    if (name > 0 && name < PS2_MAX_TEX && s_tex[name].valid)
        s_tex[name].tileAtlas = tileAtlas;
}

bool ps2_texture_bound_is_tile_atlas() {
    return s_boundTex > 0 && s_boundTex < PS2_MAX_TEX &&
           s_tex[s_boundTex].valid && s_tex[s_boundTex].tileAtlas;
}
void ps2_texture_delete_names(int count, const unsigned int* names) {
    if (count <= 0 || names == nullptr)
        return;
    for (int i = 0; i < count; i++) {
        const unsigned int id = names[i];
        if (id > 0 && id < PS2_MAX_TEX && s_tex[id].valid) {
            if (s_tex[id].cpuMem) { free(s_tex[id].cpuMem); s_tex[id].cpuMem = nullptr; }
#ifdef PS2_ENABLE_PSMT8
            if (s_tex[id].cpuIdx) { free(s_tex[id].cpuIdx); s_tex[id].cpuIdx = nullptr; }
            if (s_tex[id].clut)   { free(s_tex[id].clut);   s_tex[id].clut = nullptr; }
            if (s_tex[id].remap)  { free(s_tex[id].remap);  s_tex[id].remap = nullptr; }
            ps2_vram_free(s_tex[id].gs.VramClut, s_tex[id].clutVramSize);
            s_tex[id].clutVramSize = 0;
            s_tex[id].gs.VramClut = 0;
            s_tex[id].gs.Clut = nullptr;
#endif
            ps2_vram_free(s_tex[id].gs.Vram, s_tex[id].vramSize);
            s_tex[id].vramSize = 0;
            s_tex[id].gs.Vram = 0;
            s_tex[id].gs.Mem = nullptr;
            s_tex[id].dirtyUpload = false;
            s_tex[id].valid = s_tex[id].uploaded = false;
        }
    }
}

// GS page geometry for the two formats this backend stores textures in.
//
// A page is 8KB in every format; only its texel dimensions change. Both of ours
// are 64 texel rows tall, and that is the granularity a partial upload can
// address -- not the 16x16 tile TextureFX actually rewrites.
//
// The reason is that gsKit_texture_send() hardcodes TRXPOS to (0,0), so the only
// way to aim a transfer somewhere other than the start of the buffer is to move
// the destination base pointer, and that pointer addresses pages. Within a page
// row the buffer is linear, so page row N of a texture that is a whole number of
// pages wide begins at byte offset N * pageH * width * bytesPerTexel -- the same
// offset as in the CPU copy, which is what makes the source pointer arithmetic
// below the plain linear one.
static bool ps2_texture_page_geometry(u32 psm, int* pageW, int* pageH) {
    if (psm == GS_PSM_T8)   { *pageW = 128; *pageH = 64; return true; }
    if (psm == GS_PSM_CT16) { *pageW = 64;  *pageH = 64; return true; }
    return false;
}

// True when this texture can be uploaded one page row at a time. A width that is
// not a whole number of pages leaves TBW padding between page rows, which breaks
// the contiguous-slab assumption the byte offset above depends on.
static bool ps2_texture_supports_partial(const PS2Tex& te) {
#if PS2_TEXTURE_PARTIAL_UPLOAD
    int pageW = 0, pageH = 0;
    if (!ps2_texture_page_geometry(te.gs.PSM, &pageW, &pageH))
        return false;
    if (te.gs.Width == 0 || te.gs.Height == 0)
        return false;
    if ((te.gs.Width % (u32)pageW) != 0 || (te.gs.Height % (u32)pageH) != 0)
        return false;
    return (te.gs.Height / (u32)pageH) <= 32; // one bit per page row
#else
    (void)te;
    return false;
#endif
}

// gsKit_texture_send() memaligns a DMA-chain packet and never checks the
// result — under heap exhaustion it writes the GIF tags through NULL (TLB-miss
// storm at 0x0..0x98). For sub-512KB textures the packet EMBEDS the texture
// data (disasm: alloc = (9 + data_qwc + 2)*16 ≈ texture bytes + tags), so the
// probe must cover the full texture size — a small fixed probe "passed" while
// the real ~64KB terrain re-send still failed. gsKit ships prebuilt, so probe
// first and skip the send (retrying via dirtyUpload) when it cannot fit.
static u32 ps2_upload_packet_bytes_for(int width, int height, int psm) {
    return gsKit_texture_size_ee(width, height, psm) + 1024;
}

static u32 ps2_upload_packet_bytes(const GSTEXTURE* t) {
    return ps2_upload_packet_bytes_for((int)t->Width, (int)t->Height, (int)t->PSM);
}

static bool ps2_heap_can_upload_bytes(size_t need) {
    void* probe = memalign(64, need);
    if (!probe)
        return false;
    free(probe);
    return true;
}

// Parked upload block — the fix for the heap creep that ends in "Out of memory!".
//
// The animated terrain tiles (water/lava/fire/portal) mark terrain.png dirty
// every tick, so the deferred flush below re-sends it -- the whole 256x256 atlas
// before PS2_TEXTURE_PARTIAL_UPLOAD, the dirty page rows now, but a recurring
// transient block either way.
// Each send is a memalign(64, ~65.7KB) inside gsKit that is freed again before
// the call returns, and the probe took a second one. That is
// two ~64KB transient blocks per tick, forever, at the top of the heap: any
// small allocation that outlives one of them (a chunk, a mesh growth, an
// entity) lands in the hole, and the next 64KB request no longer fits, so the
// arena sbrk's another ~68KB. The FRAME log showed exactly that — mallocUsed
// flat while the arena grew in 68KB steps until free hit 0.
//
// So instead of leaving that block transient, hold one permanently and lend it
// to gsKit: free it immediately before the send and re-take it immediately
// after. Nothing else can run in between (single threaded), so the send always
// finds a hole of the right size in the same place, and it can never fail —
// which also keeps the NULL-memalign TLB-miss guard this replaced.
static void*  s_uploadPark      = nullptr;
static size_t s_uploadParkBytes = 0;
// Only park for the recurring in-world sizes. A one-off oversized upload
// (startup art) is better served by the probe path than by pinning RAM.
static const size_t PS2_UPLOAD_PARK_MAX = 160 * 1024;

// Bytes currently lent to gsKit, or 0 when the park is not lent out. Split from
// the upload itself so the whole-texture and per-page-row paths share one park
// instead of each fighting the heap with its own transient block.
static size_t s_uploadParkLent = 0;

// Frees the park so gsKit's own memalign lands in the hole it leaves. Returns
// false when the heap cannot serve `need` at all; the caller must then skip the
// send rather than let gsKit DMA through a NULL packet.
static bool ps2_upload_park_take(size_t need) {
    if (need <= PS2_UPLOAD_PARK_MAX) {
        if (s_uploadPark == nullptr || s_uploadParkBytes < need) {
            // Grow the park to the largest recurring packet. A bigger park than
            // gsKit asks for is fine: malloc splits it for the send and
            // coalesces the pieces back when gsKit frees, so the re-take gets
            // the same block again.
            if (s_uploadPark != nullptr) {
                free(s_uploadPark);
                s_uploadPark = nullptr;
                s_uploadParkBytes = 0;
            }
            void* p = memalign(64, need);
            if (p != nullptr) {
                s_uploadPark = p;
                s_uploadParkBytes = need;
            }
        }
        if (s_uploadPark != nullptr && s_uploadParkBytes >= need) {
            s_uploadParkLent = s_uploadParkBytes;
            free(s_uploadPark);
            s_uploadPark = nullptr;
            return true;
        }
    }

    // No park (heap too tight to hold one, or a one-off oversized texture):
    // fall back to probing before the unchecked memalign inside gsKit.
    s_uploadParkLent = 0;
    return ps2_heap_can_upload_bytes(need);
}

static void ps2_upload_park_return() {
    if (s_uploadParkLent == 0)
        return;
    s_uploadPark = memalign(64, s_uploadParkLent);
    s_uploadParkBytes = (s_uploadPark != nullptr) ? s_uploadParkLent : 0;
    s_uploadParkLent = 0;
}

// Uploads `t`, preferring the parked block. Returns false only when the heap
// cannot serve the packet at all — the caller then keeps dirtyUpload set and
// retries on a later frame instead of letting gsKit DMA through a NULL packet.
static bool ps2_texture_upload_guarded(GSTEXTURE* t) {
    if (!ps2_upload_park_take((size_t)ps2_upload_packet_bytes(t)))
        return false;
    gsKit_texture_upload(gsGlobal, t);
    ps2_upload_park_return();
    return true;
}

bool ps2_texture_upload_rgba(unsigned int name, int level, int width, int height,
                             const void* pixels) {
    if (level != 0 || !pixels || !gsGlobal) return false;
    const unsigned int id = name;
    if (id == 0 || id >= PS2_MAX_TEX || !s_tex[id].valid) return false;
    const int w = width;
    const int h = height;
    if (w <= 0 || h <= 0) return false;
    if (static_cast<size_t>(w) > std::numeric_limits<size_t>::max() / static_cast<size_t>(h)) return false;
    const size_t pixelCount = static_cast<size_t>(w) * static_cast<size_t>(h);
    if (pixelCount > static_cast<size_t>(std::numeric_limits<u32>::max()) ||
        pixelCount > std::numeric_limits<size_t>::max() / sizeof(u16)) return false;

    PS2Tex& te = s_tex[id];
    if (te.cpuMem) { free(te.cpuMem); te.cpuMem = nullptr; }
    // Re-specifying an existing texture: recycle its old VRAM block first.
    ps2_vram_free(te.gs.Vram, te.vramSize);
    te.vramSize = 0;
    te.gs.Vram = 0;
    te.gs.Mem = nullptr;
    te.uploaded = false;
    te.dirtyUpload = false;
    te.dirtyPageRows = 0;
#ifdef PS2_ENABLE_PSMT8
    if (te.cpuIdx) { free(te.cpuIdx); te.cpuIdx = nullptr; }
    if (te.clut)   { free(te.clut);   te.clut = nullptr; }
    if (te.remap)  { free(te.remap);  te.remap = nullptr; }
    ps2_vram_free(te.gs.VramClut, te.clutVramSize);
    te.clutVramSize = 0;
    te.gs.VramClut = 0;
    te.gs.Clut = nullptr;
#endif

    te.gs.Width  = (u32)w;
    te.gs.Height = (u32)h;
    te.gs.Filter = GS_FILTER_NEAREST;
    te.gs.Delayed = 0;

    const u32 npx = static_cast<u32>(pixelCount);
    // CT16 conversion buffer: pixel source for the T8 palettizer, or the
    // long-lived CPU copy on the CT16 path. 64-byte aligned for GS DMA.
    u16* px16 = (u16*)memalign(64, npx * sizeof(u16));
    if (!px16) {
        MC_LOG_ERROR("render", "[PS2] ps2_texture_upload_rgba: OUT OF RAM for %dx%d px16 (tex id=%u)\n", w, h, id);
        te.valid = false;
        return false;
    }
    const u8* src = (const u8*)pixels;
    for (u32 p = 0; p < npx; p++)
        px16[p] = rgba_to_psmct16(src + p * 4);

#ifdef PS2_ENABLE_PSMT8
    // 8-bit palettized: 1 byte/pixel + 512-byte CLUT. Exact for <=256 unique
    // CT16 colors, nearest-match quantized beyond that (see ps2_palettize_ct16).
    te.cpuIdx = (u8*)memalign(64, npx);
    te.clut   = (u16*)memalign(64, 256 * sizeof(u16));
    u16 paletteLin[256];
    int nPal = 0;
    if (te.cpuIdx && te.clut)
        nPal = ps2_palettize_ct16(px16, npx, te.cpuIdx, paletteLin);
    free(px16);
    if (nPal <= 0) {
        MC_LOG_ERROR("render", "[PS2] ps2_texture_upload_rgba: palettize failed for %dx%d tex id=%u\n", w, h, id);
        if (te.cpuIdx) { free(te.cpuIdx); te.cpuIdx = nullptr; }
        if (te.clut)   { free(te.clut);   te.clut = nullptr; }
        te.valid = false;
        return false;
    }
    // CSM1 wants index bits 3/4 swapped in storage; unused entries stay
    // transparent black.
    memset(te.clut, 0, 256 * sizeof(u16));
    for (int i = 0; i < nPal; i++)
        te.clut[ps2_clut_csm1_pos(i)] = paletteLin[i];

    te.gs.PSM             = GS_PSM_T8;
    te.gs.ClutPSM         = GS_PSM_CT16;
    te.gs.Clut            = (u32*)te.clut;
    te.gs.ClutStorageMode = GS_CLUT_STORAGE_CSM1;
    gsKit_setup_tbw(&te.gs);

    u32 vsz  = gsKit_texture_size(w, h, GS_PSM_T8);
    u32 cvsz = gsKit_texture_size(16, 16, GS_PSM_CT16);
    te.gs.Vram     = ps2_vram_alloc(vsz);
    te.gs.VramClut = ps2_vram_alloc(cvsz);
    if (te.gs.Vram == GSKIT_ALLOC_ERROR || te.gs.VramClut == GSKIT_ALLOC_ERROR) {
        MC_LOG_WARN("render", "[PS2] ps2_texture_upload_rgba: VRAM full for %dx%d tex id=%u\n", w, h, id);
        if (te.gs.Vram != GSKIT_ALLOC_ERROR)     ps2_vram_free(te.gs.Vram, vsz);
        if (te.gs.VramClut != GSKIT_ALLOC_ERROR) ps2_vram_free(te.gs.VramClut, cvsz);
        te.gs.Vram = 0; te.gs.VramClut = 0;
        te.vramSize = te.clutVramSize = 0;
        free(te.cpuIdx); te.cpuIdx = nullptr;
        free(te.clut);   te.clut = nullptr;
        te.valid = false;
        return false;
    }
    te.vramSize = vsz;
    te.clutVramSize = cvsz;

    te.gs.Mem = (u32*)te.cpuIdx;
    if (!ps2_texture_upload_guarded(&te.gs)) // sends indices + CLUT
        te.dirtyUpload = true; // OOM right now: retry from the deferred flush
    te.gs.Mem = nullptr;
#else
    // PSM_CT16 (A1B5G5R5) uses 2 bytes/pixel, halving VRAM vs CT32.
    // Alpha is 1-bit: fully transparent (A=0) vs opaque (A=1), sufficient
    // for terrain cutouts. GUI translucency is approximated via GS alpha blend.
    te.gs.PSM    = GS_PSM_CT16;
    te.gs.ClutStorageMode = GS_CLUT_NONE;
    te.gs.Clut = nullptr; te.gs.VramClut = 0;
    te.gs.ClutPSM = 0;

    gsKit_setup_tbw(&te.gs);

    u32 vsz = gsKit_texture_size(w, h, GS_PSM_CT16);
    te.gs.Vram = ps2_vram_alloc(vsz);
    if (te.gs.Vram == GSKIT_ALLOC_ERROR) {
        MC_LOG_WARN("render", "[PS2] ps2_texture_upload_rgba: VRAM full for %dx%d tex id=%u\n", w, h, id);
        te.vramSize = 0;
        te.valid = false;
        free(px16);
        return false;
    }
    te.vramSize = vsz;
    te.cpuMem = px16;

    te.gs.Mem = (u32*)te.cpuMem;
    if (!ps2_texture_upload_guarded(&te.gs))
        te.dirtyUpload = true; // OOM right now: retry from the deferred flush
    te.gs.Mem = nullptr;
#endif
    te.uploaded = true;
    return true;
}

#ifdef PS2_ENABLE_PSMT8
// Map one CT16 color to the texture's palette index. Lazy 64K-entry memo
// (built only for textures that actually receive glTexSubImage2D traffic —
// in practice just terrain.png's animated water/lava/fire tiles, whose
// colors come from the original texture and therefore hit the palette).
static u8 ps2_t8_remap_color(PS2Tex& te, u16 v) {
    if (!te.remap) {
        te.remap = (u16*)malloc(65536 * sizeof(u16));
        if (!te.remap) return 0;
        memset(te.remap, 0xFF, 65536 * sizeof(u16));
        for (int i = 0; i < 256; i++) {
            u16 c = te.clut[ps2_clut_csm1_pos(i)];
            if (te.remap[c] == 0xFFFF)
                te.remap[c] = (u16)i;
        }
    }
    u16 m = te.remap[v];
    if (m != 0xFFFF) return (u8)m;
    int best = 0, bestD = 0x7FFFFFFF;
    for (int i = 0; i < 256; i++) {
        int d = ps2_ct16_dist(v, te.clut[ps2_clut_csm1_pos(i)]);
        if (d < bestD) { bestD = d; best = i; }
    }
    te.remap[v] = (u16)best;
    return (u8)best;
}
#endif

#if PS2_TEXTURE_PARTIAL_UPLOAD
// Sends texel rows [y0, y0 + rows) of `te` to GS VRAM. Both bounds are page-row
// aligned, so the source rows are contiguous in the CPU copy and the destination
// is a contiguous run of GS pages the base pointer can be aimed at directly.
//
// This is gsKit_texture_upload() with band dimensions substituted for the
// texture's own, CLUT handling included. The palette send is not redundant on
// the T8 path: it is the call that carries the TEXFLUSH, and without it the
// rasteriser can keep sampling texels the GS still holds from before the
// transfer.
static bool ps2_texture_send_band(PS2Tex& te, int y0, int rows) {
    const int width = (int)te.gs.Width;

#ifdef PS2_ENABLE_PSMT8
    const u8* src = (const u8*)te.cpuIdx;
    const u32 byteOffset = (u32)y0 * (u32)width;          // PSMT8: 1 byte/texel
#else
    const u8* src = (const u8*)te.cpuMem;
    const u32 byteOffset = (u32)y0 * (u32)width * 2u;     // PSMCT16: 2 bytes/texel
#endif
    if (src == nullptr)
        return false;

    const size_t need = (size_t)ps2_upload_packet_bytes_for(width, rows, (int)te.gs.PSM);
    if (!ps2_upload_park_take(need))
        return false;

    gsKit_texture_send((u32*)(src + byteOffset), width, rows,
                       te.gs.Vram + byteOffset, te.gs.PSM, te.gs.TBW,
                       GS_CLUT_TEXTURE);
#ifdef PS2_ENABLE_PSMT8
    gsKit_texture_send((u32*)te.clut, 16, 16, te.gs.VramClut,
                       te.gs.ClutPSM, 1, GS_CLUT_PALLETE);
#endif
    ps2_upload_park_return();
    return true;
}
#endif // PS2_TEXTURE_PARTIAL_UPLOAD

// Accepted and deliberately not acted on -- the GS already decides all three,
// and decides them better:
//
//   filter   the upload path pins GS_FILTER_NEAREST. Minecraft only asks for
//            linear on the few blurred textures, and a paletted PSMT8 texture
//            costs four CLUT lookups per filtered texel on this hardware.
//   wrap     ps2_select_clamp (Ps2ClipGuard.h) derives CLAMP vs REGION_REPEAT
//            from the UV bounds of each primitive, which is strictly more
//            information than a per-texture wrap mode carries -- it is what
//            keeps atlas tiles from bleeding into their neighbours.
//   mipmaps  no level beyond 0 is ever uploaded; see ps2_texture_upload_rgba.
//
// This is the same contract the glTexParameteri stub had. It is kept as a real
// entry point rather than dropped so the request stays visible at the call site
// and a backend that grows TEX1 control has somewhere to land.
void ps2_texture_set_parameters(bool linear, bool mipmaps, bool clamp) {
    (void)linear;
    (void)mipmaps;
    (void)clamp;
}

bool ps2_texture_upload_sub_rgba(unsigned int name, int level, int x, int y,
                                 int width, int height, const void* pixels) {
    if (level != 0 || !pixels) return false;
    const unsigned int id = name;
    if (id == 0 || id >= PS2_MAX_TEX || !s_tex[id].valid) return false;
    const int xoff = x;
    const int yoff = y;
    const int w = width;
    const int h = height;
    PS2Tex& te = s_tex[id];
    if (xoff < 0 || yoff < 0 || w <= 0 || h <= 0) return false;
    const std::int64_t endX = static_cast<std::int64_t>(xoff) + static_cast<std::int64_t>(w);
    const std::int64_t endY = static_cast<std::int64_t>(yoff) + static_cast<std::int64_t>(h);
    if (endX > static_cast<std::int64_t>(te.gs.Width) ||
        endY > static_cast<std::int64_t>(te.gs.Height)) return false;
    const u8* src = (const u8*)pixels;
#ifdef PS2_ENABLE_PSMT8
    if (!te.cpuIdx || !te.clut) return false;
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            int dp = (yoff+row)*(int)te.gs.Width + (xoff+col);
            int sp = row*w + col;
            te.cpuIdx[dp] = ps2_t8_remap_color(te, rgba_to_psmct16(src + sp * 4));
        }
    }
#else
    if (!te.cpuMem) return false;
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            int dp = (yoff+row)*(int)te.gs.Width + (xoff+col);
            int sp = row*w + col;
            te.cpuMem[dp] = rgba_to_psmct16(src + sp * 4);
        }
    }
#endif
    // Don't re-send per call: terrain.png gets several animated-tile updates per
    // frame (water/lava/fire), and each upload here used to DMA the full texture
    // again. Mark the GS page rows this rectangle covers; the next draw that
    // binds the texture sends those, once.
    te.dirtyUpload = true;
#if PS2_TEXTURE_PARTIAL_UPLOAD
    if (ps2_texture_supports_partial(te)) {
        int pageW = 0, pageH = 0;
        ps2_texture_page_geometry(te.gs.PSM, &pageW, &pageH);
        const int firstRow = yoff / pageH;
        const int lastRow = (yoff + h - 1) / pageH;
        for (int row = firstRow; row <= lastRow; row++)
            te.dirtyPageRows |= (1u << row);
    } else {
        te.dirtyPageRows = 0; // 0 while dirtyUpload is set means "all of it"
    }
#endif
    return true;
}

// Deferred flush point for the sub-image traffic above: animated tiles mark
// themselves dirty and one full send happens here instead of one per touched
// tile. It goes through the parked block so this per-frame ~64KB packet cannot
// walk the top of the heap; on a frame the heap cannot serve it the send is
// skipped and retried next frame (gsKit would otherwise build the packet through
// a NULL memalign result and TLB-miss).
GSTEXTURE* ps2_texture_resolve(unsigned int name) {
    if (name == 0 || name >= PS2_MAX_TEX)
        return nullptr;
    PS2Tex& te = s_tex[name];
    if (!te.valid || !te.uploaded)
        return nullptr;
    if (te.dirtyUpload) {
#if PS2_TEXTURE_PARTIAL_UPLOAD
        if (te.dirtyPageRows != 0 && ps2_texture_supports_partial(te)) {
            int pageW = 0, pageH = 0;
            ps2_texture_page_geometry(te.gs.PSM, &pageW, &pageH);
            const int pageRowCount = (int)(te.gs.Height / (u32)pageH);
            // Adjacent dirty page rows leave as one band, so the common case of
            // two neighbouring animated tiles still costs a single transfer.
            int row = 0;
            while (row < pageRowCount) {
                if ((te.dirtyPageRows & (1u << row)) == 0) {
                    row++;
                    continue;
                }
                int end = row;
                while (end + 1 < pageRowCount && (te.dirtyPageRows & (1u << (end + 1))) != 0)
                    end++;
                const int span = end - row + 1;
                if (!ps2_texture_send_band(te, row * pageH, span * pageH))
                    break; // heap too tight this frame; the bits stay set
                te.dirtyPageRows &= ~(((1u << span) - 1u) << row);
                row = end + 1;
            }
            if (te.dirtyPageRows == 0)
                te.dirtyUpload = false;
            return &te.gs;
        }
#endif
#ifdef PS2_ENABLE_PSMT8
        te.gs.Mem = (u32*)te.cpuIdx;
#else
        te.gs.Mem = (u32*)te.cpuMem;
#endif
        if (ps2_texture_upload_guarded(&te.gs)) {
            te.dirtyUpload = false;
            te.dirtyPageRows = 0;
        }
        te.gs.Mem = nullptr;
    }
    return &te.gs;
}

// System-RAM cost of the texture cache, split by what is holding it.
//
// Dynamic textures keep a CPU-side copy so TextureFX/custom animation sub-image
// updates can patch and re-send them. Static named textures release that mirror
// after a successful GS upload through TextureResidencyPolicy. Any mirror still
// present here is therefore dynamic or waiting for a deferred upload retry.
//
//   pix   = the pixel copy itself (cpuIdx on T8, cpuMem on CT16), now normally
//           limited to animated/dynamic textures and deferred upload retries.
//   clut  = 512 bytes per T8 texture. Noise, reported for completeness.
//   remap = 128 KB EACH (65536 u16). ps2_t8_remap_color allocates a full
//           CT16-value -> palette-index memo the first time glTexSubImage2D
//           touches a T8 texture, i.e. once per ANIMATED atlas (terrain, items).
//           It is a pure speed memo for a function that can also compute the
//           answer directly, so it is the one line here that can be reclaimed
//           outright rather than traded against image quality.
extern "C" void ps2_dbg_texture_ram_bytes(long* pixOut, long* clutOut, long* remapOut) {
    long pix = 0, clut = 0, remap = 0;
    for (int i = 0; i < PS2_MAX_TEX; i++) {
        const PS2Tex& t = s_tex[i];
        if (!t.valid)
            continue;
        const long npx = (long)t.gs.Width * (long)t.gs.Height;
#ifdef PS2_ENABLE_PSMT8
        if (t.cpuIdx) pix   += npx;               // 1 byte/px
        if (t.clut)   clut  += 256 * (long)sizeof(u16);
        if (t.remap)  remap += 65536 * (long)sizeof(u16);
#endif
        if (t.cpuMem) pix += npx * (long)sizeof(u16); // 2 bytes/px
    }
    if (pixOut)   *pixOut   = pix;
    if (clutOut)  *clutOut  = clut;
    if (remapOut) *remapOut = remap;
}

#endif // PS2_PLATFORM
