#include "ps2/render/Ps2Vu1Terrain.h"

#if defined(PS2_PLATFORM) && defined(PS2_ENABLE_VU1_TERRAIN)

#include <algorithm>
#include <cstdint>
#include <cstring>

#include <dma.h>
#include <dmaKit.h>
#include <gif_tags.h>
#include <gsKit.h>

#include "platform/Log.h"
#include "ps2/render/Ps2Graphics.h"
#include "ps2/render/Ps2RenderBackend.h"
#include "ps2/render/Ps2Tuning.h"
#include "ps2/render/Ps2Vu1DmaQueue.h"
#include "ps2/render/Ps2Vu1TerrainPackets.h"

extern "C" {
extern unsigned char ps2Vu1Terrain_CodeStart[];
extern unsigned char ps2Vu1Terrain_CodeEnd[];
extern unsigned char ps2Vu1TerrainFast_CodeEnd[];
extern unsigned char ps2Vu1TerrainClip_Entry[];
extern unsigned char ps2Vu1TerrainCanary_Entry[];
}

namespace
{
    static const unsigned int kVifNop = 0x00;
    static const unsigned int kVifStcycl = 0x01;
    static const unsigned int kVifFlusha = 0x13;
    static const unsigned int kVifMscal = 0x14;
    static const unsigned int kVifMpg = 0x4a;
    static const unsigned int kVifUnpackV4_32 = 0x6c;

    static const int kCanaryDataQw = 16;
    static const int kCanaryGifQwords = 15;
    static const int kCanaryPacketQwords = kCanaryGifQwords + 2;
    static_assert(kCanaryDataQw + kCanaryGifQwords <= 1024,
                  "VU1 canary packet exceeds data memory");

    static Ps2Vu1DmaQueue s_queue;
    static unsigned int s_microPacket[1024] __attribute__((aligned(64)));
    static unsigned int s_barrierPacket[4] __attribute__((aligned(64)));
    static Ps2VifQword s_canaryPacket[kCanaryPacketQwords] __attribute__((aligned(64)));
    static Ps2Vu1TerrainCommonState s_commonState __attribute__((aligned(16)));
    static Ps2TerrainGpuState s_passGpuState;
    static Ps2Vu1TerrainStats s_stats = {};
    // Lifetime, never reset. See ps2_vu1_terrain_submitted_vertices().
    static long s_lifetimeVertices = 0;
    // Latched by disableVu1Terrain() below. Never cleared: a VIF1 stall means
    // the packet contract is broken, and retrying it every frame would turn one
    // dropped frame into a permanent 300ms-per-frame timeout.
    static bool s_vu1Disabled = false;
    static bool s_commonStateValid = false;
    static bool s_passGpuStateValid = false;
    static bool s_path1Owned = false;
    static int s_nextTerrainBuffer = 0;
    static int s_clippedProbeBatchesRemaining = 0;

    static_assert(PS2_VU1_CLIPPED_PROBE_BATCHES_PER_FRAME >= 0,
                  "VU1 clipped probe budget cannot be negative");

#ifdef PS2_RENDER_STATS
#define PS2_VU1_TERRAIN_STAT(expr) do { expr; } while (0)
#else
#define PS2_VU1_TERRAIN_STAT(expr) do { } while (0)
#endif

    static inline unsigned int vifCode(unsigned int command, unsigned int num,
                                       unsigned int immediate)
    {
        return (command << 24) | ((num & 0xffu) << 16) | (immediate & 0xffffu);
    }

    static inline void setPacketU64(volatile Ps2VifQword& qword, int half,
                                    unsigned long long value)
    {
        qword.w[half * 2 + 0] = (unsigned int)(value & 0xffffffffull);
        qword.w[half * 2 + 1] = (unsigned int)(value >> 32);
    }

    static inline void setPacketCommands(volatile Ps2VifQword& qword,
                                         unsigned int a, unsigned int b = 0,
                                         unsigned int c = 0, unsigned int d = 0)
    {
        qword.w[0] = a;
        qword.w[1] = b;
        qword.w[2] = c;
        qword.w[3] = d;
    }

