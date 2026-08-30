#include "platform/ClientProfilerBackend.h"

#include "platform/Log.h"
#include "java/System.h"
#include "net/minecraft/src/RenderGlobal.h"
#include "net/minecraft/src/World.h"

#include <algorithm>

namespace
{
struct WiiWindow
{
	long long startedMs = 0;
	long long frameNs = 0;
	long long tickNs = 0;
	long long renderNs = 0;
	// Time inside lwjgl::Display::update(). On Wii that is the wait for the
	// EFB->XFB copy plus VIDEO_WaitVSync(), i.e. how much of the frame the CPU
	// spends parked on the GP and the retrace. It is the number that separates
	// a GP-bound build from a CPU-bound one, so it cannot stay unrecorded.
	long long swapNs = 0;
	long long maxSwapNs = 0;
	long long maxFrameNs = 0;
	long long maxTickNs = 0;
	long long maxRenderNs = 0;
	long long chunkLoadNs = 0, maxChunkLoadNs = 0;
	int chunkLoadCount = 0;
	long long populateNs = 0, maxPopulateNs = 0;
	int populateCount = 0;
	long long populatePhaseNs[8] = {};
	long long maxPopulatePhaseNs[8] = {};
	int populatePhaseCount[8] = {};
	long long generateNs = 0, maxGenerateNs = 0;
	int generateCount = 0;
	long long meshNs = 0, maxMeshNs = 0;
	int meshCount = 0;
	long long unloadSaveNs = 0, maxUnloadSaveNs = 0;
	int unloadSaveCount = 0;
	long long tickUpdatesNs = 0, maxTickUpdatesNs = 0;
	long long tickQueueSum = 0, tickQueueMax = 0;
	long long mobSpawnNs = 0, maxMobSpawnNs = 0;
	long long saveInfoNs = 0, maxSaveInfoNs = 0;
	long long mapStorageNs = 0, maxMapStorageNs = 0;
	int frames = 0;
	int ticks = 0;
	int maxTicksPerFrame = 0;
	int chunkUpdates = 0;
	int slowTicks = 0;
} g_wii;

constexpr long long kSlowTickNs = 100000000LL;

void add(long long ns, long long& total, long long& maximum)
{
	total += ns;
	if (ns > maximum) maximum = ns;
}

void addSample(long long ns, long long& total, long long& maximum, int& count)
{
	add(ns, total, maximum);
	++count;
}

void resetWindow(long long nowMs)
{
	g_wii = WiiWindow{};
	g_wii.startedMs = nowMs;
}
}

extern "C" void wii_perf_add_chunk_load_ns(long long ns) { addSample(ns, g_wii.chunkLoadNs, g_wii.maxChunkLoadNs, g_wii.chunkLoadCount); }
extern "C" void wii_perf_add_populate_ns(long long ns) { addSample(ns, g_wii.populateNs, g_wii.maxPopulateNs, g_wii.populateCount); }
extern "C" void wii_perf_add_populate_phase_ns(int phase, long long ns)
{
	if (phase >= 0 && phase < 8)
		addSample(ns, g_wii.populatePhaseNs[phase], g_wii.maxPopulatePhaseNs[phase], g_wii.populatePhaseCount[phase]);
}
extern "C" void wii_perf_add_generate_ns(long long ns) { addSample(ns, g_wii.generateNs, g_wii.maxGenerateNs, g_wii.generateCount); }
extern "C" void wii_perf_add_mesh_ns(long long ns) { addSample(ns, g_wii.meshNs, g_wii.maxMeshNs, g_wii.meshCount); }
extern "C" void wii_perf_add_unload_save_ns(long long ns) { addSample(ns, g_wii.unloadSaveNs, g_wii.maxUnloadSaveNs, g_wii.unloadSaveCount); }
extern "C" void wii_perf_add_tickupdates_ns(long long ns) { add(ns, g_wii.tickUpdatesNs, g_wii.maxTickUpdatesNs); }
extern "C" void wii_perf_add_tickupdates_queue(long long size) { g_wii.tickQueueSum += size; g_wii.tickQueueMax = std::max(g_wii.tickQueueMax, size); }
extern "C" void wii_perf_add_mobspawn_ns(long long ns) { add(ns, g_wii.mobSpawnNs, g_wii.maxMobSpawnNs); }
extern "C" void wii_perf_add_saveworldinfo_ns(long long ns) { add(ns, g_wii.saveInfoNs, g_wii.maxSaveInfoNs); }
extern "C" void wii_perf_add_mapstorage_ns(long long ns) { add(ns, g_wii.mapStorageNs, g_wii.maxMapStorageNs); }

