#include "platform/ClientProfilerBackend.h"

#include "platform/Log.h"
#include "net/minecraft/src/RenderGlobal.h"
#include "net/minecraft/src/World.h"
#include "ps2/render/Ps2Tuning.h"

extern "C" void ps2_perf_frame_begin();
extern "C" void ps2_perf_frame_end();
extern "C" void ps2_perf_format_and_reset(char* out, int outSize);
extern "C" void ps2_perf_add_tick_ns(long long ns, int ticks);
extern "C" void ps2_perf_add_render_ns(long long ns);
extern "C" void ps2_perf_add_display_update_ns(long long ns);
extern "C" void ps2_perf_add_lighting_ns(long long ns);
extern "C" long ps2_dbg_ram_free_kb();
extern "C" long ps2_dbg_malloc_used_kb();
extern "C" long ps2_dbg_malloc_free_kb();
extern "C" void ps2_dbg_draw_dump();
extern "C" void ps2_dbg_depth_dump();
extern "C" long ps2_dbg_gs_queue_ram_bytes();
extern "C" void ps2_dbg_texture_ram_bytes(long* pixOut, long* clutOut, long* remapOut);
int ps2_mesh_staging_slots_in_use();

namespace
{
long textureRamKb()
{
	long pix = 0, clut = 0, remap = 0;
	ps2_dbg_texture_ram_bytes(&pix, &clut, &remap);
	return (pix + clut + remap) / 1024;
}
}

namespace ClientProfilerBackend
{
void frameBegin() { ps2_perf_frame_begin(); }
void ticks(long long ns, int ticksThisFrame) { ps2_perf_add_tick_ns(ns, ticksThisFrame); }
void lighting(long long ns) { ps2_perf_add_lighting_ns(ns); }
void displayUpdate(long long ns) { ps2_perf_add_display_update_ns(ns); }
void render(long long ns) { ps2_perf_add_render_ns(ns); }

void frameEnd(long long, long long tickNs, long long renderNs,
              int, int chunkUpdates, World* world, RenderGlobal* renderGlobal)
{
	ps2_perf_frame_end();
#if MC_LOG_LEVEL >= 2
	static int frameCount = 0;
	++frameCount;
	if (world != nullptr && (frameCount <= 5 || (frameCount % 120) == 0))
	{
		char phases[1536] = {};
		ps2_perf_format_and_reset(phases, (int)sizeof(phases));
		const std::string chunks = world->getChunkProviderStats();
		RenderTerrainMeshRam meshRam;
		if (renderGlobal != nullptr)
			renderGlobal->terrainMeshRamBreakdown(meshRam);
		MC_LOG_DEBUG("frame", "frame=%d tick=%ldms render=%ldms free=%ldKB mallocUsed=%ldKB mallocFree=%ldKB mesh=%ldKB(raw0=%ld raw1=%ld packed=%ld groups=%ld stg=%ld) tex=%ldKB gsq=%ldKB stage=%d/%d ents=%d lightQ=%d updates=%d pending=%d | %s | %s\n",
		             frameCount,
		             (long)(tickNs / 1000000LL), (long)(renderNs / 1000000LL),
		             ps2_dbg_ram_free_kb(), ps2_dbg_malloc_used_kb(), ps2_dbg_malloc_free_kb(),
		             (long)(meshRam.total() / 1024),
		             (long)(meshRam.liveOpaque / 1024), (long)(meshRam.liveTranslucent / 1024),
		             (long)(meshRam.packedMesh / 1024), (long)(meshRam.faceGroups / 1024),
		             (long)(meshRam.stagingPool / 1024),
		             textureRamKb(), ps2_dbg_gs_queue_ram_bytes() / 1024,
		             ps2_mesh_staging_slots_in_use(), PS2_MESH_STAGING_SLOTS,
		             (int)world->getLoadedEntityList().size(),
		             (int)world->getPendingLightingUpdateCount(),
		             chunkUpdates,
		             renderGlobal != nullptr ? (int)renderGlobal->pendingRendererUpdateCount() : 0,
		             phases, chunks.c_str());
		ps2_dbg_draw_dump();
		ps2_dbg_depth_dump();
	}
#endif
}
}
