#ifdef PS2_PLATFORM

#include "ps2/render/Ps2Draw3D.h"
#include "ps2/render/Ps2ClipGuard.h"
#ifdef PS2_ENABLE_VU1_TRANSFORM
#include "ps2/render/Ps2Vu1Transform.h"
#endif
#include "ps2/render/Ps2RenderBackend.h"
#include "ps2/system/Ps2Scratchpad.h"

#include <gsInline.h>
#include <gsPrimitive.h>
#include <libvux.h>
#include <math.h>
#include <string.h>

#ifdef PS2_RENDER_STATS
#define PS2_FAST_DRAW_STAT(expr) do { expr; } while (0)
#else
#define PS2_FAST_DRAW_STAT(expr) do { } while (0)
#endif

#ifdef PS2_RENDER_STATS
// ---- Batching health and stage attribution ----
//
// PS2_OPT_BATCH_SIZE / PS2_OPT_STRIP_MAX_VERTS only buy anything if a packet
// actually accumulates. quads/stripFlush is the number to read: at 32 the batch
// is working, at ~1 every quad is carrying its own GIFtag + TEX0 pair and the
// batch size is decorative. clampAsk/clampSet says whether a clamp change is
// what broke it.
//
// The cycle counters attribute the per-triangle cost instead of guessing at it.
// They read COP0 Count (one tick per EE cycle, ~294MHz, wraps every ~14.6s --
// fine for per-period deltas). ps2sdk exposes the same register as cpu_ticks();
// this is the inlined form so the measurement is one instruction, not a call.
extern "C" {
long g_ps2_dbg_strip_flush = 0;
long g_ps2_dbg_batch_flush = 0;
long g_ps2_dbg_strip_quads = 0;
long g_ps2_dbg_clamp_ask   = 0;
long g_ps2_dbg_clamp_set   = 0;
long g_ps2_dbg_vu0_quads   = 0;
unsigned long g_ps2_dbg_cyc_xform   = 0;
unsigned long g_ps2_dbg_cyc_project = 0;
unsigned long g_ps2_dbg_cyc_emit    = 0;
#ifdef PS2_ENABLE_VU1_TRANSFORM
long g_ps2_dbg_vu1_batches = 0;
long g_ps2_dbg_vu1_vertices = 0;
unsigned long g_ps2_dbg_cyc_vu1_wait = 0;
#endif
}

static inline unsigned int ps2opt_ee_cycles(void)
{
	unsigned int c;
	__asm__ __volatile__("mfc0 %0, $9" : "=r"(c));
	return c;
}
#define PS2_CYC_BEGIN(v)      const unsigned int v = ps2opt_ee_cycles()
#define PS2_CYC_END(v, acc)   do { (acc) += ps2opt_ee_cycles() - (v); } while (0)
#else
#define PS2_CYC_BEGIN(v)      do { } while (0)
#define PS2_CYC_END(v, acc)   do { } while (0)
#endif

static inline void ps2_fast_queue_guard(GSGLOBAL* gs, int vertexCount) {
    if (!gs || !gs->Os_Queue)
        return;
    GSQUEUE* q = gs->Os_Queue;
    if (gs->CurQueue != q || !q->pool_cur || !q->pool_max[q->dbuf])
        return;
    unsigned int needed = 8192u + (unsigned int)vertexCount * 128u;
    // Pointer compare before the subtraction: past the end, an unsigned
    // difference wraps and permanently disables the guard. See the long note on
    // ps2_gs_queue_guard in Ps2GsQueue.cpp.
    const unsigned char* cursor = (const unsigned char*)q->pool_cur;
    const unsigned char* limit  = (const unsigned char*)q->pool_max[q->dbuf];
    if (cursor > limit)
        ps2_gs_queue_note_overflow((long)(cursor - limit));
    if (cursor < limit && (unsigned int)(limit - cursor) > needed)
        return;
    ps2_gs_queue_flush_oneshot();
}

// Number of triangles to accumulate before submitting a single GIF DMA packet.
// Reduces per-triangle gsKit call overhead significantly in the perspective path.
#define PS2_OPT_BATCH_SIZE 64

// Quad-as-tristrip emission: a fully-accepted quad goes to the GS as a
// 4-vertex triangle strip (first two vertices use the XYZ3 no-kick register,
// so consecutive quads chain in one packet) instead of 6 triangle-list
// vertices — one third less GIF data to build and DMA for all terrain.
// Set to 0 to fall back to the triangle-list path when debugging.
#define PS2_OPT_STRIP_QUADS 1
// 32 quads, not 64. A GSPRIMSTQPOINT is 48 bytes (rgbaq + stq + xyz2), so 64
// quads is 12KB of staging — larger than the EE's 8KB D-cache, and every quad is
// written here and then memcpy'd into the GS queue, so the buffer is walked
// twice. At 12KB the second walk is guaranteed to miss on everything the first
// walk brought in. 32 quads is 6KB, which fits, and the tag cost of flushing
// twice as often is 32 bytes per 32 quads (1 byte/quad) — nothing next to the
// per-quad flushing that clamp changes used to force (see Ps2GreedyMesh.cpp).
#define PS2_OPT_STRIP_MAX_VERTS (32 * 4)

#ifdef PS2_ENABLE_VU1_TRANSFORM
// Below this size the VIF setup/copy cost is larger than four VU0 transforms.
// Terrain batches are normally far above it; tiny entity/UI meshes stay VU0.
#define PS2_OPT_VU1_MIN_VERTS (8 * 4)
#endif

#if PS2_OPT_STRIP_QUADS
// Staged in the EE scratchpad: this buffer is written vertex by vertex and then
// read back whole by the memcpy in flushStrip(), which is exactly the access
// pattern an 8KB data cache handles worst. See Ps2Scratchpad.h for the region
// map and for why a CPU-only, main-thread-only buffer is safe to put there.
#if PS2_ENABLE_SCRATCHPAD
static_assert(sizeof(GSPRIMSTQPOINT) * PS2_OPT_STRIP_MAX_VERTS <= PS2_SPR_STRIP_BATCH_BYTES,
              "PS2_OPT_STRIP_MAX_VERTS outgrew its scratchpad region");
static GSPRIMSTQPOINT *const s_stripBatch =
	ps2_spr_region<GSPRIMSTQPOINT>(PS2_SPR_STRIP_BATCH_OFFSET);