namespace ClientProfilerBackend
{
void frameBegin()
{
	if (g_wii.startedMs == 0)
		resetWindow(System::currentTimeMillis());
}

void ticks(long long ns, int ticksThisFrame)
{
	g_wii.tickNs += ns;
	g_wii.maxTickNs = std::max(g_wii.maxTickNs, ns);
	g_wii.ticks += ticksThisFrame;
	g_wii.maxTicksPerFrame = std::max(g_wii.maxTicksPerFrame, ticksThisFrame);
	if (ns > kSlowTickNs) ++g_wii.slowTicks;
}

void lighting(long long) {}
void displayUpdate(long long ns) { add(ns, g_wii.swapNs, g_wii.maxSwapNs); }
void render(long long) {}

void frameEnd(long long frameNs, long long, long long renderNs,
              int, int chunkUpdates, World* world, RenderGlobal* renderGlobal)
{
	g_wii.frameNs += frameNs;
	g_wii.renderNs += renderNs;
	g_wii.maxFrameNs = std::max(g_wii.maxFrameNs, frameNs);
	g_wii.maxRenderNs = std::max(g_wii.maxRenderNs, renderNs);
	++g_wii.frames;
	g_wii.chunkUpdates += chunkUpdates;
	const long long now = System::currentTimeMillis();
	if (now - g_wii.startedMs < 10000)
		return;

	const long long avgFrame = g_wii.frames ? g_wii.frameNs / g_wii.frames : 0;
	const long long avgTick = g_wii.frames ? g_wii.tickNs / g_wii.frames : 0;
	const long long avgRender = g_wii.frames ? g_wii.renderNs / g_wii.frames : 0;
	const long long avgSwap = g_wii.frames ? g_wii.swapNs / g_wii.frames : 0;
	MC_LOG_INFO("wii.perf", "frames=%d fps=%d frame=%ld/%ldms tick=%ld/%ldms render=%ld/%ldms swap=%ld/%ldms ticks=%d maxTicks=%d updates=%d pending=%d slowTicks=%d\n",
	            g_wii.frames, g_wii.frames / 10,
	            (long)(avgFrame / 1000000LL), (long)(g_wii.maxFrameNs / 1000000LL),
	            (long)(avgTick / 1000000LL), (long)(g_wii.maxTickNs / 1000000LL),
	            (long)(avgRender / 1000000LL), (long)(g_wii.maxRenderNs / 1000000LL),
	            (long)(avgSwap / 1000000LL), (long)(g_wii.maxSwapNs / 1000000LL),
	            g_wii.ticks, g_wii.maxTicksPerFrame, g_wii.chunkUpdates,
	            renderGlobal != nullptr ? (int)renderGlobal->pendingRendererUpdateCount() : 0,
	            g_wii.slowTicks);
	MC_LOG_INFO("wii.perf", "chunkLoad=%d:%ld/%ldms generate=%d:%ld/%ldms populate=%d:%ld/%ldms mesh=%d:%ld/%ldms unloadSave=%d:%ld/%ldms tickUpdates=%ld/%ldms queue=%ld/%ld mobSpawn=%ld/%ldms saveInfo=%ld/%ldms mapStorage=%ld/%ldms\n",
	            g_wii.chunkLoadCount, (long)(g_wii.chunkLoadCount ? g_wii.chunkLoadNs / g_wii.chunkLoadCount / 1000000LL : 0), (long)(g_wii.maxChunkLoadNs / 1000000LL),
	            g_wii.generateCount, (long)(g_wii.generateCount ? g_wii.generateNs / g_wii.generateCount / 1000000LL : 0), (long)(g_wii.maxGenerateNs / 1000000LL),
	            g_wii.populateCount, (long)(g_wii.populateCount ? g_wii.populateNs / g_wii.populateCount / 1000000LL : 0), (long)(g_wii.maxPopulateNs / 1000000LL),
	            g_wii.meshCount, (long)(g_wii.meshCount ? g_wii.meshNs / g_wii.meshCount / 1000000LL : 0), (long)(g_wii.maxMeshNs / 1000000LL),
	            g_wii.unloadSaveCount, (long)(g_wii.unloadSaveCount ? g_wii.unloadSaveNs / g_wii.unloadSaveCount / 1000000LL : 0), (long)(g_wii.maxUnloadSaveNs / 1000000LL),
	            (long)(g_wii.ticks ? g_wii.tickUpdatesNs / g_wii.ticks / 1000000LL : 0), (long)(g_wii.maxTickUpdatesNs / 1000000LL),
	            (long)(g_wii.ticks ? g_wii.tickQueueSum / g_wii.ticks : 0), (long)g_wii.tickQueueMax,
	            (long)(g_wii.ticks ? g_wii.mobSpawnNs / g_wii.ticks / 1000000LL : 0), (long)(g_wii.maxMobSpawnNs / 1000000LL),
	            (long)(g_wii.ticks ? g_wii.saveInfoNs / g_wii.ticks / 1000000LL : 0), (long)(g_wii.maxSaveInfoNs / 1000000LL),
	            (long)(g_wii.ticks ? g_wii.mapStorageNs / g_wii.ticks / 1000000LL : 0), (long)(g_wii.maxMapStorageNs / 1000000LL));
	MC_LOG_INFO("wii.perf", "populatePhase total=%ld/%ld lakes=%ld/%ld dungeons=%ld/%ld fillers=%ld/%ld ores=%ld/%ld decoration=%ld/%ld springs=%ld/%ld snow=%ld/%ldms\n",
	            (long)(g_wii.populatePhaseCount[0] ? g_wii.populatePhaseNs[0] / g_wii.populatePhaseCount[0] / 1000000LL : 0), (long)(g_wii.maxPopulatePhaseNs[0] / 1000000LL),
	            (long)(g_wii.populatePhaseCount[1] ? g_wii.populatePhaseNs[1] / g_wii.populatePhaseCount[1] / 1000000LL : 0), (long)(g_wii.maxPopulatePhaseNs[1] / 1000000LL),
	            (long)(g_wii.populatePhaseCount[2] ? g_wii.populatePhaseNs[2] / g_wii.populatePhaseCount[2] / 1000000LL : 0), (long)(g_wii.maxPopulatePhaseNs[2] / 1000000LL),
	            (long)(g_wii.populatePhaseCount[3] ? g_wii.populatePhaseNs[3] / g_wii.populatePhaseCount[3] / 1000000LL : 0), (long)(g_wii.maxPopulatePhaseNs[3] / 1000000LL),
	            (long)(g_wii.populatePhaseCount[4] ? g_wii.populatePhaseNs[4] / g_wii.populatePhaseCount[4] / 1000000LL : 0), (long)(g_wii.maxPopulatePhaseNs[4] / 1000000LL),
	            (long)(g_wii.populatePhaseCount[5] ? g_wii.populatePhaseNs[5] / g_wii.populatePhaseCount[5] / 1000000LL : 0), (long)(g_wii.maxPopulatePhaseNs[5] / 1000000LL),
	            (long)(g_wii.populatePhaseCount[6] ? g_wii.populatePhaseNs[6] / g_wii.populatePhaseCount[6] / 1000000LL : 0), (long)(g_wii.maxPopulatePhaseNs[6] / 1000000LL),
	            (long)(g_wii.populatePhaseCount[7] ? g_wii.populatePhaseNs[7] / g_wii.populatePhaseCount[7] / 1000000LL : 0), (long)(g_wii.maxPopulatePhaseNs[7] / 1000000LL));
	if (world != nullptr)
	{
		const std::string chunks = world->getChunkProviderStats();
		MC_LOG_INFO("wii.perf", "world ents=%d lightQ=%d pending=%d | %s\n",
		            (int)world->getLoadedEntityList().size(),
		            (int)world->getPendingLightingUpdateCount(),
		            renderGlobal != nullptr ? (int)renderGlobal->pendingRendererUpdateCount() : 0,
		            chunks.c_str());
	}
	resetWindow(now);
}
}
