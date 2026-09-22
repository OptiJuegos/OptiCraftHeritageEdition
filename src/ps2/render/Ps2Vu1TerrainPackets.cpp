#include "ps2/render/Ps2Vu1TerrainPackets.h"

#if defined(PS2_PLATFORM) && defined(PS2_ENABLE_VU1_TERRAIN)

#include <algorithm>
#include <cstdint>
#include <cstring>

#include <gsKit.h>
#include <gsInline.h>

#include "ps2/render/Ps2ClipGuard.h"

namespace
{
    enum DmaTagId
    {
        DMA_TAG_CNT = 1,
        DMA_TAG_REF = 3
    };

#if PS2_VU1_CLIPPED_PARTIALS || PS2_VU1_SIDE_CLIPPED_PARTIALS || PS2_VU1_CLIPPED_PROBE_BATCHES_PER_FRAME > 0
    static constexpr int kBufferBase[2] = { 24, 524 };
    static const int kBufferQwords = 500;
#else
    static constexpr int kBufferBase[2] = { 16, 520 };
    static const int kBufferQwords = 504;
#endif
    static const int kControlQw = 0;
    static const int kFastPositionQw = 8;
    static const int kFastTexCoordQw = 88;
    static const int kFastColorQw = 168;
    static const int kFastOutputQw = 248;
    static const int kClipPositionQw = 8;
    static const int kClipTexCoordQw = 28;
    static const int kClipColorQw = 48;
    static const int kClipScratchAQw = 68;
    static const int kClipScratchBQw = 95;
    static const int kClipOutputQw = 122;
    static const int kClipMaxPolygonVertices = 9;
    static const int kClipMaxOutputVerticesPerQuad =
        (kClipMaxPolygonVertices - 2) * 3;
    // [A+D tag NLOOP 1][CLAMP][draw GIF tag]. The remaining pass-level GS
    // registers travel once per pass through kPassStateChainQwords below.
    static const int kOutputHeaderQwords = 3;
    static const int kBatchStateQwords = 5;
    static const int kBatchChainQwords = 18;
    static const int kPassStateFirstQw = 9;
    static const int kPassStateRegisters = 6;
    static const int kPassStateChainQwords = 3 + kPassStateRegisters;

    static const unsigned int kVifNop = 0x00;
    static const unsigned int kVifStcycl = 0x01;
    static const unsigned int kVifItop = 0x04;
    static const unsigned int kVifMscal = 0x14;
    static const unsigned int kVifUnpackV2_16 = 0x65;
    static const unsigned int kVifUnpackV4_32 = 0x6c;
    static const unsigned int kVifUnpackV4_16 = 0x6d;
    static const unsigned int kVifUnpackV4_8 = 0x6e;
    static const unsigned int kVifDirect = 0x50;
    static const unsigned int kVifUnsigned = 0x4000;

    static_assert(PS2_VU1_TERRAIN_CONTEXT_QWORDS <= kBufferBase[0],
                  "VU1 context overlaps the first dynamic buffer");
    static_assert(kFastOutputQw + kOutputHeaderQwords +
                      PS2_VU1_TERRAIN_MAX_VERTICES * 3 <= kBufferQwords,
                  "VU1 terrain output exceeds a dynamic buffer");
    static_assert(kClipPositionQw + PS2_VU1_TERRAIN_CLIPPED_MAX_VERTICES <= kClipTexCoordQw &&
                      kClipTexCoordQw + PS2_VU1_TERRAIN_CLIPPED_MAX_VERTICES <= kClipColorQw &&
                      kClipColorQw + PS2_VU1_TERRAIN_CLIPPED_MAX_VERTICES <= kClipScratchAQw,
                  "VU1 clipped terrain input streams overlap");
    static_assert(kClipScratchAQw + kClipMaxPolygonVertices * 3 <= kClipScratchBQw,
                  "VU1 clipped terrain scratch buffers overlap");
    static_assert(kClipScratchBQw + kClipMaxPolygonVertices * 3 <= kClipOutputQw,
                  "VU1 clipped terrain scratch overlaps output");
    static_assert(kClipOutputQw + kOutputHeaderQwords +
                      (PS2_VU1_TERRAIN_CLIPPED_MAX_VERTICES / 4) *
                          kClipMaxOutputVerticesPerQuad * 3 <= kBufferQwords,
                  "VU1 clipped terrain output exceeds a dynamic buffer");
    static_assert(kBufferBase[1] + kBufferQwords <= 1024,
                  "VU1 terrain buffers exceed data memory");
    static_assert(kPassStateFirstQw + kPassStateRegisters <=
                      PS2_VU1_TERRAIN_CONTEXT_QWORDS,
                  "VU1 pass state records fall outside the common state");
    static_assert((PS2_VU1_TERRAIN_MAX_VERTICES & 3) == 0,
                  "VU1 terrain batches must contain whole quads");
    static_assert((PS2_VU1_TERRAIN_CLIPPED_MAX_VERTICES & 3) == 0,
                  "VU1 clipped terrain batches must contain whole quads");