    static bool buildCanaryPacket(volatile Ps2VifQword* packet, int microAddress,
                                  const Ps2TerrainGpuState& gpu)
    {
        if (packet == nullptr || microAddress < 0 || microAddress >= 1024 ||
            gpu.viewW <= 0.0f || gpu.viewH <= 0.0f || !gsGlobal)
            return false;

        for (int i = 0; i < kCanaryPacketQwords; ++i)
            setPacketCommands(packet[i], 0, 0, 0, 0);

        setPacketCommands(packet[0],
                          vifCode(kVifStcycl, 0, 0x0101),
                          vifCode(kVifNop, 0, 0),
                          vifCode(kVifNop, 0, 0),
                          vifCode(kVifUnpackV4_32, kCanaryGifQwords,
                                  kCanaryDataQw));

        const unsigned long long adTag = GIF_TAG(2, 0, 0, 0, 0, 1);
        setPacketU64(packet[1], 0, adTag);
        setPacketU64(packet[1], 1, GIF_AD);
        setPacketU64(packet[2], 0, GS_SETREG_TEST(0, 0, 0, 0, 0, 0, 1, 1));
        setPacketU64(packet[2], 1, (unsigned long long)(GS_TEST_1 + gpu.primContext));
        setPacketU64(packet[3], 0,
                     GS_SETREG_ZBUF(gsGlobal->ZBuffer / 8192, gsGlobal->PSMZ, 1));
        setPacketU64(packet[3], 1, (unsigned long long)(GS_ZBUF_1 + gpu.primContext));

        const unsigned long long prim = GS_SETREG_PRIM(
            GS_PRIM_PRIM_TRISTRIP, 0, 0, 0, 0, 0, 0, gpu.primContext, 0);
        const unsigned long long drawTag = GIF_TAG(4, 0, 1, prim, 0, 2);
        setPacketU64(packet[4], 0, drawTag);
        setPacketU64(packet[4], 1,
                     (unsigned long long)GIF_REG_RGBAQ |
                     ((unsigned long long)GIF_REG_XYZ2 << 4));

        const int halfWidth = std::min(80, (int)gpu.viewW / 4);
        const int halfHeight = std::min(48, (int)gpu.viewH / 4);
        const int centerX = gpu.offsetX + (int)(gpu.viewW * 8.0f);
        const int centerY = gpu.offsetY + (int)(gpu.viewH * 8.0f);
        const unsigned int x0 = (unsigned int)(centerX - halfWidth * 16);
        const unsigned int x1 = (unsigned int)(centerX + halfWidth * 16);
        const unsigned int y0 = (unsigned int)(centerY - halfHeight * 16);
        const unsigned int y1 = (unsigned int)(centerY + halfHeight * 16);
        const unsigned long long magenta = GS_SETREG_RGBAQ(
            0x80, 0x00, 0x80, 0x80, 0x3f800000);
        const unsigned long long vertices[4] = {
            GS_SETREG_XYZ2(x0, y0, 0),
            GS_SETREG_XYZ2(x0, y1, 0),
            GS_SETREG_XYZ2(x1, y0, 0),
            GS_SETREG_XYZ2(x1, y1, 0)
        };
        for (int vertex = 0; vertex < 4; ++vertex)
        {
            setPacketU64(packet[5 + vertex * 2], 0, magenta);
            setPacketU64(packet[6 + vertex * 2], 0, vertices[vertex]);
        }

        const unsigned long long restoreTag = GIF_TAG(2, 1, 0, 0, 0, 1);
        setPacketU64(packet[13], 0, restoreTag);
        setPacketU64(packet[13], 1, GIF_AD);
        setPacketU64(packet[14], 0, gpu.test);
        setPacketU64(packet[14], 1, (unsigned long long)(GS_TEST_1 + gpu.primContext));
        setPacketU64(packet[15], 0, gpu.zbuf);
        setPacketU64(packet[15], 1, (unsigned long long)(GS_ZBUF_1 + gpu.primContext));

        setPacketCommands(packet[16],
                          vifCode(kVifMscal, 0, (unsigned int)microAddress),
                          vifCode(kVifFlusha, 0, 0),
                          vifCode(kVifNop, 0, 0),
                          vifCode(kVifNop, 0, 0));
        return true;
    }

