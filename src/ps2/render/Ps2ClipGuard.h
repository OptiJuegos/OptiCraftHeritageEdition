#pragma once

#if defined(PS2_PLATFORM)

#include <libvux.h>
#include <math.h>

#include "ps2/render/Ps2Tuning.h" // PS2_LINEAR_DEPTH

// Guard band and color helpers shared by gles_ps2.cpp and the fast draw paths.
// They live here because all PS2 render paths already include this file.
#ifndef PS2_GUARD_BAND_PX
#define PS2_GUARD_BAND_PX 256.0f
#endif

static inline float ps2_guard_clip_scale(float extent)
{
    return 1.0f + 2.0f * PS2_GUARD_BAND_PX / extent;
}

static inline float ps2_clamp_guard(float v, float extent)
{
    const float lo = -PS2_GUARD_BAND_PX;
    const float hi = extent + PS2_GUARD_BAND_PX;
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static inline bool ps2_tri_offscreen(float x0, float y0,
                                     float x1, float y1,
                                     float x2, float y2,
                                     float w, float h)
{
    if (x0 < 0.0f && x1 < 0.0f && x2 < 0.0f) return true;
    if (x0 > w    && x1 > w    && x2 > w)    return true;
    if (y0 < 0.0f && y1 < 0.0f && y2 < 0.0f) return true;
    if (y0 > h    && y1 > h    && y2 > h)    return true;
    return false;
}

static inline int ps2_texcol_scale(int c)
{
    if (c < 0) c = 0;
    if (c > 255) c = 255;
    // GS texture modulation uses 128 as the neutral value, not 255.
    return (c * 128 + 127) / 255;
}

#ifndef PS2_TEXCOL
#define PS2_TEXCOL(v) ps2_texcol_scale((int)(v))
#endif

#ifndef PS2_TEXCOL_GUI
#define PS2_TEXCOL_GUI(v) ps2_texcol_scale((int)(v))
#endif

// ---- Texture clamp selection ----
//
// Which CLAMP_1 setting a triangle/quad needs is a pure function of its UV
// bounding box and the texture size. It used to live inside
// ps2_set_texture_clamp_for_uv (which writes to DMA immediately), so the
// batching draw paths each carried their own hand-copied *prediction* of it to
// know when to flush before the state changed. Those copies had drifted: they
// only reproduced the REGION_REPEAT branch and silently ignored the plain
// REPEAT one, so a batch could be flushed under a clamp mode it was not built
// with. Deciding here and applying separately keeps every path on one rule.
enum
{
    PS2_CLAMPSEL_NORMAL = 0, // GS_CMODE_CLAMP
    PS2_CLAMPSEL_REPEAT = 1, // GS_CMODE_REPEAT
    PS2_CLAMPSEL_REGION = 2  // GS_CMODE_REGION_REPEAT, with ufix/vfix
};

struct Ps2ClampSel
{ 
    int mode;
    int ufix, vfix; // only meaningful for PS2_CLAMPSEL_REGION

    bool operator==(const Ps2ClampSel& o) const
    {
        return mode == o.mode && ufix == o.ufix && vfix == o.vfix;
    }
    bool operator!=(const Ps2ClampSel& o) const { return !(*this == o); }
};

// UVs are in texels (u * texW), not normalised.
static inline Ps2ClampSel ps2_select_clamp(float texW, float texH, bool ortho,
                                           bool atlas,
                                           float minU, float minV,
                                           float maxU, float maxV)
{
    Ps2ClampSel sel;
    sel.ufix = sel.vfix = -1;

    // Simple full-texture repeat: UV exceeds texture bounds (e.g. the tiled dirt
    // menu background). Only explicitly marked tile atlases use REGION_REPEAT
    // below. Texture dimensions cannot identify an atlas: the main-menu
    // panorama faces are also 256x256 full-image textures.
    if (!atlas && (maxU > texW + 0.01f || maxV > texH + 0.01f)) {
        sel.mode = PS2_CLAMPSEL_REPEAT;
        return sel;
    }

    // Every world-space atlas face goes through REGION_REPEAT, not just the ones
    // wide enough to need tiling.
    //
    // REGION_REPEAT with MINU=15/MAXU=ufix computes u_final = (u & 15) | ufix,
    // which confines the sampled texel to the quad's own 16x16 cell. That is
    // required for a merged (greedy) face, which has to tile its cell -- but it
    // is also what makes a SINGLE-cell face safe, because plain CLAMP only limits
    // u to the whole 256x256 atlas and lets a boundary texel spill into the
    // neighbouring tile. On the terrain atlas cell 0 is grass-top and cell 1 is
    // stone, so that spill paints light grey speckles across grass -- visible on
    // any large flat surface at a grazing angle, where the perspective divide is
    // least precise.
    //
    // It was always latent; it became consistently visible once the greedy mesher
    // started grouping unmerged quads together (Ps2GreedyMesh.cpp), because
    // before that the clamp changed so often that many 1x1 quads happened to be
    // drawn while a neighbour's REGION state was still programmed, which masked
    // the bleed by accident.
    //
    // Correct for both cases: for a face inside one cell u spans [j, j+15.99], so
    // (u & 15) | j reproduces j..j+15 exactly. This assumes no single face's UVs
    // legitimately straddle two atlas cells -- an assumption the span-gated
    // version above already made, since it would have wrapped such a face too.
    if (!ortho && atlas) {
        // Truncation, not floorf. Now that every atlas face takes this branch
        // this runs twice for every textured quad in the frame (~830/frame
        // measured), and floorf is a libm call on the EE. minU/minV are texel
        // coordinates inside a 256x256 atlas and cannot be negative here, and for
        // non-negative input truncation IS floor.
        int ufix = ((int)minU) & ~15;
        int vfix = ((int)minV) & ~15;
        if (ufix < 0) ufix = 0; if (ufix > 240) ufix = 240;
        if (vfix < 0) vfix = 0; if (vfix > 240) vfix = 240;
        sel.mode = PS2_CLAMPSEL_REGION;
        sel.ufix = ufix;
        sel.vfix = vfix;
        return sel;
    }

    sel.mode = PS2_CLAMPSEL_NORMAL;
    return sel;
}

// ---- GS depth ----
//
// The generic path and both fast-draw backends write into the same PSMZ_16
// buffer, so the eye-space -> GS Z mapping has to be identical in all three.
// It lives here so there is one copy of it.
//
// The port shipped the textbook GL mapping, d = (1 - z_ndc)/2 scaled to
// 0..PS2_GS_Z_MAX. That is correct, and on a 24-bit desktop depth buffer it is
// also fine, but a 16-bit depth buffer still has limited precision and z_ndc is hyperbolic: with
// glFrustum(near=0.05, far=64) the resolution is
//
//     dZ/dt = ZMAX * near * far / ((far - near) * t^2)
//
// i.e. ~1640/t^2 GS units per block. That is thousands of levels in the first
// metre and then 6 levels per block at 16 blocks, 1.6 at 32 — the whole visible
// world past ~20 blocks lands on a handful of integers. Surfaces that are
// genuinely a fraction of a block apart, like a water top (y+0.875) and the
// grass rim beside it, then quantise to the SAME Z; with GEQUAL both pass and
// whichever draws last wins, so the shoreline flickers in and out as the camera
// turns. Note the far plane barely matters here (near*far/(far-near) ~= near
// once far >> near) — precision is set almost entirely by the near plane.
//
// PS2_LINEAR_DEPTH replaces it with Z proportional to eye distance (a W buffer,
// which is what the GS is happiest with): a flat ~512 GS units per block at
// every distance instead of 1640/t^2.
//
// The trade is that the GS interpolates Z linearly in *screen* space, and
// linear-in-distance is not correct under that interpolation, so a triangle
// bows inward by at most span^2/(4*t) blocks for a triangle spanning `span` of
// depth at distance t. Minecraft terrain is per-block quads (span <= ~1.4), so
// that is 0.5/t blocks — an order of magnitude below the quantisation error it
// removes at every distance where the error mattered. Greedy meshing merges
// faces into much larger quads and would widen `span`, so PS2_ENABLE_GREEDY_MESH
// and this knob want measuring together.
struct Ps2DepthMap
{
    float zFar;   // far plane, eye units
    float scale;  // zRange / (zFar - zNear), PS2_LINEAR_DEPTH only
    float qScale; // hyperbolic mapping as z = qScale * (1/t) + qBias
    float qBias;  // see PS2_DEPTH_NEAR_BOOST for where the pair comes from
};

// cw is the clip-space w, which for any glFrustum-derived projection is the
// eye-space distance; q is its reciprocal, already computed by the caller for
// the perspective divide.
static inline int ps2_gs_depth(const Ps2DepthMap& dm, float cz, float cw,
                               float q, float zRange)
{
#if PS2_LINEAR_DEPTH
    (void)cz; (void)q;
    float z = (dm.zFar - cw) * dm.scale;
#else
    // Equivalent to the textbook (1 - cz*q) * 0.5 * zRange when the mapping's
    // near plane is the projection's own, but expressed in terms of q alone so
    // PS2_DEPTH_NEAR_BOOST can move it. Dropping cz is what decouples the two.
    (void)cz; (void)cw;
    float z = dm.qScale * q + dm.qBias;
#endif
    if (z < 0.0f) z = 0.0f;
    if (z > zRange) z = zRange;
    return (int)z;
}

// UV bounding box of a triangle, in texels.
static inline void ps2_uv_bounds3(float u0, float v0, float u1, float v1,
                                  float u2, float v2,
                                  float& minU, float& minV,
                                  float& maxU, float& maxV)
{
    minU = maxU = u0;
    minV = maxV = v0;
    if (u1 < minU) minU = u1; if (u1 > maxU) maxU = u1;
    if (u2 < minU) minU = u2; if (u2 > maxU) maxU = u2;
    if (v1 < minV) minV = v1; if (v1 > maxV) maxV = v1;
    if (v2 < minV) minV = v2; if (v2 > maxV) maxV = v2;
}

// ---- VU0 macro-mode vertex transform ----
//
// These replace N calls to libvux's Vu0ApplyMatrix. That function is worth
// reading in disassembly before changing anything here:
//
//     Vu0ApplyMatrix:
//       lqc2 vf20,0(a1)      <- the vertex
//       lqc2 vf16,0(a0)      <- matrix column 0
//       lqc2 vf17,16(a0)     <- column 1
//       lqc2 vf18,32(a0)     <- column 2
//       lqc2 vf19,48(a0)     <- column 3
//       vmulax.xyzw   ACC, vf16, vf20x
//       vmadday.xyzw  ACC, vf17, vf20y
//       vmaddaz.xyzw  ACC, vf18, vf20z
//       vmaddw.xyzw   vf20, vf19, vf20w
//       jr ra
//       sqc2 vf20,0(a2)
//
// Two costs, both paid per vertex. First, it is a real out-of-line call that
// reloads all 64 bytes of the matrix every time -- so a quad loaded the same
// matrix four times, 192 bytes of pure waste, plus four call/return pipeline
// breaks. Second, and worse: those four COP2 ops are a serial dependency chain
// through the accumulator, each with ~4 cycles of latency on the R5900, and
// there is only ONE ACC on VU0. Nothing can be interleaved into those stalls --
// not even another vertex -- so the chain runs at roughly its latency, ~16
// cycles of which ~4 do work.
//
// The fix is to drop the accumulator entirely and express the transform as
// independent vmul/vadd, which gives every vertex its own registers and lets
// four vertices be interleaved round-robin. Per vertex:
//
//     O = c0*V.x;  T = c1*V.y;  O += T
//                  T = c2*V.z;  O += T
//                  T = c3*V.w;  O += T
//
// That is 7 ops instead of 4, but issued as 7 rounds of 4 independent ops the
// pipeline never stalls: by the time a round comes back to vertex 0, four issue
// slots have passed, which is exactly the latency being covered. The matrix is
// loaded once for all four vertices.
//
// Register use is hardcoded, exactly as libvux does it: GCC's R5900 backend does
// not allocate COP2 registers for ordinary C, so vf16-vf31 are free to use. Each
// block is self-contained -- it loads the matrix and the vertices, computes, and
// stores -- so nothing depends on a vf register surviving between statements, and
// a gsKit or memcpy call that happens to touch vf regs cannot interfere.
//
//   vf16-vf19 = matrix columns 0..3
//   vf20-vf23 = input vertices
//   vf24-vf27 = accumulating results
//   vf28-vf31 = per-vertex temporaries
//
// w is not assumed to be 1 even though every current caller sets it so; the
// vmulw round keeps these general for a projective input.

static inline void ps2_vu0_xform3(const VU_MATRIX* m,
                                  const VU_VECTOR* in0,
                                  const VU_VECTOR* in1,
                                  const VU_VECTOR* in2,
                                  VU_VECTOR* out0,
                                  VU_VECTOR* out1,
                                  VU_VECTOR* out2)
{
    __asm__ __volatile__ (
        "lqc2       $vf16, 0x00(%[m])       \n"
        "lqc2       $vf17, 0x10(%[m])       \n"
        "lqc2       $vf18, 0x20(%[m])       \n"
        "lqc2       $vf19, 0x30(%[m])       \n"
        "lqc2       $vf20, 0x00(%[i0])      \n"
        "lqc2       $vf21, 0x00(%[i1])      \n"
        "lqc2       $vf22, 0x00(%[i2])      \n"
        "vmulx.xyzw $vf24, $vf16, $vf20x    \n"
        "vmulx.xyzw $vf25, $vf16, $vf21x    \n"
        "vmulx.xyzw $vf26, $vf16, $vf22x    \n"
        "vmuly.xyzw $vf28, $vf17, $vf20y    \n"
        "vmuly.xyzw $vf29, $vf17, $vf21y    \n"
        "vmuly.xyzw $vf30, $vf17, $vf22y    \n"
        "vadd.xyzw  $vf24, $vf24, $vf28     \n"
        "vadd.xyzw  $vf25, $vf25, $vf29     \n"
        "vadd.xyzw  $vf26, $vf26, $vf30     \n"
        "vmulz.xyzw $vf28, $vf18, $vf20z    \n"
        "vmulz.xyzw $vf29, $vf18, $vf21z    \n"
        "vmulz.xyzw $vf30, $vf18, $vf22z    \n"
        "vadd.xyzw  $vf24, $vf24, $vf28     \n"
        "vadd.xyzw  $vf25, $vf25, $vf29     \n"
        "vadd.xyzw  $vf26, $vf26, $vf30     \n"
        "vmulw.xyzw $vf28, $vf19, $vf20w    \n"
        "vmulw.xyzw $vf29, $vf19, $vf21w    \n"
        "vmulw.xyzw $vf30, $vf19, $vf22w    \n"
        "vadd.xyzw  $vf24, $vf24, $vf28     \n"
        "vadd.xyzw  $vf25, $vf25, $vf29     \n"
        "vadd.xyzw  $vf26, $vf26, $vf30     \n"
        "sqc2       $vf24, 0x00(%[o0])      \n"
        "sqc2       $vf25, 0x00(%[o1])      \n"
        "sqc2       $vf26, 0x00(%[o2])      \n"
        :
        : [m]  "r" (m),
          [i0] "r" (in0), [i1] "r" (in1), [i2] "r" (in2),
          [o0] "r" (out0), [o1] "r" (out1), [o2] "r" (out2)
        : "memory");
}

static inline void ps2_vu0_xform4(const VU_MATRIX* m,
                                  const VU_VECTOR* in0,
                                  const VU_VECTOR* in1,
                                  const VU_VECTOR* in2,
                                  const VU_VECTOR* in3,
                                  VU_VECTOR* out0,
                                  VU_VECTOR* out1,
                                  VU_VECTOR* out2,
                                  VU_VECTOR* out3)
{
    __asm__ __volatile__ (
        "lqc2       $vf16, 0x00(%[m])       \n"
        "lqc2       $vf17, 0x10(%[m])       \n"
        "lqc2       $vf18, 0x20(%[m])       \n"
        "lqc2       $vf19, 0x30(%[m])       \n"
        "lqc2       $vf20, 0x00(%[i0])      \n"
        "lqc2       $vf21, 0x00(%[i1])      \n"
        "lqc2       $vf22, 0x00(%[i2])      \n"
        "lqc2       $vf23, 0x00(%[i3])      \n"
        "vmulx.xyzw $vf24, $vf16, $vf20x    \n"
        "vmulx.xyzw $vf25, $vf16, $vf21x    \n"
        "vmulx.xyzw $vf26, $vf16, $vf22x    \n"
        "vmulx.xyzw $vf27, $vf16, $vf23x    \n"
        "vmuly.xyzw $vf28, $vf17, $vf20y    \n"
        "vmuly.xyzw $vf29, $vf17, $vf21y    \n"
        "vmuly.xyzw $vf30, $vf17, $vf22y    \n"
        "vmuly.xyzw $vf31, $vf17, $vf23y    \n"
        "vadd.xyzw  $vf24, $vf24, $vf28     \n"
        "vadd.xyzw  $vf25, $vf25, $vf29     \n"
        "vadd.xyzw  $vf26, $vf26, $vf30     \n"
        "vadd.xyzw  $vf27, $vf27, $vf31     \n"
        "vmulz.xyzw $vf28, $vf18, $vf20z    \n"
        "vmulz.xyzw $vf29, $vf18, $vf21z    \n"
        "vmulz.xyzw $vf30, $vf18, $vf22z    \n"
        "vmulz.xyzw $vf31, $vf18, $vf23z    \n"
        "vadd.xyzw  $vf24, $vf24, $vf28     \n"
        "vadd.xyzw  $vf25, $vf25, $vf29     \n"
        "vadd.xyzw  $vf26, $vf26, $vf30     \n"
        "vadd.xyzw  $vf27, $vf27, $vf31     \n"
        "vmulw.xyzw $vf28, $vf19, $vf20w    \n"
        "vmulw.xyzw $vf29, $vf19, $vf21w    \n"
        "vmulw.xyzw $vf30, $vf19, $vf22w    \n"
        "vmulw.xyzw $vf31, $vf19, $vf23w    \n"
        "vadd.xyzw  $vf24, $vf24, $vf28     \n"
        "vadd.xyzw  $vf25, $vf25, $vf29     \n"
        "vadd.xyzw  $vf26, $vf26, $vf30     \n"
        "vadd.xyzw  $vf27, $vf27, $vf31     \n"
        "sqc2       $vf24, 0x00(%[o0])      \n"
        "sqc2       $vf25, 0x00(%[o1])      \n"
        "sqc2       $vf26, 0x00(%[o2])      \n"
        "sqc2       $vf27, 0x00(%[o3])      \n"
        :
        : [m]  "r" (m),
          [i0] "r" (in0), [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3),
          [o0] "r" (out0), [o1] "r" (out1), [o2] "r" (out2), [o3] "r" (out3)
        : "memory");
}


// Batch form for terrain/entity fallbacks. It keeps vf16-vf19 resident for the
// entire batch and advances four VU_VECTORs per EE loop iteration, so the matrix
// is loaded once per batch rather than once per quad. count must be a positive
// multiple of four; callers keep the small/special cases on ps2_vu0_xform3/4.
//
// Unlike ps2_vu0_xform3/4 this form REQUIRES w == 1.0 on every input vertex,
// which is what loadVert() writes for both the packed and the float streams.
// The w round then scales column 3 by a known 1.0, so the four vmulw disappear
// and the column is added straight in: 6 ops per vertex instead of 7, and
// vf28-vf31 are left untouched by the last round. A projective vertex fed here
// would silently lose the translation scaling -- send those to ps2_vu0_xform4.
static inline void ps2_vu0_xform4_batch(const VU_MATRIX* m,
                                        const VU_VECTOR* input,
                                        VU_VECTOR* output,
                                        int count)
{
    if (m == nullptr || input == nullptr || output == nullptr || count <= 0 || (count & 3) != 0)
        return;

    const VU_VECTOR* in = input;
    VU_VECTOR* out = output;
    int remaining = count;
    __asm__ __volatile__ (
        "lqc2       $vf16, 0x00(%[m])       \n"
        "lqc2       $vf17, 0x10(%[m])       \n"
        "lqc2       $vf18, 0x20(%[m])       \n"
        "lqc2       $vf19, 0x30(%[m])       \n"
        "1:                                    \n"
        "lqc2       $vf20, 0x00(%[in])      \n"
        "lqc2       $vf21, 0x10(%[in])      \n"
        "lqc2       $vf22, 0x20(%[in])      \n"
        "lqc2       $vf23, 0x30(%[in])      \n"
        "vmulx.xyzw $vf24, $vf16, $vf20x    \n"
        "vmulx.xyzw $vf25, $vf16, $vf21x    \n"
        "vmulx.xyzw $vf26, $vf16, $vf22x    \n"
        "vmulx.xyzw $vf27, $vf16, $vf23x    \n"
        "vmuly.xyzw $vf28, $vf17, $vf20y    \n"
        "vmuly.xyzw $vf29, $vf17, $vf21y    \n"
        "vmuly.xyzw $vf30, $vf17, $vf22y    \n"
        "vmuly.xyzw $vf31, $vf17, $vf23y    \n"
        "vadd.xyzw  $vf24, $vf24, $vf28     \n"
        "vadd.xyzw  $vf25, $vf25, $vf29     \n"
        "vadd.xyzw  $vf26, $vf26, $vf30     \n"
        "vadd.xyzw  $vf27, $vf27, $vf31     \n"
        "vmulz.xyzw $vf28, $vf18, $vf20z    \n"
        "vmulz.xyzw $vf29, $vf18, $vf21z    \n"
        "vmulz.xyzw $vf30, $vf18, $vf22z    \n"
        "vmulz.xyzw $vf31, $vf18, $vf23z    \n"
        "vadd.xyzw  $vf24, $vf24, $vf28     \n"
        "vadd.xyzw  $vf25, $vf25, $vf29     \n"
        "vadd.xyzw  $vf26, $vf26, $vf30     \n"
        "vadd.xyzw  $vf27, $vf27, $vf31     \n"
        "vadd.xyzw  $vf24, $vf24, $vf19     \n"
        "vadd.xyzw  $vf25, $vf25, $vf19     \n"
        "vadd.xyzw  $vf26, $vf26, $vf19     \n"
        "vadd.xyzw  $vf27, $vf27, $vf19     \n"
        "sqc2       $vf24, 0x00(%[out])     \n"
        "sqc2       $vf25, 0x10(%[out])     \n"
        "sqc2       $vf26, 0x20(%[out])     \n"
        "sqc2       $vf27, 0x30(%[out])     \n"
        "addiu      %[in], %[in], 0x40      \n"
        "addiu      %[out], %[out], 0x40    \n"
        "addiu      %[n], %[n], -4          \n"
        "bgtz       %[n], 1b                \n"
        "nop                                    \n"
        : [in] "+r" (in), [out] "+r" (out), [n] "+r" (remaining)
        : [m] "r" (m)
        : "memory");
}

// Shared near/guard-band clipper for the PS2 GL compatibility path and the
// fast-draw backends. It clips in clip-space against:
//   z + w >= 0 (the homogeneous near plane), w >= epsilon (safe divide),
//   -gx*w <= x <= gx*w, -gy*w <= y <= gy*w.

static const float PS2_NEAR_CLIP_W = 0.0001f;
static const int PS2_CLIP_MAX_POLY = 16;

struct ClipVert
{
    float cx, cy, cz, cw;
    float u, v;
    unsigned char r, g, bl, a;
};

enum
{
    PS2_CLIP_NEAR  = 1 << 0,
    PS2_CLIP_LEFT  = 1 << 1,
    PS2_CLIP_RIGHT = 1 << 2,
    PS2_CLIP_DOWN  = 1 << 3,
    PS2_CLIP_UP    = 1 << 4,
    PS2_CLIP_EYE   = 1 << 5
};

// Near plane only.
//
// This is what a "fully inside the frustum" section still has to pay. Skipping
// the four guard-band planes is safe -- they are a trivial-reject optimisation
// and the GS scissors whatever survives -- but skipping the NEAR test is not,
// because the projection divides by w. A vertex at or behind the eye has w <= 0,
// so 1/w explodes and the quad is smeared across the whole screen as a huge
// stretched polygon.
//
// Relying on the frustum classification alone to guarantee w > 0 is what broke:
// the flag is computed once per frame in clipRenderersByFrustrum, which runs
// BEFORE updateRenderers, and it skips sections that have no mesh yet. A section
// whose build completed later in the same frame was therefore drawn with a flag
// from an older camera position. One compare per vertex makes that whole class
// of failure impossible by construction rather than by argument.
static inline int ps2_clip_outcode_near(float z, float w)
{
    int c = 0;
    if (z + w < 0.0f) c |= PS2_CLIP_NEAR;
    if (w < PS2_NEAR_CLIP_W) c |= PS2_CLIP_EYE;
    return c;
}

static inline int ps2_clip_outcode(float x, float y, float z, float w, float gx, float gy)
{
    int c = ps2_clip_outcode_near(z, w);
    if (x < -gx * w) c |= PS2_CLIP_LEFT;
    if (x >  gx * w) c |= PS2_CLIP_RIGHT;
    if (y < -gy * w) c |= PS2_CLIP_DOWN;
    if (y >  gy * w) c |= PS2_CLIP_UP;
    return c;
}

static inline ClipVert ps2_clip_lerp(const ClipVert& a, const ClipVert& b, float t)
{
    ClipVert o;
    o.cx = a.cx + (b.cx - a.cx) * t;
    o.cy = a.cy + (b.cy - a.cy) * t;
    o.cz = a.cz + (b.cz - a.cz) * t;
    o.cw = a.cw + (b.cw - a.cw) * t;
    o.u  = a.u  + (b.u  - a.u)  * t;
    o.v  = a.v  + (b.v  - a.v)  * t;
    o.r  = (unsigned char)(a.r  + (float)((int)b.r  - (int)a.r)  * t);
    o.g  = (unsigned char)(a.g  + (float)((int)b.g  - (int)a.g)  * t);
    o.bl = (unsigned char)(a.bl + (float)((int)b.bl - (int)a.bl) * t);
    o.a  = (unsigned char)(a.a  + (float)((int)b.a  - (int)a.a)  * t);
    return o;
}

static inline float ps2_clip_plane_value(const ClipVert& v, int plane, float gx, float gy)
{
    switch (plane) {
    case PS2_CLIP_NEAR:  return v.cz + v.cw;
    case PS2_CLIP_EYE:   return v.cw - PS2_NEAR_CLIP_W;
    case PS2_CLIP_LEFT:  return v.cx + gx * v.cw;
    case PS2_CLIP_RIGHT: return gx * v.cw - v.cx;
    case PS2_CLIP_DOWN:  return v.cy + gy * v.cw;
    case PS2_CLIP_UP:    return gy * v.cw - v.cy;
    default:             return 1.0f;
    }
}

static inline int ps2_clip_against_plane(const ClipVert* in, int inCount,
                                         ClipVert* out, int plane,
                                         float gx, float gy)
{
    if (inCount <= 0)
        return 0;

    int outCount = 0;
    ClipVert prev = in[inCount - 1];
    float prevVal = ps2_clip_plane_value(prev, plane, gx, gy);
    bool prevInside = prevVal >= 0.0f;

    for (int i = 0; i < inCount; ++i) {
        ClipVert cur = in[i];
        float curVal = ps2_clip_plane_value(cur, plane, gx, gy);
        bool curInside = curVal >= 0.0f;

        if (curInside != prevInside) {
            // Adjacent triangles traverse their shared edge in opposite
            // directions. Always interpolate outside -> inside so the two
            // intersections are bit-identical, including UVs and colors.
            // Otherwise rounding can leave a crack after GS 12.4 conversion.
            const ClipVert& outside = prevInside ? cur : prev;
            const ClipVert& inside = prevInside ? prev : cur;
            const float outsideVal = prevInside ? curVal : prevVal;
            const float insideVal = prevInside ? prevVal : curVal;
            float t = outsideVal / (outsideVal - insideVal);
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            if (outCount < PS2_CLIP_MAX_POLY)
                out[outCount++] = ps2_clip_lerp(outside, inside, t);
        }

        if (curInside && outCount < PS2_CLIP_MAX_POLY)
            out[outCount++] = cur;

        prev = cur;
        prevVal = curVal;
        prevInside = curInside;
    }
    return outCount;
}

static inline int ps2_clip_poly_guard(ClipVert* poly, int count, int mask, float gx, float gy)
{
    ClipVert tmpA[PS2_CLIP_MAX_POLY];
    ClipVert tmpB[PS2_CLIP_MAX_POLY];

    if (count <= 0)
        return 0;
    if (count > PS2_CLIP_MAX_POLY)
        count = PS2_CLIP_MAX_POLY;

    for (int i = 0; i < count; ++i)
        tmpA[i] = poly[i];

    ClipVert* in = tmpA;
    ClipVert* out = tmpB;
    int n = count;

    const int planes[6] = { PS2_CLIP_NEAR, PS2_CLIP_EYE, PS2_CLIP_LEFT, PS2_CLIP_RIGHT, PS2_CLIP_DOWN, PS2_CLIP_UP };
    for (int p = 0; p < 6; ++p) {
        if ((mask & planes[p]) == 0)
            continue;
        n = ps2_clip_against_plane(in, n, out, planes[p], gx, gy);
        if (n <= 0)
            return 0;
        ClipVert* swap = in;
        in = out;
        out = swap;
    }

    for (int i = 0; i < n; ++i)
        poly[i] = in[i];
    return n;
}

// Homogeneous clip of a line segment against the same five planes, using the
// same plane values, so a line and a polygon edge can never disagree about
// where the frustum is.
//
// Lines cannot reuse ps2_clip_poly_guard: Sutherland-Hodgman walks a CLOSED
// edge list, so it treats the two endpoints as a degenerate polygon and clips
// the segment a->b->a. Against a single plane that happens to give the right
// two points, but it emits them in polygon order and duplicates the crossing,
// and against two planes it collapses to nothing. A segment needs the simpler
// parametric form below, which just walks each endpoint inward to the crossing.
//
// Returns false when the segment lies entirely outside one plane, in which case
// a and b are left untouched.
static inline bool ps2_clip_segment_guard(ClipVert& a, ClipVert& b, int mask,
                                          float gx, float gy)
{
    const int planes[5] = { PS2_CLIP_NEAR, PS2_CLIP_LEFT, PS2_CLIP_RIGHT, PS2_CLIP_DOWN, PS2_CLIP_UP };
    for (int p = 0; p < 5; ++p) {
        if ((mask & planes[p]) == 0)
            continue;

        const float va = ps2_clip_plane_value(a, planes[p], gx, gy);
        const float vb = ps2_clip_plane_value(b, planes[p], gx, gy);
        if (va < 0.0f && vb < 0.0f)
            return false;
        if (va >= 0.0f && vb >= 0.0f)
            continue;

        // Exactly one endpoint is outside, so va - vb cannot be zero here.
        float t = va / (va - vb);
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        const ClipVert crossing = ps2_clip_lerp(a, b, t);
        if (va < 0.0f)
            a = crossing;
        else
            b = crossing;
    }
    return true;
}

#endif // PS2_PLATFORM