    static inline unsigned int vifCode(unsigned int command, unsigned int num,
                                       unsigned int immediate)
    {
        return (command << 24) | ((num & 0xffu) << 16) | (immediate & 0xffffu);
    }

    static inline unsigned int vifUnpack(unsigned int command, unsigned int num,
                                         unsigned int address, bool isUnsigned)
    {
        const unsigned int flags = isUnsigned ? kVifUnsigned : 0u;
        return vifCode(command, num, (address & 0x3ffu) | flags);
    }

    static inline unsigned int floatBits(float value)
    {
        unsigned int bits;
        std::memcpy(&bits, &value, sizeof(bits));
        return bits;
    }

    static inline void setU64(Ps2VifQword& q, int half, unsigned long long value)
    {
        q.w[half * 2 + 0] = (unsigned int)(value & 0xffffffffull);
        q.w[half * 2 + 1] = (unsigned int)(value >> 32);
    }

    static inline Ps2VifQword dmaTag(unsigned int id, unsigned int qwc,
                                     const void* address = nullptr)
    {
        Ps2VifQword q = {{ 0, 0, 0, 0 }};
        q.w[0] = (qwc & 0xffffu) | ((id & 7u) << 28);
        q.w[1] = (unsigned int)(reinterpret_cast<std::uintptr_t>(address) & 0x7fffffffu);
        return q;
    }

    static inline void commandQword(Ps2VifQword& q, unsigned int a,
                                    unsigned int b = 0, unsigned int c = 0,
                                    unsigned int d = 0)
    {
        q.w[0] = a;
        q.w[1] = b;
        q.w[2] = c;
        q.w[3] = d;
    }

    static Ps2VifQword* reserveChain(Ps2Vu1DmaQueue& queue, int qwords)
    {
        Ps2VifQword* chain = queue.reserve(qwords);
        if (chain != nullptr)
            return chain;
        if (!queue.submit())
            return nullptr;
        return queue.reserve(qwords);
    }