    static inline unsigned long eeCycles()
    {
#ifdef PS2_RENDER_STATS
        unsigned int cycles;
        __asm__ __volatile__("mfc0 %0, $9" : "=r"(cycles));
        return cycles;
#else
        return 0;
#endif
    }

    // Take the whole direct-terrain backend out of service for the rest of the
    // session. Everything it draws is also drawable by the clipping-capable VU0
    // path, so a VIF1 or VU1 stall costs frame rate and nothing else -- which is
    // strictly better than the alternative this replaces, where the EE spun in
    // dma_channel_wait with no timeout and the console stopped responding.
    static void disableVu1Terrain(const char* reason)
    {
        if (s_vu1Disabled)
            return;
        s_vu1Disabled = true;
        // ps2_vu1_dma_recover() resets VIF1, which clears VU1 micro memory, and
        // already clears the shared resident-program tag; this is belt-and-
        // braces for any caller that reaches disableVu1Terrain() without going
        // through recover() first.
        ps2_vu1_set_resident_program(PS2_VU1_PROGRAM_NONE);
        s_path1Owned = false;
        s_commonStateValid = false;
        s_passGpuStateValid = false;
        s_queue.discardPending();
        // Path1 was dropped mid-pass, so ps2_render_release_path1() will take
        // its early return and never resync the backend's GS register caches.
        // Without this the first VU0 draw after the fallback believes TEST,
        // ZBUF, CLAMP and ALPHA still hold what the VU1 pass programmed.
        ps2_render_invalidate_path1_state();
        MC_LOG_ERROR("render",
            "[PS2] direct VU1 terrain disabled for this session (%s);"
            " falling back to the VU0 path\n",
            reason ? reason : "?");
    }

    static bool waitDirectDma(const char* site)
    {
#ifdef PS2_RENDER_STATS
        const unsigned long begin = eeCycles();
#endif
        if (!ps2_vu1_dma_wait(site))
        {
            disableVu1Terrain(site);
            return false;
        }
        PS2_VU1_TERRAIN_STAT(s_stats.waitCycles += eeCycles() - begin);
        PS2_VU1_TERRAIN_STAT(++s_stats.waits);
        return true;
    }

    static bool sendNormalAndWait(const void* data, int qwords, const char* site)
    {
        if (qwords <= 0 || !s_queue.drain())
            return false;
        // The packet was written through the UCAB mirror; without this its last
        // partial 128-byte line is still in the write-gather buffer when the
        // DMAC reads it. See ps2_vu1_dma_flush_ucab in Ps2Vu1DmaQueue.h.
        ps2_vu1_dma_flush_ucab();
        if (dma_channel_send_normal_ucab(DMA_CHANNEL_VIF1,
                const_cast<void*>(data), (unsigned int)qwords, 0) < 0)
            return false;
        return waitDirectDma(site);
    }