#else
static GSPRIMSTQPOINT s_stripBatch[PS2_OPT_STRIP_MAX_VERTS] __attribute__((aligned(16)));
#endif

// One clamp selection per staged quad. Unlike the vertex batch this is tiny
// (32 * 12 bytes) and remains in ordinary EE memory. The selection is emitted
// as an inline A+D CLAMP write inside flushStrip(), so changing atlas cells no
// longer forces an early GIF packet flush.
static Ps2ClampSel s_stripClamp[PS2_OPT_STRIP_MAX_VERTS / 4];

static inline u64 ps2opt_clamp_reg(const Ps2ClampSel& sel)
{
	switch (sel.mode)
	{
	case PS2_CLAMPSEL_REPEAT:
		return GS_SETREG_CLAMP(GS_CMODE_REPEAT, GS_CMODE_REPEAT, 0, 0, 0, 0);
	case PS2_CLAMPSEL_REGION:
		return GS_SETREG_CLAMP(GS_CMODE_REGION_REPEAT, GS_CMODE_REGION_REPEAT,
			15, sel.ufix, 15, sel.vfix);
	default:
		return GS_SETREG_CLAMP(GS_CMODE_CLAMP, GS_CMODE_CLAMP, 0, 0, 0, 0);
	}
}

static inline int ps2opt_log2u(unsigned v)
{
	int r = 0;
	while ((1u << r) < v) r++;
	return r;
}

// TEX0 built the same way gsKit's textured prims do (TFX=0 modulate,
// TCC=PrimAlphaEnable, CLD=1 when a CLUT is present).
static inline u64 ps2opt_tex0(const GSGLOBAL* gs, const GSTEXTURE* t)
{
	const int tw = ps2opt_log2u(t->Width);
	const int th = ps2opt_log2u(t->Height);
	if (t->VramClut == 0)
		return GS_SETREG_TEX0(t->Vram / 256, t->TBW, t->PSM, tw, th,
			gs->PrimAlphaEnable, 0, 0, 0, 0, 0, 0);
	return GS_SETREG_TEX0(t->Vram / 256, t->TBW, t->PSM, tw, th,
		gs->PrimAlphaEnable, 0, t->VramClut / 256, t->ClutPSM,
		t->ClutStorageMode, 0, 1);
}
#endif

struct Ps2ProjVert {
    float x, y, q, w;
    int z;
};

// Attributes fetched only for triangles that survive culling/clipping.
struct Ps2EmitVert {
    float u, v;
    unsigned char r, g, bl, a;
};

// Copy column-major float[16] into VU_MATRIX (no transpose).
static inline VU_MATRIX ps2opt_mvp_to_vu(const float* m) {
    VU_MATRIX out;
    __builtin_memcpy(&out, m, 16 * sizeof(float));
    return out;
}

// Project a clip-space vertex. Caller guarantees the vertex is inside the
// near + guard-band planes, so q is bounded and the screen coords are inside
// the GS 12.4 representable window by construction.
static inline void ps2opt_project(const Ps2DepthMap& dm, bool forceNearZ,
                                  float cx, float cy, float cz, float cw,
                                  float hw, float hh, Ps2ProjVert& out) {
    float q = 1.0f / cw;
    out.q = q;
    out.w = cw;
    out.x = (cx * q + 1.0f) * hw;
    out.y = (-cy * q + 1.0f) * hh;
    if (forceNearZ) {
        out.z = PS2_GS_Z_MAX;
        return;
    }
    // Same 0..PS2_GS_Z_MAX range as the generic path — the signed PSMZ_16S
    // buffer turns anything above 0x7FFF into a negative depth. Terrain and
    // entities share one Z buffer, so both must use the same mapping; it lives
    // in ps2_gs_depth (Ps2ClipGuard.h) so there is only one of it.
    out.z = ps2_gs_depth(dm, cz, cw, q, (float)PS2_GS_Z_MAX);
}