    static void fillBatchState(Ps2VifQword* state, int count,
                               float translateX, float translateY, float translateZ,
                               const Ps2TerrainGpuState& gpu,
                               int tileX, int tileY,
                               bool clipped)
    {
        std::memset(state, 0, sizeof(Ps2VifQword) * kBatchStateQwords);
        const int positionQw = clipped ? kClipPositionQw : kFastPositionQw;
        const int texCoordQw = clipped ? kClipTexCoordQw : kFastTexCoordQw;
        const int colorQw = clipped ? kClipColorQw : kFastColorQw;
        const int outputQw = clipped ? kClipOutputQw : kFastOutputQw;
        commandQword(state[0], (unsigned int)count,
                     (unsigned int)positionQw,
                     (unsigned int)texCoordQw,
                     (unsigned int)colorQw);
        commandQword(state[1], (unsigned int)outputQw, 0, 0, 0);
        state[2].w[0] = floatBits(translateX);
        state[2].w[1] = floatBits(translateY);
        state[2].w[2] = floatBits(translateZ);

        const unsigned long long prim = GS_SETREG_PRIM(
            clipped ? GS_PRIM_PRIM_TRIANGLE : GS_PRIM_PRIM_TRISTRIP,
            gpu.render.smoothShading ? 1 : 0, 1,
            gpu.render.fogEnabled ? 1 : 0,
            gpu.primAlphaEnable, gpu.primAAEnable,
            0, gpu.primContext, 0);
        const unsigned long long drawTag = GIF_TAG(clipped ? 0 : count, 1, 1, prim, 0, 3);
        setU64(state[3], 0, drawTag);
        setU64(state[3], 1, 0x412ull);

        const unsigned long long clamp = GS_SETREG_CLAMP(
            GS_CMODE_REGION_REPEAT, GS_CMODE_REGION_REPEAT,
            15, tileX << 4, 15, tileY << 4);
        setU64(state[4], 0, clamp);
        setU64(state[4], 1, (unsigned long long)(GS_CLAMP_1 + gpu.primContext));
    }
}