    static bool uploadMicrocode()
    {
        // VU1's microcode slot is shared with any other direct-VU1 backend
        // (see Ps2Vu1DmaQueue.h). Do not shortcut on "did I ever upload" --
        // another backend may have replaced it since, and executing this
        // terrain program's data against a different resident microcode is
        // silent corruption, not a clean failure.
        if (ps2_vu1_resident_program() == PS2_VU1_PROGRAM_TERRAIN)
            return true;

        const unsigned char* begin = ps2Vu1Terrain_CodeStart;
#if PS2_VU1_CLIPPED_PARTIALS || PS2_VU1_CLIPPED_PROBE_BATCHES_PER_FRAME > 0 || PS2_VU1_TERRAIN_CANARY
        const unsigned char* end = ps2Vu1Terrain_CodeEnd;
#else
        // Keep the known-good fast path on its original single-MPG upload.
        // The longer, split upload is exercised only by an explicit clipped
        // terrain or canary validation build.
        const unsigned char* end = ps2Vu1TerrainFast_CodeEnd;
#endif
        const int byteCount = (int)(end - begin);
        if (byteCount <= 0 || (byteCount & 7) != 0)
            return false;

        const int instructionCount = byteCount / 8;
        if (instructionCount <= 0 || instructionCount > 2048)
            return false;

        volatile unsigned int* packet = reinterpret_cast<volatile unsigned int*>(
            reinterpret_cast<std::uintptr_t>(s_microPacket) | 0x30000000u);
        const int packetCapacityWords = (int)(sizeof(s_microPacket) / sizeof(s_microPacket[0]));
        int words = 0;
        int uploadedInstructions = 0;
        while (uploadedInstructions < instructionCount)
        {
            const int batchInstructions = std::min(255,
                instructionCount - uploadedInstructions);
            const int batchWords = batchInstructions * 2;
            if (words + 1 + batchWords > packetCapacityWords)
                return false;

            packet[words++] = vifCode(kVifMpg,
                (unsigned int)batchInstructions,
                (unsigned int)uploadedInstructions);
            for (int i = 0; i < batchWords; ++i)
            {
                unsigned int word;
                std::memcpy(&word,
                    begin + ((std::size_t)uploadedInstructions * 2u + (std::size_t)i) * 4u,
                    sizeof(word));
                packet[words++] = word;
            }
            uploadedInstructions += batchInstructions;
        }
        while ((words & 3) != 0)
        {
            if (words >= packetCapacityWords)
                return false;
            packet[words++] = vifCode(kVifNop, 0, 0);
        }

        if (!sendNormalAndWait((const void*)packet, words / 4, "mpg"))
            return false;
        ps2_vu1_set_resident_program(PS2_VU1_PROGRAM_TERRAIN);
        return true;
    }

    static bool sendFlusha()
    {
        if (!s_queue.drain())
            return false;

        volatile unsigned int* packet = reinterpret_cast<volatile unsigned int*>(
            reinterpret_cast<std::uintptr_t>(s_barrierPacket) | 0x30000000u);
        packet[0] = vifCode(kVifFlusha, 0, 0);
        packet[1] = vifCode(kVifNop, 0, 0);
        packet[2] = vifCode(kVifNop, 0, 0);
        packet[3] = vifCode(kVifNop, 0, 0);
        // One quadword is a single partial write-gather line, so this packet is
        // the one that can never reach memory on its own.
        ps2_vu1_dma_flush_ucab();
        if (dma_channel_send_normal_ucab(DMA_CHANNEL_VIF1,
                (void*)packet, 1, 0) < 0)
            return false;
        return waitDirectDma("flusha");
    }

    static bool ensureCommonState(const Ps2NativeFrameContext& frame,
                                  const Ps2TerrainGpuState& gpu)
    {
        if (s_commonStateValid)
            return true;

        ps2_vu1_terrain_build_common_state(s_commonState, frame, gpu);
        if (!ps2_vu1_terrain_append_common_state(s_queue, s_commonState))
            return false;

        s_commonStateValid = true;
        return true;
    }
}

bool ps2_vu1_terrain_available()
{
#if PS2_LINEAR_DEPTH
    return false;
#else
    // The queue latches its own stall: a timeout inside reserveChain() surfaces
    // as an ordinary append failure, which the caller would otherwise retry on
    // the next section and pay the full timeout for again.
    if (s_queue.faulted())
        disableVu1Terrain("queue");
    return !s_vu1Disabled;
#endif
}