static inline bool ps2opt_cull(bool cullFace, bool ccw, bool back,
                                float x0, float y0,
                                float x1, float y1,
                                float x2, float y2) {
    if (!cullFace) return false;
    float area = (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
    if (area > -0.0001f && area < 0.0001f) return false;
    bool front = ccw ? (area < 0.0f) : (area > 0.0f);
    return back ? !front : front;
}

bool ps2_draw_3d(const Ps2Draw3DState& state) {
    if (!state.gsGlobal || !state.vertices || !state.mvp)
        return false;
    if (state.count < (state.quads ? 4 : 3))
        return false;
    if (state.vertexSize < 3)
        return false;
    if (state.colorEnabled && (!state.colors || state.colorFloat))
        return false;

    const char* vbase = (const char*)state.vertices;
    const char* tbase = (const char*)state.texCoords;
    const char* cbase = (const char*)state.colors;

    const int vstride = state.vertexStride   ? state.vertexStride   : state.vertexSize * 4;
    const int tstride = state.texCoordStride ? state.texCoordStride : 2 * 4;
    const int cstride = state.colorStride    ? state.colorStride    : state.colorSize;

    const bool textured = state.texture && state.texCoords && state.texCoordEnabled;
	const bool colored  = state.colorEnabled && state.colors;
	const bool packedTerrain = state.packedTerrain;
	const bool useClampRuns = packedTerrain && state.tileAtlas;
	Ps2NativeClampRunCursor clampRunCursor(
		useClampRuns ? state.clampRuns : nullptr,
		useClampRuns ? state.clampRunCount : 0);
	const Ps2NativeClampRun* selectedClampRun = nullptr;
	Ps2ClampSel currentQuadClamp = { PS2_CLAMPSEL_NORMAL, -1, -1 };
	bool currentQuadClampValid = false;

	if (packedTerrain && (!state.quads || !textured || !colored))
		return false;

    const float hw = state.viewW * 0.5f;
    const float hh = state.viewH * 0.5f;
    const float fw = hw * 2.0f;
    const float fh = hh * 2.0f;
    const float gx = ps2_guard_clip_scale(fw);
    const float gy = ps2_guard_clip_scale(fh);
    const int ntri = state.count / 3;

    // Convert MVP once per draw call; ps2_vu0_xform3/4 load it into VU0
    // registers once per triangle/quad instead of once per vertex.
    VU_MATRIX vu_mvp __attribute__((aligned(16))) = ps2opt_mvp_to_vu(state.mvp);

#ifdef PS2_ENABLE_PERSPECTIVE_TEXTURES
    const bool perspBatch = textured;
#else
    const bool perspBatch = false;
#endif

    // Same reasoning as s_stripBatch above, and the same lifetime: this was
    // already a `static` array, so it was never re-entrant or per-call state.
    // gsKit consumes it with a CPU memcpy (verified in the linked ELF: the
    // prim_list entry point tail-calls memcpy from this pointer), never as a
    // DMA source, which is the condition Ps2Scratchpad.h requires.
#if PS2_ENABLE_SCRATCHPAD
    static_assert(sizeof(GSPRIMSTQPOINT) * PS2_OPT_BATCH_SIZE * 3 <= PS2_SPR_TRI_BATCH_BYTES,
                  "PS2_OPT_BATCH_SIZE outgrew its scratchpad region");
    GSPRIMSTQPOINT *const batch = ps2_spr_region<GSPRIMSTQPOINT>(PS2_SPR_TRI_BATCH_OFFSET);
#else
    static GSPRIMSTQPOINT batch[PS2_OPT_BATCH_SIZE * 3];
#endif
    int nbatch = 0;
    bool clampInitialized = false;
    Ps2ClampSel lastClamp = { -1, -1, -1 };
    const float texW = textured ? (float)state.texture->Width  : 1.0f;
    const float texH = textured ? (float)state.texture->Height : 1.0f;

#if PS2_OPT_STRIP_QUADS
    int nstrip = 0; // vertices in the strip batch (4 per quad)

    // One self-contained A+D packet straight into the gsKit queue:
    // GIFTAG (PRE=1 carries the TRISTRIP prim) + TEX0 + per-vertex
    // RGBAQ/ST/XYZ pairs. NLOOP is final, so no gsKit tag fix-up applies.
    auto flushStrip = [&]() {
        if (nstrip <= 0)
            return;
        PS2_CYC_BEGIN(cycFlush);
        PS2_FAST_DRAW_STAT(g_ps2_dbg_strip_flush++);
        const int quadCount = nstrip / 4;
        PS2_FAST_DRAW_STAT(g_ps2_dbg_strip_quads += quadCount);
        GSGLOBAL* gs = state.gsGlobal;

        // Atlas tile changes used to call flushStrip() before every CLAMP
        // update. A typical terrain tile contains only 10-20 visible quads, so
        // the 32-quad batch almost never filled. Encode CLAMP as A+D commands
        // inside this same packet instead. Independent quads already restart
        // the strip with XYZ3 on their first two vertices, therefore changing
        // CLAMP between quads is legal and preserves exact sampling semantics.
        int clampWrites = 0;
        Ps2ClampSel packetClamp = { -1, -1, -1 };
        for (int q = 0; q < quadCount; ++q)
        {
            if (q == 0 || s_stripClamp[q] != packetClamp)
            {
                packetClamp = s_stripClamp[q];
                ++clampWrites;
            }
        }

        const int qData = 1 + clampWrites + nstrip * 3;
        ps2_fast_queue_guard(gs, nstrip + clampWrites);
        u64* p = (u64*)gsKit_heap_alloc(gs, qData, qData * 16, GIF_AD);
        if (p == nullptr) {
            // Same contract ps2_gs_write_reg follows: drop the packet rather
            // than dereference the result. The staged quads are lost, which is
            // one frame of missing terrain -- writing through null is a
            // TLB-miss storm the console does not come back from.
            nstrip = 0;
            PS2_CYC_END(cycFlush, g_ps2_dbg_cyc_emit);
            return;
        }
        const u64 prim = GS_SETREG_PRIM(GS_PRIM_PRIM_TRISTRIP,
            state.render.smoothShading ? 1 : 0, 1,
            gs->PrimFogEnable, gs->PrimAlphaEnable, gs->PrimAAEnable,
            0, gs->PrimContext, 0);
        *p++ = GIF_TAG(qData, 1, 1, prim, 0, 1);
        *p++ = 0x0E; // REGS descriptor: A+D
        *p++ = ps2opt_tex0(gs, state.texture);
        *p++ = (u64)(GS_TEX0_1 + gs->PrimContext);

        packetClamp = Ps2ClampSel{ -1, -1, -1 };
        for (int q = 0; q < quadCount; ++q)
        {
            const Ps2ClampSel& sel = s_stripClamp[q];
            if (q == 0 || sel != packetClamp)
            {
                packetClamp = sel;
                *p++ = ps2opt_clamp_reg(sel);
                *p++ = (u64)(GS_CLAMP_1 + gs->PrimContext);
                PS2_FAST_DRAW_STAT(g_ps2_dbg_clamp_set++);
            }
            memcpy(p, &s_stripBatch[q * 4], 4 * sizeof(GSPRIMSTQPOINT));
            p += (4 * sizeof(GSPRIMSTQPOINT)) / sizeof(u64);
        }

        // Keep ps2_apply_clamp_sel's CPU-side cache coherent with the final GS
        // state produced by the inline packet. Usually this is a no-op; when it
        // is not, the queued write follows this packet and leaves both views in
        // agreement for generic triangle/entity draws.
        if (state.applyClampSel && packetClamp.mode >= 0)
            state.applyClampSel(packetClamp.mode, packetClamp.ufix, packetClamp.vfix);
        if (packetClamp.mode >= 0)
        {
            lastClamp = packetClamp;
            clampInitialized = true;
        }

        nstrip = 0;
        PS2_CYC_END(cycFlush, g_ps2_dbg_cyc_emit);
    };
#endif

    auto flushBatch = [&]() {
        if (nbatch > 0) {
            PS2_CYC_BEGIN(cycFlush);
            PS2_FAST_DRAW_STAT(g_ps2_dbg_batch_flush++);
            ps2_fast_queue_guard(state.gsGlobal, nbatch * 3);
            gsKit_prim_list_triangle_goraud_texture_stq_3d(
                state.gsGlobal, state.texture, nbatch * 3, batch);
            nbatch = 0;
            PS2_CYC_END(cycFlush, g_ps2_dbg_cyc_emit);
        }
    };

    // Apply one resolved clamp selection and flush only when it changes. Packed
    // terrain supplies it from tile runs; meshes without metadata keep using
    // the UV selector through ensureClamp below. One implementation serves both
    // batch and the quad strip batch — this logic existed twice, verbatim, and
    // neither copy reproduced the plain-REPEAT branch of the real function.
    auto ensureClampSel = [&](const Ps2ClampSel& sel) {
        if (!state.applyClampSel)
            return;
        PS2_FAST_DRAW_STAT(g_ps2_dbg_clamp_ask++);
        if (clampInitialized && sel == lastClamp)
            return;
        PS2_FAST_DRAW_STAT(g_ps2_dbg_clamp_set++);
        flushBatch();
#if PS2_OPT_STRIP_QUADS
        flushStrip(); // strip packets share the clamp state
#endif
        state.applyClampSel(sel.mode, sel.ufix, sel.vfix);
        lastClamp = sel;
        clampInitialized = true;
    };

    auto ensureClamp = [&](float minU, float minV, float maxU, float maxV) {
        ensureClampSel(ps2_select_clamp(texW, texH, state.ortho, state.tileAtlas,
                                       minU, minV, maxU, maxV));
    };

	// Packed terrain is physically stored in GS strip order (1,2,0,3), while
	// the clipping code below reasons about the source quad as (0,1,2,3).
	// Translate logical corners at the fetch boundary so every later decision
	// remains shared with ordinary meshes.
	auto packedIndex = [&](int idx) {
		static const int inverseStripOrder[4] = { 2, 0, 1, 3 };
		return (idx & ~3) + inverseStripOrder[idx & 3];
	};

	auto fetchEmit = [&](int idx, Ps2EmitVert& ev) {
		const int sourceIndex = packedTerrain ? packedIndex(idx) : idx;
        if (textured) {
			if (packedTerrain) {
				const short* t = reinterpret_cast<const short*>(tbase + sourceIndex * tstride);
				ev.u = (float)t[0] * (1.0f / 4096.0f);
				ev.v = (float)t[1] * (1.0f / 4096.0f);
			} else {
				const float* t = (const float*)(tbase + sourceIndex * tstride);
				ev.u = t[0]; ev.v = t[1];
			}
        } else {
            ev.u = ev.v = 0.0f;
        }
        if (colored) {
			const unsigned char* c = (const unsigned char*)cbase + sourceIndex * cstride;
			if (packedTerrain) {
				// Ps2TerrainMesh already stores colors at GS-native 0..128 scale.
				// Keep them there instead of round-tripping through 0..255 and
				// back (modColor/fogColorScale keep the rest of the pipeline
				// consistent with this scale for terrain).
				ev.r = c[0];
				ev.g = c[1];
				ev.bl = c[2];
				ev.a = (unsigned char)((unsigned int)c[3] * 2u);
			} else {
				ev.r = c[0]; ev.g = c[1]; ev.bl = c[2];
				ev.a = state.colorSize >= 4 ? c[3] : 255;
			}
        } else {
            ev.r = state.render.flatR; ev.g = state.render.flatG;
            ev.bl = state.render.flatB; ev.a = state.render.flatA;
        }
		if (state.render.lightVertex)
			state.render.lightVertex(sourceIndex, ev.r, ev.g, ev.bl);
    };

    Ps2ProjVert pv[3];
    Ps2EmitVert ev[3];
    ClipVert poly[PS2_CLIP_MAX_POLY];

    // Terrain colors stay in their native GS 0..128 scale end to end (see
    // fetchEmit and modColor below), so the fog blend basis must match.
    const float fogColorScale = packedTerrain ? 128.0f : 255.0f;

    auto applyFog = [&](Ps2EmitVert& e, float w) {
        if (!state.render.fogEnabled || w <= 0.0f) return;
        const float f = ps2_render_fog_factor(state.render, w);
        const float inv = 1.0f - f;
        e.r  = (unsigned char)(f * e.r  + inv * state.render.fogR * fogColorScale);
        e.g  = (unsigned char)(f * e.g  + inv * state.render.fogG * fogColorScale);
        e.bl = (unsigned char)(f * e.bl + inv * state.render.fogB * fogColorScale);
    };

    // GS texture modulation is 0..128, not 0..255. Terrain already stores
    // colors at GS scale (fetchEmit copies them through unchanged), so only
    // the conventional 0..255 sources need PS2_TEXCOL's rescale.
    auto modColor = [&](unsigned char v) -> int {
        return packedTerrain ? v : PS2_TEXCOL(v);
    };

    // Offscreen/backface test + emit of one projected triangle (pv/ev filled
    // by the caller). Perspective-textured draws go through the STQ batch;
    // everything else is a per-triangle gsKit prim.
    auto emitTri = [&]() {
        if (ps2_tri_offscreen(pv[0].x, pv[0].y, pv[1].x, pv[1].y,
                              pv[2].x, pv[2].y, fw, fh)) {
            PS2_FAST_DRAW_STAT(if (state.debugOffscreen) (*state.debugOffscreen)++);
            return;
        }
        if (ps2opt_cull(state.render.cullFace, state.render.frontFaceCCW, state.render.cullBackFace,
                         pv[0].x, pv[0].y, pv[1].x, pv[1].y, pv[2].x, pv[2].y)) {
            PS2_FAST_DRAW_STAT(if (state.debugBackface) (*state.debugBackface)++);
            return;
        }
        PS2_FAST_DRAW_STAT(if (state.debugPrims) (*state.debugPrims)++);

        if (!state.render.smoothShading) {
            for (int i = 0; i < 2; ++i) {
                ev[i].r = ev[2].r; ev[i].g = ev[2].g;
                ev[i].bl = ev[2].bl; ev[i].a = ev[2].a;
            }
        }
        applyFog(ev[0], pv[0].w); applyFog(ev[1], pv[1].w); applyFog(ev[2], pv[2].w);

        float u0 = ev[0].u*texW, v0 = ev[0].v*texH;
        float u1 = ev[1].u*texW, v1 = ev[1].v*texH;
        float u2 = ev[2].u*texW, v2 = ev[2].v*texH;

        if (textured && currentQuadClampValid)
            ensureClampSel(currentQuadClamp);

        if (perspBatch) {
            if (!currentQuadClampValid) {
                float minU, minV, maxU, maxV;
                ps2_uv_bounds3(u0, v0, u1, v1, u2, v2, minU, minV, maxU, maxV);
                ensureClamp(minU, minV, maxU, maxV);
            }

            int b = nbatch * 3;
            for (int i = 0; i < 3; i++) {
                float q = pv[i].q;
                // GS modulate basis is 128, not 255 — modColor rescales
                // conventional 0..255 colors (see Ps2ClipGuard.h) and passes
                // terrain's already-GS-scale colors through unchanged.
                batch[b + i].rgbaq = color_to_RGBAQ(modColor(ev[i].r), modColor(ev[i].g),
                                                    modColor(ev[i].bl), (u8)(ev[i].a >> 1), q);
                batch[b + i].stq  = vertex_to_STQ(ev[i].u * q, ev[i].v * q);
                batch[b + i].xyz2 = vertex_to_XYZ2(state.gsGlobal, pv[i].x, pv[i].y, pv[i].z);
            }
            nbatch++;
            if (nbatch == PS2_OPT_BATCH_SIZE)
                flushBatch();
            return;
        }

        if (textured) {
            // Modulate basis is 128, not 255: modColor rescales conventional
            // 0..255 colors (see Ps2ClipGuard.h) so textures are not drawn
            // ~2x too bright, and passes terrain's GS-scale colors through.
            u64 c0 = GS_SETREG_RGBAQ(modColor(ev[0].r), modColor(ev[0].g), modColor(ev[0].bl), (u8)(ev[0].a >> 1), 0);
            u64 c1 = GS_SETREG_RGBAQ(modColor(ev[1].r), modColor(ev[1].g), modColor(ev[1].bl), (u8)(ev[1].a >> 1), 0);
            u64 c2 = GS_SETREG_RGBAQ(modColor(ev[2].r), modColor(ev[2].g), modColor(ev[2].bl), (u8)(ev[2].a >> 1), 0);
            if (!currentQuadClampValid && state.setTextureClampForUv)
                state.setTextureClampForUv(state.texture, u0, v0, u1, v1, u2, v2);
            ps2_fast_queue_guard(state.gsGlobal, 3);
            gsKit_prim_triangle_goraud_texture_3d(state.gsGlobal, state.texture,
                pv[0].x, pv[0].y, pv[0].z, u0, v0,
                pv[1].x, pv[1].y, pv[1].z, u1, v1,
                pv[2].x, pv[2].y, pv[2].z, u2, v2,
                c0, c1, c2);
        } else {
            // Untextured gouraud: vertex color goes straight to the frame
            // buffer, full 0-255 range — no halving.
            u64 c0 = GS_SETREG_RGBAQ(ev[0].r, ev[0].g, ev[0].bl, (u8)(ev[0].a >> 1), 0);
            u64 c1 = GS_SETREG_RGBAQ(ev[1].r, ev[1].g, ev[1].bl, (u8)(ev[1].a >> 1), 0);
            u64 c2 = GS_SETREG_RGBAQ(ev[2].r, ev[2].g, ev[2].bl, (u8)(ev[2].a >> 1), 0);
            ps2_fast_queue_guard(state.gsGlobal, 3);
            gsKit_prim_triangle_gouraud_3d(state.gsGlobal,
                pv[0].x, pv[0].y, pv[0].z,
                pv[1].x, pv[1].y, pv[1].z,
                pv[2].x, pv[2].y, pv[2].z,
                c0, c1, c2);
        }
    };

    // Clip one straddling triangle (given as 3 ClipVerts with attributes) in
    // homogeneous space, fan-triangulate, project and emit the pieces.
    auto clipAndEmit = [&](const ClipVert* t0, const ClipVert* t1, const ClipVert* t2, int mask) {
        poly[0] = *t0; poly[1] = *t1; poly[2] = *t2;
        int n = ps2_clip_poly_guard(poly, 3, mask, gx, gy);
        if (n == 0) {
            PS2_FAST_DRAW_STAT(if (state.debugClipped) (*state.debugClipped)++);
            return;
        }
        for (int k = 1; k + 1 < n; k++) {
            const ClipVert* tri3[3] = { &poly[0], &poly[k], &poly[k + 1] };
            for (int i = 0; i < 3; i++) {
                ps2opt_project(state.depth, state.forceNearZ, tri3[i]->cx, tri3[i]->cy, tri3[i]->cz, tri3[i]->cw, hw, hh, pv[i]);
                ev[i].u = tri3[i]->u;  ev[i].v = tri3[i]->v;
                ev[i].r = tri3[i]->r;  ev[i].g = tri3[i]->g;  // fog applied in emitTri
                ev[i].bl = tri3[i]->bl; ev[i].a = tri3[i]->a;
            }
            emitTri();
        }
    };

    // ---- Quad-aware main loop ----
    //
    // The Tesselator expands every quad to [v0,v1,v2, v0,v2,v3] (triangles
    // (0,1,2) and (0,2,3)). Nearly all world/entity geometry is quads, so
    // detecting the pattern lets us transform and project 4 unique vertices
    // instead of 6 — a third less VU0/divide work on the hottest path.
    VU_VECTOR uin[4] __attribute__((aligned(16)));
    VU_VECTOR uclip[4] __attribute__((aligned(16)));
    int uoc[4];
    Ps2ProjVert upr[4];
    bool uprValid[4];
    Ps2EmitVert uev[4];
    bool uevValid[4];
    int usrc[4];

    auto loadVert = [&](int idx, VU_VECTOR& dst) {
		if (packedTerrain) {
			const int sourceIndex = packedIndex(idx);
			const short* v = reinterpret_cast<const short*>(vbase + sourceIndex * vstride);
			dst.x = (float)v[0] * (1.0f / 1024.0f) + 8.0f;
			dst.y = (float)v[1] * (1.0f / 1024.0f) + 8.0f;
			dst.z = (float)v[2] * (1.0f / 1024.0f) + 8.0f;
			dst.w = 1.0f;
			return;
		}
		const float* v = (const float*)(vbase + idx * vstride);
		dst.x = v[0]; dst.y = v[1]; dst.z = v[2]; dst.w = 1.0f;
    };

    auto samePos = [&](int ia, int ib) -> bool {
        const float* a = (const float*)(vbase + ia * vstride);
        const float* b = (const float*)(vbase + ib * vstride);
        return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
    };

    // Emit one triangle of a quad given unique-vertex cache slots (lazy
    // projection + lazy attribute fetch, both shared between the two tris).
    auto emitQuadTri = [&](int a, int b, int c) {
        if (uoc[a] & uoc[b] & uoc[c]) {
            PS2_FAST_DRAW_STAT(if (state.debugClipped) (*state.debugClipped)++);
            return;
        }
        int mask = uoc[a] | uoc[b] | uoc[c];
        if (mask == 0) {
            const int idx[3] = { a, b, c };
            for (int i = 0; i < 3; i++) {
                int u = idx[i];
                if (!uprValid[u]) {
                    ps2opt_project(state.depth, state.forceNearZ, uclip[u].x, uclip[u].y, uclip[u].z, uclip[u].w, hw, hh, upr[u]);
                    uprValid[u] = true;
                }
                pv[i] = upr[u];
            }
            for (int i = 0; i < 3; i++) {
                int u = idx[i];
                if (!uevValid[u]) {
                    fetchEmit(usrc[u], uev[u]);
                    uevValid[u] = true;
                }
                ev[i] = uev[u];
            }
            emitTri();
        } else {
            ClipVert cv[3];
            const int idx[3] = { a, b, c };
            for (int i = 0; i < 3; i++) {
                int u = idx[i];
                cv[i].cx = uclip[u].x; cv[i].cy = uclip[u].y;
                cv[i].cz = uclip[u].z; cv[i].cw = uclip[u].w;
                if (!uevValid[u]) {
                    fetchEmit(usrc[u], uev[u]);
                    uevValid[u] = true;
                }
                cv[i].u = uev[u].u; cv[i].v = uev[u].v;
                cv[i].r = uev[u].r; cv[i].g = uev[u].g;
                cv[i].bl = uev[u].bl; cv[i].a = uev[u].a;
            }
            clipAndEmit(&cv[0], &cv[1], &cv[2], mask);
        }
    };

#if PS2_OPT_STRIP_QUADS
    // Whole-quad strip emission: every vertex inside the guard band, the
    // (planar) quad faces the camera and is on screen -> 4 strip vertices.
    auto emitQuadStrip = [&]() -> bool {
        PS2_CYC_BEGIN(cycProj);
        for (int i = 0; i < 4; i++) {
            if (!uprValid[i]) {
                ps2opt_project(state.depth, state.forceNearZ, uclip[i].x, uclip[i].y, uclip[i].z, uclip[i].w, hw, hh, upr[i]);
                uprValid[i] = true;
            }
        }
        PS2_CYC_END(cycProj, g_ps2_dbg_cyc_project);
        // All four projected corners beyond the same screen edge -> reject.
        if ((upr[0].x < 0.0f && upr[1].x < 0.0f && upr[2].x < 0.0f && upr[3].x < 0.0f) ||
            (upr[0].y < 0.0f && upr[1].y < 0.0f && upr[2].y < 0.0f && upr[3].y < 0.0f) ||
            (upr[0].x > fw   && upr[1].x > fw   && upr[2].x > fw   && upr[3].x > fw) ||
            (upr[0].y > fh   && upr[1].y > fh   && upr[2].y > fh   && upr[3].y > fh)) {
            PS2_FAST_DRAW_STAT(if (state.debugClipped) (*state.debugClipped) += 2);
            return true;
        }
        // Planar quad: one backface test decides both triangles.
        if (ps2opt_cull(state.render.cullFace, state.render.frontFaceCCW, state.render.cullBackFace,
                        upr[0].x, upr[0].y, upr[1].x, upr[1].y, upr[2].x, upr[2].y)) {
            PS2_FAST_DRAW_STAT(if (state.debugBackface) (*state.debugBackface) += 2);
            return true;
        }

        for (int i = 0; i < 4; i++) {
            if (!uevValid[i]) {
                fetchEmit(usrc[i], uev[i]);
                uevValid[i] = true;
            }
        }

        // applyFog() already handles disabled fog and invalid w values.
        // Keeping the check there avoids duplicating the old linear-only
        // fogActive state now that LINEAR/EXP/EXP2 share one path.
        for (int i = 0; i < 4; i++)
            applyFog(uev[i], upr[i].w);

        Ps2ClampSel quadClamp = currentQuadClamp;
        if (currentQuadClampValid) {
            PS2_FAST_DRAW_STAT(g_ps2_dbg_clamp_ask++);
        } else {
            // UV bounds over the four corners, in texels. For strip quads the
            // selected state is staged beside the vertices instead of applied
            // immediately; flushStrip() inserts the CLAMP write between quads.
            float minU = uev[0].u, maxU = uev[0].u, minV = uev[0].v, maxV = uev[0].v;
            for (int i = 1; i < 4; i++) {
                if (uev[i].u < minU) minU = uev[i].u;
                if (uev[i].u > maxU) maxU = uev[i].u;
                if (uev[i].v < minV) minV = uev[i].v;
                if (uev[i].v > maxV) maxV = uev[i].v;
            }
            PS2_FAST_DRAW_STAT(g_ps2_dbg_clamp_ask++);
            quadClamp = ps2_select_clamp(texW, texH, state.ortho, state.tileAtlas,
                minU * texW, minV * texH, maxU * texW, maxV * texH);
        }

        if (nstrip + 4 > PS2_OPT_STRIP_MAX_VERTS)
            flushStrip();
        s_stripClamp[nstrip / 4] = quadClamp;

        // Strip order (1,2,0,3) yields triangles (1,2,0) and (2,0,3) — the
        // same two the list path draws. First two vertices use XYZ3 (no
        // drawing kick), so consecutive quads chain in one packet.
        PS2_CYC_BEGIN(cycPack);
        static const int order[4] = { 1, 2, 0, 3 };
        GSPRIMSTQPOINT* sp = &s_stripBatch[nstrip];
        for (int k = 0; k < 4; k++) {
            const int u = order[k];
            const float q = upr[u].q;
            sp[k].rgbaq = color_to_RGBAQ(modColor(uev[u].r), modColor(uev[u].g),
                                         modColor(uev[u].bl), (u8)(uev[u].a >> 1), q);
            sp[k].stq  = vertex_to_STQ(uev[u].u * q, uev[u].v * q);
            sp[k].xyz2 = vertex_to_XYZ2(state.gsGlobal, upr[u].x, upr[u].y, upr[u].z);
        }
        sp[0].xyz2.tag = GS_XYZ3;
        sp[1].xyz2.tag = GS_XYZ3;
        nstrip += 4;
        PS2_CYC_END(cycPack, g_ps2_dbg_cyc_emit);
        PS2_FAST_DRAW_STAT(if (state.debugPrims) (*state.debugPrims) += 2);
        return true;
    };
#endif

    // Clip-classify and emit the four clip-space vertices already stored in
    // uclip[]. Both VU0 and VU1 feed this exact path so projection, clipping,
    // fog, lighting, clamp selection and GIF output remain bit-for-bit shared.
    auto processClipQuad = [&]() {
        currentQuadClampValid = false;
        const Ps2NativeClampRun* clampRun = clampRunCursor.find(usrc[0]);
        if (clampRun != nullptr) {
            if (clampRun != selectedClampRun) {
                selectedClampRun = clampRun;
                currentQuadClamp.mode = PS2_CLAMPSEL_REGION;
                currentQuadClamp.ufix = (int)clampRun->tileX << 4;
                currentQuadClamp.vfix = (int)clampRun->tileY << 4;
            }
            currentQuadClampValid = true;
        }

        for (int i = 0; i < 4; i++) {
            // "Fully inside" drops the four guard-band planes but NEVER the near
            // test -- see ps2_clip_outcode_near in Ps2ClipGuard.h for why the
            // divide by w cannot be left unguarded.
            uoc[i] = state.fullyInside
                ? ps2_clip_outcode_near(uclip[i].z, uclip[i].w)
                : ps2_clip_outcode(uclip[i].x, uclip[i].y, uclip[i].z, uclip[i].w, gx, gy);
            uprValid[i] = uevValid[i] = false;
        }
#if PS2_OPT_STRIP_QUADS
        if (perspBatch && (uoc[0] | uoc[1] | uoc[2] | uoc[3]) == 0) {
            emitQuadStrip();
            return;
        }
#endif
        emitQuadTri(0, 1, 2);
        emitQuadTri(0, 2, 3);
    };

    // VU0 fallback for small draws, generic triangles and any VU1 setup error.
    auto processStagedQuad = [&]() {
        PS2_CYC_BEGIN(cycXform);
        for (int i = 0; i < 4; i++)
            loadVert(usrc[i], uin[i]);
        ps2_vu0_xform4(&vu_mvp, &uin[0], &uin[1], &uin[2], &uin[3],
                       &uclip[0], &uclip[1], &uclip[2], &uclip[3]);
        PS2_CYC_END(cycXform, g_ps2_dbg_cyc_xform);
        processClipQuad();
    };

    if (state.quads) {
        const int nquad = state.count / 4;
        int firstVu0Quad = 0;
        const bool sliced = state.slices != nullptr && state.sliceCount > 0;

#ifdef PS2_ENABLE_VU1_TRANSFORM
        // VU1 phase 1: offload homogeneous MVP transforms only. VIF1 receives
        // batches of XYZ1, VU1 writes clip-space XYZW into alternating banks,
        // and the EE consumes bank N while VU1 fills bank N+1. The existing
        // emitter remains authoritative for clipping and GS output.
		if (!packedTerrain && nquad * 4 >= PS2_OPT_VU1_MIN_VERTS &&
            ps2_vu1_transform_begin(state.mvp)) {
            const int maxBatchVerts = PS2_VU1_TRANSFORM_MAX_VERTICES & ~3;
            int currentQuad = 0;
            int currentBank = 0;
            int currentVerts = nquad * 4;
            if (currentVerts > maxBatchVerts)
                currentVerts = maxBatchVerts;

            if (ps2_vu1_transform_submit(state.vertices, vstride, state.first,
                                         currentVerts, currentBank)) {
                PS2_FAST_DRAW_STAT(g_ps2_dbg_vu1_batches++);
                PS2_FAST_DRAW_STAT(g_ps2_dbg_vu1_vertices += currentVerts);

                while (currentVerts > 0) {
                    PS2_CYC_BEGIN(cycVu1Wait);
                    const bool completed = ps2_vu1_transform_wait();
                    PS2_CYC_END(cycVu1Wait, g_ps2_dbg_cyc_vu1_wait);
                    if (!completed)
                        break;

                    const int currentQuads = currentVerts / 4;
                    const int nextQuad = currentQuad + currentQuads;
                    int nextVerts = (nquad - nextQuad) * 4;
                    if (nextVerts > maxBatchVerts)
                        nextVerts = maxBatchVerts;

                    bool nextSubmitted = false;
                    const int nextBank = currentBank ^ 1;
                    if (nextVerts > 0) {
                        nextSubmitted = ps2_vu1_transform_submit(
                            state.vertices, vstride,
                            state.first + nextQuad * 4,
                            nextVerts, nextBank);
                        if (nextSubmitted) {
                            PS2_FAST_DRAW_STAT(g_ps2_dbg_vu1_batches++);
                            PS2_FAST_DRAW_STAT(g_ps2_dbg_vu1_vertices += nextVerts);
                        }
                    }

                    // This loop intentionally runs after the next submit. VIF1
                    // and VU1 can therefore transform the next bank while the
                    // EE does projection/culling/attribute work for this one.
                    for (int q = 0; q < currentQuads; q++) {
                        const int absoluteQuad = currentQuad + q;
                        const int base = state.first + absoluteQuad * 4;
                        usrc[0] = base;
                        usrc[1] = base + 1;
                        usrc[2] = base + 2;
                        usrc[3] = base + 3;

                        for (int i = 0; i < 4; i++) {
                            Ps2Vu1ClipVertex clip;
                            ps2_vu1_transform_read(currentBank, q * 4 + i, clip);
                            uclip[i].x = clip.x;
                            uclip[i].y = clip.y;
                            uclip[i].z = clip.z;
                            uclip[i].w = clip.w;
                        }
                        processClipQuad();
                    }

                    firstVu0Quad = nextQuad;
                    if (nextVerts <= 0 || !nextSubmitted)
                        break;
                    currentQuad = nextQuad;
                    currentVerts = nextVerts;
                    currentBank = nextBank;
                }
            }
        }
#endif

        PS2_FAST_DRAW_STAT(g_ps2_dbg_vu0_quads += nquad - firstVu0Quad);

        // VU0 fallback batches: stage several quads, load the MVP once, then
        // consume the transformed results through the exact same clip/emission
        // path as VU1. Sixteen quads use ~2KB for input+output staging, still
        // modest on the EE stack, and halve batch-call/matrix-load overhead
        // versus the old 8-quad group.
        static const int kVu0BatchQuads = 16;
        VU_VECTOR vu0In[kVu0BatchQuads * 4] __attribute__((aligned(16)));
        VU_VECTOR vu0Out[kVu0BatchQuads * 4] __attribute__((aligned(16)));
        int vu0Base[kVu0BatchQuads];

        if (sliced) {
            // Zero-copy scatter path: walk the caller's slice list instead of
            // one contiguous [first, first+count) range, filling the same
            // batch arrays and sharing the same strip/clamp state across
            // slice boundaries so a fragmented tile run still batches like a
            // contiguous one. See the comment on Ps2Draw3DState::slices.
            int sliceIndex = 0;
            int quadInSlice = 0;
            while (sliceIndex < state.sliceCount) {
                const int firstSliceQuads = state.slices[sliceIndex].vertexCount / 4;
                if (firstSliceQuads <= 0 || quadInSlice >= firstSliceQuads) {
                    ++sliceIndex;
                    quadInSlice = 0;
                    continue;
                }

                int batchQuads = 0;
                int scanSlice = sliceIndex;
                int scanQuad = quadInSlice;
                while (batchQuads < kVu0BatchQuads && scanSlice < state.sliceCount) {
                    const Ps2NativeSlice& scanSl = state.slices[scanSlice];
                    const int scanSliceQuads = scanSl.vertexCount / 4;
                    if (scanQuad >= scanSliceQuads) {
                        ++scanSlice;
                        scanQuad = 0;
                        continue;
                    }
                    const int base = scanSl.firstVertex + scanQuad * 4;
                    vu0Base[batchQuads] = base;
                    for (int i = 0; i < 4; ++i)
                        loadVert(base + i, vu0In[batchQuads * 4 + i]);
                    ++batchQuads;
                    ++scanQuad;
                }
                if (batchQuads == 0)
                    break;

                PS2_CYC_BEGIN(cycVu0Batch);
                ps2_vu0_xform4_batch(&vu_mvp, vu0In, vu0Out, batchQuads * 4);
                PS2_CYC_END(cycVu0Batch, g_ps2_dbg_cyc_xform);

                for (int bq = 0; bq < batchQuads; ++bq) {
                    const int base = vu0Base[bq];
                    for (int i = 0; i < 4; ++i) {
                        usrc[i] = base + i;
                        uclip[i] = vu0Out[bq * 4 + i];
                    }
                    processClipQuad();
                }

                sliceIndex = scanSlice;
                quadInSlice = scanQuad;
            }
        } else {
            for (int q = firstVu0Quad; q < nquad; ) {
                int batchQuads = nquad - q;
                if (batchQuads > kVu0BatchQuads)
                    batchQuads = kVu0BatchQuads;

                for (int bq = 0; bq < batchQuads; ++bq) {
                    const int base = state.first + (q + bq) * 4;
                    vu0Base[bq] = base;
                    if (q + bq + 1 < nquad)
                        __builtin_prefetch(vbase + (base + 4) * vstride, 0, 1);
                    for (int i = 0; i < 4; ++i)
                        loadVert(base + i, vu0In[bq * 4 + i]);
                }

                PS2_CYC_BEGIN(cycVu0Batch);
                ps2_vu0_xform4_batch(&vu_mvp, vu0In, vu0Out, batchQuads * 4);
                PS2_CYC_END(cycVu0Batch, g_ps2_dbg_cyc_xform);

                for (int bq = 0; bq < batchQuads; ++bq) {
                    const int base = vu0Base[bq];
                    for (int i = 0; i < 4; ++i) {
                        usrc[i] = base + i;
                        uclip[i] = vu0Out[bq * 4 + i];
                    }
                    processClipQuad();
                }
                q += batchQuads;
            }
        }
        flushBatch();
#if PS2_OPT_STRIP_QUADS
        flushStrip();
#endif
        return true;
    }

    int tri = 0;
    while (tri < ntri) {
        const int base = state.first + tri * 3;

        if (tri + 1 < ntri)
            __builtin_prefetch(vbase + (state.first + (tri + 1) * 3) * vstride, 0, 1);

        // Quad pattern check: two triangles left and the Tesselator layout.
        if (tri + 1 < ntri && samePos(base, base + 3) && samePos(base + 2, base + 4)) {
            usrc[0] = base; usrc[1] = base + 1; usrc[2] = base + 2; usrc[3] = base + 5;
            processStagedQuad();
            tri += 2;
            continue;
        }

        // Generic triangle (entity models with non-quad layout, leftovers).
        for (int i = 0; i < 3; i++)
            loadVert(base + i, uin[i]);
        ps2_vu0_xform3(&vu_mvp, &uin[0], &uin[1], &uin[2],
                       &uclip[0], &uclip[1], &uclip[2]);

        int oc0, oc1, oc2;
        if (state.fullyInside) {
            // Near plane still tested; see the quad path above.
            oc0 = ps2_clip_outcode_near(uclip[0].z, uclip[0].w);
            oc1 = ps2_clip_outcode_near(uclip[1].z, uclip[1].w);
            oc2 = ps2_clip_outcode_near(uclip[2].z, uclip[2].w);
        } else {
            oc0 = ps2_clip_outcode(uclip[0].x, uclip[0].y, uclip[0].z, uclip[0].w, gx, gy);
            oc1 = ps2_clip_outcode(uclip[1].x, uclip[1].y, uclip[1].z, uclip[1].w, gx, gy);
            oc2 = ps2_clip_outcode(uclip[2].x, uclip[2].y, uclip[2].z, uclip[2].w, gx, gy);
        }
        if (oc0 & oc1 & oc2) {
            PS2_FAST_DRAW_STAT(if (state.debugClipped) (*state.debugClipped)++);
            tri++;
            continue;
        }

        int mask = oc0 | oc1 | oc2;
        if (mask == 0) {
            for (int i = 0; i < 3; i++)
                ps2opt_project(state.depth, state.forceNearZ, uclip[i].x, uclip[i].y, uclip[i].z, uclip[i].w, hw, hh, pv[i]);
            fetchEmit(base + 0, ev[0]);
            fetchEmit(base + 1, ev[1]);
            fetchEmit(base + 2, ev[2]);
            emitTri();
        } else {
            ClipVert cv[3];
            for (int i = 0; i < 3; i++) {
                cv[i].cx = uclip[i].x; cv[i].cy = uclip[i].y;
                cv[i].cz = uclip[i].z; cv[i].cw = uclip[i].w;
                Ps2EmitVert e;
                fetchEmit(base + i, e);
                cv[i].u = e.u; cv[i].v = e.v;
                cv[i].r = e.r; cv[i].g = e.g; cv[i].bl = e.bl; cv[i].a = e.a;
            }
            clipAndEmit(&cv[0], &cv[1], &cv[2], mask);
        }
        tri++;
    }

    flushBatch();
#if PS2_OPT_STRIP_QUADS
    flushStrip();
#endif
    return true;
}

#endif