void ps2_vu1_terrain_build_common_state(Ps2Vu1TerrainCommonState& out,
                                        const Ps2NativeFrameContext& frame,
                                        const Ps2TerrainGpuState& gpu)
{
    Ps2VifQword* state = out.qwords;
    std::memset(state, 0, sizeof(out.qwords));
    for (int col = 0; col < 4; ++col)
        for (int row = 0; row < 4; ++row)
            state[col].w[row] = floatBits(frame.mvp[col * 4 + row]);

    // The z lanes no longer scale a clip-space z into the buffer range: they
    // are the qScale/qBias pair of the depth mapping, applied by the VU to q
    // directly (see PS2_DEPTH_NEAR_BOOST). The w lane of the scale record
    // carries the buffer maximum, which the VU needs to clamp against -- unlike
    // the EE paths it has no bounds check of its own, and a vertex nearer than
    // the mapping's near plane would otherwise run beyond the unsigned
    // PSMZ_16 range used by the CT16 framebuffer.
    const float halfW = gpu.viewW * 0.5f;
    const float halfH = gpu.viewH * 0.5f;
    state[4].w[0] = floatBits(halfW);
    state[4].w[1] = floatBits(-halfH);
    state[4].w[2] = floatBits(gpu.depthQScale);
    state[4].w[3] = floatBits((float)PS2_GS_Z_MAX);
    state[5].w[0] = floatBits(halfW + (float)gpu.offsetX / 16.0f);
    state[5].w[1] = floatBits(halfH + (float)gpu.offsetY / 16.0f);
    state[5].w[2] = floatBits(gpu.depthQBias);
    // Upper XY bound for the microprogram's clamp. The x/y lanes above already
    // fold in the viewport offset, so the value the VU clamps is a GS primitive
    // coordinate and the limit is the register's own maximum.
    state[5].w[3] = floatBits(PS2_GS_XY_MAX);
    state[6].w[0] = floatBits(4.0f);
    state[6].w[1] = floatBits(8.0f);
    // Keep the common packet layout unchanged: unused decode lanes carry
    // density/256 and an integer EXP flag for both VU1 entry points.
    const bool exponentialFog = gpu.render.fogEnabled &&
        gpu.render.fogMode == 0x0800u;
    state[6].w[2] = floatBits(exponentialFog
        ? std::max(0.0f, gpu.render.fogDensity) / 256.0f : 0.0f);
    state[6].w[3] = exponentialFog ? 1u : 0u;

    float fogScale = 0.0f;
    float fogBias = 255.0f;
    if (gpu.render.fogEnabled && gpu.render.fogEnd > gpu.render.fogStart)
    {
        const float invRange = 1.0f / (gpu.render.fogEnd - gpu.render.fogStart);
        fogScale = -255.0f * invRange;
        fogBias = gpu.render.fogEnd * 255.0f * invRange;
    }
    state[7].w[0] = floatBits(fogScale);
    state[7].w[1] = floatBits(fogBias);
    state[7].w[2] = floatBits(255.0f);
    state[7].w[3] = floatBits(PS2_NEAR_CLIP_W);

    // The VU header carries the per-batch CLAMP and nothing else, so this tag
    // opens a single A+D record. The six pass-level records that follow it here
    // are shipped once per pass by ps2_vu1_terrain_append_common_state.
    const unsigned long long adTag = GIF_TAG(1, 0, 0, 0, 0, 1);
    setU64(state[8], 0, adTag);
    setU64(state[8], 1, GIF_AD);
    setU64(state[9], 0, gpu.tex0);
    setU64(state[9], 1, (unsigned long long)(GS_TEX0_1 + gpu.primContext));
    setU64(state[10], 0, gpu.test);
    setU64(state[10], 1, (unsigned long long)(GS_TEST_1 + gpu.primContext));
    setU64(state[11], 0, gpu.zbuf);
    setU64(state[11], 1, (unsigned long long)(GS_ZBUF_1 + gpu.primContext));
    setU64(state[12], 0, gpu.texa);
    setU64(state[12], 1, (unsigned long long)GS_TEXA);
    setU64(state[13], 0, gpu.alpha);
    setU64(state[13], 1, (unsigned long long)(GS_ALPHA_1 + gpu.primContext));

    const int fogR = std::max(0, std::min(255, (int)(gpu.render.fogR * 255.0f)));
    const int fogG = std::max(0, std::min(255, (int)(gpu.render.fogG * 255.0f)));
    const int fogB = std::max(0, std::min(255, (int)(gpu.render.fogB * 255.0f)));
    const unsigned long long fogColor = (unsigned long long)fogR |
        ((unsigned long long)fogG << 8) | ((unsigned long long)fogB << 16);
    setU64(state[14], 0, fogColor);
    setU64(state[14], 1, (unsigned long long)GS_FOGCOL);

    // Sampler records travel beside the context, not inside it.
    std::memset(out.extraPassRegisters, 0, sizeof(out.extraPassRegisters));
    out.extraPassRegisterCount = 0;
    setU64(out.extraPassRegisters[0], 0, gpu.tex1);
    setU64(out.extraPassRegisters[0], 1, (unsigned long long)(GS_TEX1_1 + gpu.primContext));
    out.extraPassRegisterCount = 1;
    if (gpu.mipmapped)
    {
        setU64(out.extraPassRegisters[1], 0, gpu.miptbp1);
        setU64(out.extraPassRegisters[1], 1, (unsigned long long)(GS_MIPTBP1_1 + gpu.primContext));
        out.extraPassRegisterCount = 2;
    }

#if PS2_VU1_CLIPPED_PARTIALS || PS2_VU1_SIDE_CLIPPED_PARTIALS || PS2_VU1_CLIPPED_PROBE_BATCHES_PER_FRAME > 0
    const float guardX = ps2_guard_clip_scale(gpu.viewW);
    const float guardY = ps2_guard_clip_scale(gpu.viewH);
    state[15].w[0] = floatBits(0.0f);
    state[15].w[1] = floatBits(0.0f);
    state[15].w[2] = floatBits(-PS2_NEAR_CLIP_W);
    state[15].w[3] = floatBits(1.0f);
    state[16].w[0] = floatBits(1.0f);
    state[16].w[1] = floatBits(0.0f);
    state[16].w[2] = floatBits(0.0f);
    state[16].w[3] = floatBits(guardX);
    state[17].w[0] = floatBits(-1.0f);
    state[17].w[1] = floatBits(0.0f);
    state[17].w[2] = floatBits(0.0f);
    state[17].w[3] = floatBits(guardX);
    state[18].w[0] = floatBits(0.0f);
    state[18].w[1] = floatBits(1.0f);
    state[18].w[2] = floatBits(0.0f);
    state[18].w[3] = floatBits(guardY);
    state[19].w[0] = floatBits(0.0f);
    state[19].w[1] = floatBits(-1.0f);
    state[19].w[2] = floatBits(0.0f);
    state[19].w[3] = floatBits(guardY);
#endif
}