void ps2_vu1_terrain_begin_pass()
{
    s_commonStateValid = false;
    s_nextTerrainBuffer = 0;
#if PS2_VU1_CLIPPED_PROBE_BATCHES_PER_FRAME > 0 && !PS2_VU1_CLIPPED_PARTIALS
    s_clippedProbeBatchesRemaining =
        PS2_VU1_CLIPPED_PROBE_BATCHES_PER_FRAME;
#else
    s_clippedProbeBatchesRemaining = 0;
#endif
    const bool prepared = ps2_render_prepare_terrain_gpu_state(s_passGpuState);
    s_passGpuStateValid = prepared &&
        !s_passGpuState.forceNearZ &&
        s_passGpuState.textureWidth == 256 &&
        s_passGpuState.textureHeight == 256 &&
        s_passGpuState.render.lightVertex == nullptr;

    // Say once, out loud, whether this backend is actually going to run.
    //
    // Every condition above and in ps2_terrain_draw_section's directVu1Usable
    // rejects silently, so a build with PS2_ENABLE_VU1_TERRAIN=ON and a build
    // that never executes a single VU1 batch are indistinguishable from the
    // outside -- both just render, at VU0 speed, with VU0's per-vertex fog.
    // Reported at INFO so the answer is available from a level-1 build, which
    // runs at normal speed; the per-frame TRACE breadcrumbs are too slow to
    // judge performance against.
    static bool s_readinessReported = false;
    if (!s_readinessReported)
    {
        s_readinessReported = true;
        const char* reason =
            !ps2_vu1_terrain_available()   ? (PS2_LINEAR_DEPTH
                                                ? "off: PS2_LINEAR_DEPTH is 1"
                                                : "off: disabled after a stall")
            : !PS2_DIRECT_VU1_TERRAIN      ? "off: PS2_DIRECT_VU1_TERRAIN is 0"
            : !prepared                    ? "off: terrain GPU state unavailable"
            : s_passGpuState.forceNearZ    ? "off: forceNearZ set on the pass"
            : s_passGpuState.textureWidth != 256 ||
              s_passGpuState.textureHeight != 256
                                           ? "off: terrain atlas is not 256x256"
            : s_passGpuState.render.lightVertex != nullptr
                                           ? "off: per-vertex lighting is active"
            : "ready";
        MC_LOG_INFO("render", "[PS2] direct VU1 terrain: %s\n", reason);
    }
}

bool ps2_vu1_terrain_pass_ready()
{
    return ps2_vu1_terrain_available() && s_passGpuStateValid;
}

bool ps2_vu1_terrain_clipped_probe_available()
{
    return s_clippedProbeBatchesRemaining > 0;
}

bool ps2_vu1_terrain_draw_canary()
{
#if PS2_VU1_TERRAIN_CANARY
    const int entryBytes = (int)(ps2Vu1TerrainCanary_Entry - ps2Vu1Terrain_CodeStart);
    if (!s_passGpuStateValid || entryBytes < 0 || (entryBytes & 7) != 0 ||
        !ps2_render_acquire_path1())
    {
        PS2_VU1_TERRAIN_STAT(++s_stats.canaryFailures);
        return false;
    }

    volatile Ps2VifQword* packet = reinterpret_cast<volatile Ps2VifQword*>(
        reinterpret_cast<std::uintptr_t>(s_canaryPacket) | 0x30000000u);
    if (!buildCanaryPacket(packet, entryBytes / 8, s_passGpuState) ||
        !sendFlusha())
    {
        PS2_VU1_TERRAIN_STAT(++s_stats.canaryFailures);
        return false;
    }

    ps2_vu1_dma_flush_ucab();
    if (dma_channel_send_normal_ucab(DMA_CHANNEL_VIF1,
            const_cast<Ps2VifQword*>(packet), kCanaryPacketQwords, 0) < 0)
    {
        PS2_VU1_TERRAIN_STAT(++s_stats.canaryFailures);
        return false;
    }
    PS2_VU1_TERRAIN_STAT(++s_stats.canarySubmitted);

    if (!waitDirectDma("canary"))
    {
        PS2_VU1_TERRAIN_STAT(++s_stats.canaryFailures);
        return false;
    }
    PS2_VU1_TERRAIN_STAT(++s_stats.canaryCompleted);
    return true;
#else
    return false;
#endif
}

bool ps2_render_acquire_path1()
{
    if (s_path1Owned)
        return true;
    if (s_vu1Disabled || !gsGlobal || !uploadMicrocode())
        return false;
    if (!s_queue.waitForDma())
    {
        disableVu1Terrain("acquire");
        return false;
    }

    const bool path3Pending =
        (gsGlobal->Per_Queue != nullptr && gsGlobal->Per_Queue->tag_size > 0) ||
        (gsGlobal->Os_Queue != nullptr && gsGlobal->Os_Queue->tag_size > 0);
    gsKit_queue_exec(gsGlobal);
    dmaKit_wait_fast();
    if (path3Pending)
        gsKit_finish();
    gsKit_queue_reset(gsGlobal->Os_Queue);
    s_path1Owned = true;
    PS2_VU1_TERRAIN_STAT(++s_stats.pathTransitions);
    return true;
}

void ps2_render_barrier_path1()
{
    if (s_path1Owned)
        (void)sendFlusha();
}

void ps2_render_release_path1()
{
    if (!s_path1Owned)
        return;
    (void)sendFlusha();
    s_path1Owned = false;
    // The pass-level GS registers now leave the EE once, inside the common
    // state packet, instead of riding every batch header. gsKit owns Path3
    // again from here and will rewrite TEX0/TEST/ZBUF/ALPHA under us, so the
    // next acquisition has to resend that packet or the first VU1 batch after
    // it would draw with whatever state Path3 left behind.
    s_commonStateValid = false;
    PS2_VU1_TERRAIN_STAT(++s_stats.pathTransitions);
    ps2_render_invalidate_path1_state();
}

Ps2Vu1TerrainDrawResult ps2_vu1_terrain_draw_slices(const Ps2TerrainMesh& mesh,
                                                    const Ps2Vu1TerrainSlice* slices,
                                                    int sliceCount,
                                                    int totalVertices,
                                                    int tileX, int tileY,
                                                    const Ps2NativeFrameContext& frame,
                                                    float translateX,
                                                    float translateY,
                                                    float translateZ)
{
    Ps2Vu1TerrainDrawResult result = { PS2_VU1_TERRAIN_RETRY_NATIVE, 0 };
    if (!ps2_vu1_terrain_available() || !mesh.valid() || !frame.valid ||
        slices == nullptr || sliceCount <= 0 || totalVertices <= 0 ||
        totalVertices > PS2_VU1_TERRAIN_MAX_VERTICES || (totalVertices & 3) != 0)
        return result;
    if (!s_passGpuStateValid)
        return result;
    if (!ps2_render_acquire_path1() || !ensureCommonState(frame, s_passGpuState))
        return result;

    if (!ps2_vu1_terrain_append_sliced_batch(
            s_queue, mesh, slices, sliceCount, totalVertices,
            translateX, translateY, translateZ, s_passGpuState,
            tileX, tileY, s_nextTerrainBuffer, 0))
        return result;

    PS2_VU1_TERRAIN_STAT(
        s_nextTerrainBuffer == 0 ? ++s_stats.buffer0Batches : ++s_stats.buffer1Batches);
    s_nextTerrainBuffer ^= 1;
    PS2_VU1_TERRAIN_STAT(++s_stats.batches);
    PS2_VU1_TERRAIN_STAT(s_stats.vertices += totalVertices);
    PS2_VU1_TERRAIN_STAT(s_stats.qwords += totalVertices);
    PS2_VU1_TERRAIN_STAT(++s_stats.xgkicks);
    s_lifetimeVertices += totalVertices;
    result.vertices = totalVertices;
    result.status = PS2_VU1_TERRAIN_SUBMITTED;
    return result;
}