bool ps2_vu1_terrain_append_common_state(Ps2Vu1DmaQueue& queue,
                                         const Ps2Vu1TerrainCommonState& state)
{
    const int extraRegisters = std::max(0, std::min(state.extraPassRegisterCount,
                                                    PS2_VU1_TERRAIN_MAX_EXTRA_PASS_REGISTERS));
    const int passRegisters = kPassStateRegisters + extraRegisters;
    const int passStateChainQwords = kPassStateChainQwords + extraRegisters;
    Ps2VifQword* chain = reserveChain(queue,
        PS2_VU1_TERRAIN_CONTEXT_QWORDS + 2 + passStateChainQwords);
    if (chain == nullptr)
        return false;

    chain[0] = dmaTag(DMA_TAG_CNT, PS2_VU1_TERRAIN_CONTEXT_QWORDS + 1);
    commandQword(chain[1],
                 vifCode(kVifStcycl, 0, 0x0101),
                 vifCode(kVifNop, 0, 0),
                 vifCode(kVifNop, 0, 0),
                 vifUnpack(kVifUnpackV4_32, PS2_VU1_TERRAIN_CONTEXT_QWORDS,
                           0, false));
    for (int i = 0; i < PS2_VU1_TERRAIN_CONTEXT_QWORDS; ++i)
        chain[2 + i] = state.qwords[i];

    // Pass-level GS registers straight to the GIF over Path2. They used to ride
    // the per-batch VU header, which rewrote TEX0 once per 80 vertices and made
    // the GS drop its texture cache every time. This runs while the caller still
    // holds Path1 and before the first MSCAL of the pass, so no XGKICK is in
    // flight to arbitrate against. The DIRECT code sits in the last word of its
    // qword so its payload starts on the following quadword boundary.
    Ps2VifQword* passState = chain + 2 + PS2_VU1_TERRAIN_CONTEXT_QWORDS;
    passState[0] = dmaTag(DMA_TAG_CNT, passStateChainQwords - 1);
    commandQword(passState[1],
                 vifCode(kVifNop, 0, 0),
                 vifCode(kVifNop, 0, 0),
                 vifCode(kVifNop, 0, 0),
                 vifCode(kVifDirect, 0, (unsigned int)(passRegisters + 1)));
    const unsigned long long passStateTag =
        GIF_TAG(passRegisters, 1, 0, 0, 0, 1);
    setU64(passState[2], 0, passStateTag);
    setU64(passState[2], 1, GIF_AD);
    for (int i = 0; i < kPassStateRegisters; ++i)
        passState[3 + i] = state.qwords[kPassStateFirstQw + i];
    // TEX1/MIPTBP1 ride the same A+D packet, after the six context records.
    for (int i = 0; i < extraRegisters; ++i)
        passState[3 + kPassStateRegisters + i] = state.extraPassRegisters[i];
    return true;
}