Ps2Vu1TerrainDrawResult ps2_vu1_terrain_draw_range(const Ps2TerrainMesh& mesh,
                                                   int firstVertex,
                                                   int vertexCount,
                                                   const Ps2NativeFrameContext& frame,
                                                   float translateX,
                                                   float translateY,
                                                   float translateZ,
                                                   bool fullyInside)
{
    Ps2Vu1TerrainDrawResult result = { PS2_VU1_TERRAIN_RETRY_NATIVE, 0 };
    if (!ps2_vu1_terrain_available() || !mesh.valid() || !frame.valid ||
        firstVertex < 0 || vertexCount <= 0 ||
        (firstVertex & 3) != 0 || (vertexCount & 3) != 0 ||
        firstVertex + vertexCount > mesh.vertexCount())
        return result;
#if !PS2_VU1_CLIPPED_PARTIALS && PS2_VU1_CLIPPED_PROBE_BATCHES_PER_FRAME <= 0
    if (!fullyInside)
        return result;
#endif

    if (!s_passGpuStateValid)
        return result;
    if (!ps2_render_acquire_path1() || !ensureCommonState(frame, s_passGpuState))
        return result;

    const bool clipped = !fullyInside;
    const int entryBytes = clipped
        ? (int)(ps2Vu1TerrainClip_Entry - ps2Vu1Terrain_CodeStart)
        : 0;
    if (entryBytes < 0 || (entryBytes & 7) != 0)
        return result;
    const int microAddress = entryBytes / 8;
    const int maxBatchVertices = clipped
        ? PS2_VU1_TERRAIN_CLIPPED_MAX_VERTICES
        : PS2_VU1_TERRAIN_MAX_VERTICES;

    const int rangeEnd = firstVertex + vertexCount;
    const std::vector<Ps2TerrainTileRun>& runs = mesh.runs();
    for (std::size_t i = 0; i < runs.size(); ++i)
    {
        const Ps2TerrainTileRun& run = runs[i];
        const int runBegin = std::max(firstVertex, (int)run.firstVertex);
        const int runEnd = std::min(rangeEnd, (int)(run.firstVertex + run.vertexCount));
        if (runBegin >= runEnd)
            continue;

        int cursor = runBegin;
        while (cursor < runEnd)
        {
            int batch = std::min(maxBatchVertices, runEnd - cursor);
            batch -= batch & 3;
            if (batch <= 0)
                break;

            if (!ps2_vu1_terrain_append_batch(
                    s_queue, mesh, cursor, batch,
                    translateX, translateY, translateZ,
                    s_passGpuState, run.tileX, run.tileY,
                    s_nextTerrainBuffer, microAddress, clipped))
            {
                result.status = result.vertices > 0
                    ? PS2_VU1_TERRAIN_FATAL
                    : PS2_VU1_TERRAIN_RETRY_NATIVE;
                return result;
            }
            PS2_VU1_TERRAIN_STAT(
                s_nextTerrainBuffer == 0
                    ? ++s_stats.buffer0Batches
                    : ++s_stats.buffer1Batches);
            s_nextTerrainBuffer ^= 1;
            PS2_VU1_TERRAIN_STAT(++s_stats.batches);
            PS2_VU1_TERRAIN_STAT(s_stats.vertices += batch);
            PS2_VU1_TERRAIN_STAT(if (clipped) ++s_stats.clippedBatches);
            PS2_VU1_TERRAIN_STAT(if (clipped) s_stats.clippedVertices += batch);
            PS2_VU1_TERRAIN_STAT(s_stats.qwords += batch);
            PS2_VU1_TERRAIN_STAT(++s_stats.xgkicks);
            cursor += batch;
            result.vertices += batch;
            s_lifetimeVertices += batch;
        }
    }

    if (result.vertices == vertexCount)
        result.status = PS2_VU1_TERRAIN_SUBMITTED;
    else if (result.vertices > 0)
        result.status = PS2_VU1_TERRAIN_FATAL;
    return result;
}

Ps2Vu1TerrainDrawResult ps2_vu1_terrain_probe_clipped_range(
                                                   const Ps2TerrainMesh& mesh,
                                                   int firstVertex,
                                                   int vertexCount,
                                                   const Ps2NativeFrameContext& frame,
                                                   float translateX,
                                                   float translateY,
                                                   float translateZ)
{
    Ps2Vu1TerrainDrawResult result = { PS2_VU1_TERRAIN_RETRY_NATIVE, 0 };
#if PS2_VU1_CLIPPED_PROBE_BATCHES_PER_FRAME > 0 && !PS2_VU1_CLIPPED_PARTIALS
    if (s_clippedProbeBatchesRemaining <= 0 || !mesh.valid() ||
        firstVertex < 0 || vertexCount <= 0 ||
        (firstVertex & 3) != 0 || (vertexCount & 3) != 0 ||
        firstVertex + vertexCount > mesh.vertexCount())
        return result;

    const int rangeEnd = firstVertex + vertexCount;
    const std::vector<Ps2TerrainTileRun>& runs = mesh.runs();
    for (std::size_t i = 0; i < runs.size(); ++i)
    {
        const Ps2TerrainTileRun& run = runs[i];
        const int runBegin = std::max(firstVertex, (int)run.firstVertex);
        const int runEnd = std::min(rangeEnd,
            (int)(run.firstVertex + run.vertexCount));
        int batch = std::min(PS2_VU1_TERRAIN_CLIPPED_MAX_VERTICES,
            runEnd - runBegin);
        batch -= batch & 3;
        if (batch <= 0)
            continue;

        // Consume the probe slot even on failure. Repeating a bad microprogram
        // for every partial range in the same frame would defeat the safety cap.
        --s_clippedProbeBatchesRemaining;
        result = ps2_vu1_terrain_draw_range(mesh, runBegin, batch, frame,
            translateX, translateY, translateZ, false);
        if (result.status == PS2_VU1_TERRAIN_SUBMITTED)
        {
            PS2_VU1_TERRAIN_STAT(++s_stats.clippedProbeBatches);
            PS2_VU1_TERRAIN_STAT(s_stats.clippedProbeVertices += result.vertices);
        }
        else
        {
            PS2_VU1_TERRAIN_STAT(++s_stats.clippedProbeRetries);
        }
        return result;
    }

    // A valid packed terrain range should overlap one tile run. Count this as
    // a failed probe contract and do not keep searching this frame.
    s_clippedProbeBatchesRemaining = 0;
    PS2_VU1_TERRAIN_STAT(++s_stats.clippedProbeRetries);
#else
    (void)mesh;
    (void)firstVertex;
    (void)vertexCount;
    (void)frame;
    (void)translateX;
    (void)translateY;
    (void)translateZ;
#endif
    return result;
}

long ps2_vu1_terrain_submitted_vertices()
{
    return s_lifetimeVertices;
}

void ps2_vu1_terrain_take_stats(Ps2Vu1TerrainStats& out)
{
    Ps2Vu1DmaQueueStats queueStats;
    s_queue.takeStats(queueStats);
    out = s_stats;
    out.pages += queueStats.pages;
    out.qwords += queueStats.qwords;
    out.waits += queueStats.waits;
    out.maxPageQwords = std::max(out.maxPageQwords, queueStats.maxPageQwords);
    out.waitCycles += queueStats.waitCycles;
    std::memset(&s_stats, 0, sizeof(s_stats));
}

#else

#if defined(PS2_PLATFORM)

#include <cstring>

#include "ps2/render/Ps2RenderBackend.h"

bool ps2_vu1_terrain_available() { return false; }
void ps2_vu1_terrain_begin_pass() {}
bool ps2_vu1_terrain_pass_ready() { return false; }
bool ps2_vu1_terrain_draw_canary() { return false; }
bool ps2_vu1_terrain_clipped_probe_available() { return false; }
bool ps2_render_acquire_path1() { return false; }
void ps2_render_barrier_path1() {}
void ps2_render_release_path1() {}

Ps2Vu1TerrainDrawResult ps2_vu1_terrain_draw_slices(const Ps2TerrainMesh&,
                                                    const Ps2Vu1TerrainSlice*, int, int,
                                                    int, int, const Ps2NativeFrameContext&,
                                                    float, float, float)
{
    Ps2Vu1TerrainDrawResult result = { PS2_VU1_TERRAIN_RETRY_NATIVE, 0 };
    return result;
}

Ps2Vu1TerrainDrawResult ps2_vu1_terrain_draw_range(const Ps2TerrainMesh&,
                                                   int, int,
                                                   const Ps2NativeFrameContext&,
                                                   float, float, float, bool)
{
    Ps2Vu1TerrainDrawResult result = { PS2_VU1_TERRAIN_RETRY_NATIVE, 0 };
    return result;
}

Ps2Vu1TerrainDrawResult ps2_vu1_terrain_probe_clipped_range(
                                                   const Ps2TerrainMesh&,
                                                   int, int,
                                                   const Ps2NativeFrameContext&,
                                                   float, float, float)
{
    Ps2Vu1TerrainDrawResult result = { PS2_VU1_TERRAIN_RETRY_NATIVE, 0 };
    return result;
}

long ps2_vu1_terrain_submitted_vertices() { return 0; }

void ps2_vu1_terrain_take_stats(Ps2Vu1TerrainStats& out)
{
    std::memset(&out, 0, sizeof(out));
}

#endif
#endif