bool ps2_vu1_terrain_append_sliced_batch(Ps2Vu1DmaQueue& queue,
                                         const Ps2TerrainMesh& mesh,
                                         const Ps2Vu1TerrainSlice* slices,
                                         int sliceCount,
                                         int totalCount,
                                         float translateX,
                                         float translateY,
                                         float translateZ,
                                         const Ps2TerrainGpuState& gpu,
                                         int tileX,
                                         int tileY,
                                         int bufferIndex,
                                         int microAddress)
{
    if (slices == nullptr || sliceCount <= 0 || sliceCount > 8 ||
        totalCount <= 0 || totalCount > PS2_VU1_TERRAIN_MAX_VERTICES ||
        (totalCount & 3) != 0 || bufferIndex < 0 || bufferIndex >= 2 ||
        microAddress < 0 || microAddress >= 2048)
        return false;

    int verified = 0;
    for (int i = 0; i < sliceCount; ++i)
    {
        if (slices[i].firstVertex < 0 || slices[i].vertexCount <= 0 ||
            (slices[i].firstVertex & 3) != 0 || (slices[i].vertexCount & 3) != 0 ||
            slices[i].firstVertex + slices[i].vertexCount > mesh.vertexCount())
            return false;
        verified += slices[i].vertexCount;
    }
    if (verified != totalCount)
        return false;

    const int bufferBase = kBufferBase[bufferIndex];
    Ps2VifQword state[kBatchStateQwords] __attribute__((aligned(16)));
    fillBatchState(state, totalCount, translateX, translateY, translateZ,
                   gpu, tileX, tileY, false);

    // One state block, then three scatter streams. Each slice contributes a
    // one-qword UNPACK command plus a REF tag for positions, UVs and colours.
    // The VIF destination offsets compact the non-contiguous source ranges into
    // one contiguous VU buffer, so one MSCAL/XGKICK handles the whole tile batch.
    const int chainQwords = 9 + sliceCount * 9;
    Ps2VifQword* chain = reserveChain(queue, chainQwords);
    if (chain == nullptr)
        return false;

    int q = 0;
    chain[q++] = dmaTag(DMA_TAG_CNT, kBatchStateQwords + 1);
    commandQword(chain[q++],
                 vifCode(kVifStcycl, 0, 0x0101),
                 vifCode(kVifNop, 0, 0),
                 vifCode(kVifNop, 0, 0),
                 vifUnpack(kVifUnpackV4_32, kBatchStateQwords,
                           bufferBase + kControlQw, false));
    for (int i = 0; i < kBatchStateQwords; ++i)
        chain[q++] = state[i];

    int dst = 0;
    for (int i = 0; i < sliceCount; ++i)
    {
        const int count = slices[i].vertexCount;
        chain[q++] = dmaTag(DMA_TAG_CNT, 1);
        commandQword(chain[q++], vifCode(kVifNop, 0, 0), vifCode(kVifNop, 0, 0),
                     vifCode(kVifNop, 0, 0),
                     vifUnpack(kVifUnpackV4_16, (unsigned int)count,
                               bufferBase + kFastPositionQw + dst, false));
        chain[q++] = dmaTag(DMA_TAG_REF, (unsigned int)(count / 2),
                            mesh.positions() + (std::size_t)slices[i].firstVertex * 4u);
        dst += count;
    }

    dst = 0;
    for (int i = 0; i < sliceCount; ++i)
    {
        const int count = slices[i].vertexCount;
        chain[q++] = dmaTag(DMA_TAG_CNT, 1);
        commandQword(chain[q++], vifCode(kVifNop, 0, 0), vifCode(kVifNop, 0, 0),
                     vifCode(kVifNop, 0, 0),
                     vifUnpack(kVifUnpackV2_16, (unsigned int)count,
                               bufferBase + kFastTexCoordQw + dst, false));
        chain[q++] = dmaTag(DMA_TAG_REF, (unsigned int)(count / 4),
                            mesh.texCoords() + (std::size_t)slices[i].firstVertex * 2u);
        dst += count;
    }

    dst = 0;
    for (int i = 0; i < sliceCount; ++i)
    {
        const int count = slices[i].vertexCount;
        chain[q++] = dmaTag(DMA_TAG_CNT, 1);
        commandQword(chain[q++], vifCode(kVifNop, 0, 0), vifCode(kVifNop, 0, 0),
                     vifCode(kVifNop, 0, 0),
                     vifUnpack(kVifUnpackV4_8, (unsigned int)count,
                               bufferBase + kFastColorQw + dst, true));
        chain[q++] = dmaTag(DMA_TAG_REF, (unsigned int)(count / 4),
                            mesh.colors() + (std::size_t)slices[i].firstVertex * 4u);
        dst += count;
    }

    chain[q++] = dmaTag(DMA_TAG_CNT, 1);
    commandQword(chain[q++],
                 vifCode(kVifItop, 0, (unsigned int)bufferBase),
                 vifCode(kVifMscal, 0, (unsigned int)microAddress));
    return q == chainQwords;
}

bool ps2_vu1_terrain_append_batch(Ps2Vu1DmaQueue& queue,
                                  const Ps2TerrainMesh& mesh,
                                  int firstVertex,
                                  int count,
                                  float translateX,
                                  float translateY,
                                  float translateZ,
                                  const Ps2TerrainGpuState& gpu,
                                  int tileX,
                                  int tileY,
                                  int bufferIndex,
                                  int microAddress,
                                  bool clipped)
{
    const int maxVertices = clipped
        ? PS2_VU1_TERRAIN_CLIPPED_MAX_VERTICES
        : PS2_VU1_TERRAIN_MAX_VERTICES;
    if (count <= 0 || count > maxVertices ||
        (count & 3) != 0 || (firstVertex & 3) != 0 ||
        bufferIndex < 0 || bufferIndex >= 2 ||
        microAddress < 0 || microAddress >= 2048)
        return false;

    const int bufferBase = kBufferBase[bufferIndex];
    const int positionQw = clipped ? kClipPositionQw : kFastPositionQw;
    const int texCoordQw = clipped ? kClipTexCoordQw : kFastTexCoordQw;
    const int colorQw = clipped ? kClipColorQw : kFastColorQw;

    Ps2VifQword state[kBatchStateQwords] __attribute__((aligned(16)));
    fillBatchState(state, count, translateX, translateY, translateZ,
                   gpu, tileX, tileY, clipped);

    Ps2VifQword* chain = reserveChain(queue, kBatchChainQwords);
    if (chain == nullptr)
        return false;

    chain[0] = dmaTag(DMA_TAG_CNT, kBatchStateQwords + 1);
    commandQword(chain[1],
                 vifCode(kVifStcycl, 0, 0x0101),
                 vifCode(kVifNop, 0, 0),
                 vifCode(kVifNop, 0, 0),
                 vifUnpack(kVifUnpackV4_32, kBatchStateQwords,
                           bufferBase + kControlQw, false));
    for (int i = 0; i < kBatchStateQwords; ++i)
        chain[2 + i] = state[i];

    chain[7] = dmaTag(DMA_TAG_CNT, 1);
    commandQword(chain[8],
                 vifCode(kVifNop, 0, 0),
                 vifCode(kVifNop, 0, 0),
                 vifCode(kVifNop, 0, 0),
                 vifUnpack(kVifUnpackV4_16, (unsigned int)count,
                           bufferBase + positionQw, false));
    const short* positions = mesh.positions() + (std::size_t)firstVertex * 4u;
    chain[9] = dmaTag(DMA_TAG_REF, (unsigned int)(count / 2), positions);

    chain[10] = dmaTag(DMA_TAG_CNT, 1);
    commandQword(chain[11],
                 vifCode(kVifNop, 0, 0),
                 vifCode(kVifNop, 0, 0),
                 vifCode(kVifNop, 0, 0),
                 vifUnpack(kVifUnpackV2_16, (unsigned int)count,
                           bufferBase + texCoordQw, false));
    const short* texCoords = mesh.texCoords() + (std::size_t)firstVertex * 2u;
    chain[12] = dmaTag(DMA_TAG_REF, (unsigned int)(count / 4), texCoords);

    chain[13] = dmaTag(DMA_TAG_CNT, 1);
    commandQword(chain[14],
                 vifCode(kVifNop, 0, 0),
                 vifCode(kVifNop, 0, 0),
                 vifCode(kVifNop, 0, 0),
                 vifUnpack(kVifUnpackV4_8, (unsigned int)count,
                           bufferBase + colorQw, true));
    const unsigned char* colors = mesh.colors() + (std::size_t)firstVertex * 4u;
    chain[15] = dmaTag(DMA_TAG_REF, (unsigned int)(count / 4), colors);

    chain[16] = dmaTag(DMA_TAG_CNT, 1);
    commandQword(chain[17],
                 vifCode(kVifItop, 0, (unsigned int)bufferBase),
                 vifCode(kVifMscal, 0, (unsigned int)microAddress));
    return true;
}

#endif
