#include "platform/ClientProfilerBackend.h"

#include "platform/Log.h"
#include "platform/Profiler.h"
#include "java/System.h"

#include <cstdio>
#include <cstring>

namespace
{
// Slot counts derived from the enums so an appended render/populate phase is
// sized in automatically instead of silently falling out of the arrays (the
// Wii hardcodes 8/14 and would have needed an edit when Structures was added).
constexpr int kRenderPhaseSlots = static_cast<int>(PlatformRenderPhase::HudHints) + 1;
constexpr int kPopulatePhaseSlots = static_cast<int>(PlatformPopulatePhase::Structures) + 1;
constexpr int kTickPhaseSlots = 24;
constexpr int kTickPhaseNameChars = 16;

struct DsTickPhase
{
	char name[kTickPhaseNameChars];
	long long sumNs;
	long long maxNs;
	int count;
};

// One reporting window: everything accumulates from the last reset and is
// zeroed again when the ~10 s summary is emitted.
struct DsWindow
{
	long long startedMs = 0;
	long long frameNs = 0;
	long long tickNs = 0;
	long long renderNs = 0;
	// World::updatingLighting() sits between the ticks and the display update.
	long long lightingNs = 0;
	// Time inside lwjgl::Display::update(). Phase 1 has no vsync/gp-wait split
	// to attribute it to on the 3DS (the Wii splits it because a gpWait that
	// outgrows the tick budget is what identifies a GP-bound frame), so this
	// is the whole present cost as one number until that instrumentation exists.
	long long displayUpdateNs = 0;
	long long maxFrameNs = 0;
	long long maxTickNs = 0;
	long long maxRenderNs = 0;
	long long maxLightingNs = 0;
	long long maxDisplayUpdateNs = 0;
	int frames = 0;
	int ticks = 0;
	int maxTicksPerFrame = 0;
	int chunkUpdates = 0;
	int slowTicks = 0;
	// Render phases (PlatformRenderPhase order) and named tick phases, both per
	// window. The tick table is keyed by name the way the Wii's is: the same
	// literal is written at more than one call site.
	long long renderPhaseNs[kRenderPhaseSlots] = {};
	long long maxRenderPhaseNs[kRenderPhaseSlots] = {};
	DsTickPhase tickPhases[kTickPhaseSlots] = {};
	int tickPhaseCount = 0;
	// World-step spans forwarded from Profiler_3DS.cpp through ds_perf_*.
	long long chunkLoadNs = 0, maxChunkLoadNs = 0;
	int chunkLoadCount = 0;
	long long populateNs = 0, maxPopulateNs = 0;
	int populateCount = 0;
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
	long long populatePhaseNs[kPopulatePhaseSlots] = {};
	long long maxPopulatePhaseNs[kPopulatePhaseSlots] = {};
	int populatePhaseCount[kPopulatePhaseSlots] = {};
} g_3ds;

// A tick past 100 ms has already blown a third of the 3DS frame budget.
constexpr long long kSlowTickNs = 100000000LL;
constexpr long long kReportIntervalMs = 10000;

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
	g_3ds = DsWindow{};
	g_3ds.startedMs = nowMs;
}
}

// Defined here, declared in Profiler_3DS.cpp: same split as the Wii's
// wii_perf_* hooks, minus the GP/vsync instrumentation that has no 3DS
// counterpart in phase 1.
extern "C" void ds_perf_add_render_phase_us(int phase, unsigned int us)
{
	if (phase >= 0 && phase < kRenderPhaseSlots)
		add(static_cast<long long>(us) * 1000LL, g_3ds.renderPhaseNs[phase], g_3ds.maxRenderPhaseNs[phase]);
}
extern "C" void ds_perf_note_tick_phase(const char* name, long long ns)
{
	if (name == nullptr)
		return;
	if (ns < 0)
		ns = 0;
	for (int i = 0; i < g_3ds.tickPhaseCount; i++)
	{
		DsTickPhase& slot = g_3ds.tickPhases[i];
		if (std::strncmp(slot.name, name, kTickPhaseNameChars - 1) != 0)
			continue;
		addSample(ns, slot.sumNs, slot.maxNs, slot.count);
		return;
	}
	if (g_3ds.tickPhaseCount >= kTickPhaseSlots)
		return;
	DsTickPhase& slot = g_3ds.tickPhases[g_3ds.tickPhaseCount++];
	std::strncpy(slot.name, name, kTickPhaseNameChars - 1);
	slot.name[kTickPhaseNameChars - 1] = '\0';
	slot.sumNs = ns;
	slot.maxNs = ns;
	slot.count = 1;
}
extern "C" void ds_perf_add_chunk_load_ns(long long ns) { addSample(ns, g_3ds.chunkLoadNs, g_3ds.maxChunkLoadNs, g_3ds.chunkLoadCount); }
extern "C" void ds_perf_add_populate_ns(long long ns) { addSample(ns, g_3ds.populateNs, g_3ds.maxPopulateNs, g_3ds.populateCount); }
extern "C" void ds_perf_add_populate_phase_ns(int phase, long long ns)
{
	if (phase >= 0 && phase < kPopulatePhaseSlots)
		addSample(ns, g_3ds.populatePhaseNs[phase], g_3ds.maxPopulatePhaseNs[phase], g_3ds.populatePhaseCount[phase]);
}
extern "C" void ds_perf_add_generate_ns(long long ns) { addSample(ns, g_3ds.generateNs, g_3ds.maxGenerateNs, g_3ds.generateCount); }
extern "C" void ds_perf_add_mesh_ns(long long ns) { addSample(ns, g_3ds.meshNs, g_3ds.maxMeshNs, g_3ds.meshCount); }
extern "C" void ds_perf_add_unload_save_ns(long long ns) { addSample(ns, g_3ds.unloadSaveNs, g_3ds.maxUnloadSaveNs, g_3ds.unloadSaveCount); }
extern "C" void ds_perf_add_tickupdates_ns(long long ns) { add(ns, g_3ds.tickUpdatesNs, g_3ds.maxTickUpdatesNs); }
extern "C" void ds_perf_add_tickupdates_queue(long long size)
{
	g_3ds.tickQueueSum += size;
	if (size > g_3ds.tickQueueMax) g_3ds.tickQueueMax = size;
}
extern "C" void ds_perf_add_mobspawn_ns(long long ns) { add(ns, g_3ds.mobSpawnNs, g_3ds.maxMobSpawnNs); }
extern "C" void ds_perf_add_saveworldinfo_ns(long long ns) { add(ns, g_3ds.saveInfoNs, g_3ds.maxSaveInfoNs); }
extern "C" void ds_perf_add_mapstorage_ns(long long ns) { add(ns, g_3ds.mapStorageNs, g_3ds.maxMapStorageNs); }

namespace ClientProfilerBackend
{
void frameBegin()
{
	if (g_3ds.startedMs == 0)
		resetWindow(System::currentTimeMillis());
}

void ticks(long long ns, int ticksThisFrame)
{
	add(ns, g_3ds.tickNs, g_3ds.maxTickNs);
	g_3ds.ticks += ticksThisFrame;
	if (ticksThisFrame > g_3ds.maxTicksPerFrame)
		g_3ds.maxTicksPerFrame = ticksThisFrame;
	if (ns > kSlowTickNs) ++g_3ds.slowTicks;
}

void lighting(long long ns)
{
	add(ns, g_3ds.lightingNs, g_3ds.maxLightingNs);
}

void displayUpdate(long long ns)
{
	add(ns, g_3ds.displayUpdateNs, g_3ds.maxDisplayUpdateNs);
}

void render(long long ns)
{
	// Minecraft.cpp passes this same span again as frameEnd's renderNs, so only
	// one of the two may record it: taking it here means a menu frame (no world
	// render -> render() not called) counts zero instead of replaying the
	// previous frame's render cost.
	add(ns, g_3ds.renderNs, g_3ds.maxRenderNs);
}

void frameEnd(long long frameNs, long long, long long,
              int, int chunkUpdates, World*, RenderGlobal*)
{
	add(frameNs, g_3ds.frameNs, g_3ds.maxFrameNs);
	++g_3ds.frames;
	g_3ds.chunkUpdates += chunkUpdates;

	const long long now = System::currentTimeMillis();
	if (now - g_3ds.startedMs < kReportIntervalMs)
		return;

#if MC_LOG_LEVEL >= 1
	// The whole report block is gated because assembling these lines is dead
	// weight in silent builds; MC_LOG_LEVEL 0 presets (PS2-style) skip it.
	// Each line stays well under the 768-byte buffer McLog::write formats
	// through, so nothing is lost to vsnprintf truncation.
	const long long avgFrame = g_3ds.frames ? g_3ds.frameNs / g_3ds.frames : 0;
	const long long avgTick = g_3ds.frames ? g_3ds.tickNs / g_3ds.frames : 0;
	const long long avgRender = g_3ds.frames ? g_3ds.renderNs / g_3ds.frames : 0;
	const long long avgLighting = g_3ds.frames ? g_3ds.lightingNs / g_3ds.frames : 0;
	const long long avgDisplay = g_3ds.frames ? g_3ds.displayUpdateNs / g_3ds.frames : 0;
	MC_LOG_INFO("3ds.perf", "frames=%d fps=%d frame=%ld/%ldms tick=%ld/%ldms render=%ld/%ldms light=%ld/%ldms display=%ld/%ldms ticks=%d maxTicks=%d updates=%d slowTicks=%d\n",
	            g_3ds.frames, g_3ds.frames / 10,
	            static_cast<long>(avgFrame / 1000000LL), static_cast<long>(g_3ds.maxFrameNs / 1000000LL),
	            static_cast<long>(avgTick / 1000000LL), static_cast<long>(g_3ds.maxTickNs / 1000000LL),
	            static_cast<long>(avgRender / 1000000LL), static_cast<long>(g_3ds.maxRenderNs / 1000000LL),
	            static_cast<long>(avgLighting / 1000000LL), static_cast<long>(g_3ds.maxLightingNs / 1000000LL),
	            static_cast<long>(avgDisplay / 1000000LL), static_cast<long>(g_3ds.maxDisplayUpdateNs / 1000000LL),
	            g_3ds.ticks, g_3ds.maxTicksPerFrame, g_3ds.chunkUpdates, g_3ds.slowTicks);

	{
		// Per-frame average and window maximum, in PlatformRenderPhase order.
		// Tenths of a millisecond: most phases sit under 1 ms.
		static const char* const kRenderPhaseNames[] = {
			"sky", "frustum", "build", "opaque", "ents", "transl", "hand", "hud",
			"entDraw", "tileDraw", "hudItems", "hudText", "hudHints" };
		static_assert(sizeof(kRenderPhaseNames) / sizeof(kRenderPhaseNames[0]) == static_cast<size_t>(kRenderPhaseSlots),
		              "render phase name table must match PlatformRenderPhase");
		char line[512] = {};
		int len = 0;
		for (int i = 0; i < kRenderPhaseSlots && len < static_cast<int>(sizeof(line)) - 40; i++)
		{
			if (g_3ds.renderPhaseNs[i] == 0)
				continue;
			const long avg10 = static_cast<long>(g_3ds.frames ? g_3ds.renderPhaseNs[i] / g_3ds.frames / 100000LL : 0);
			const long max10 = static_cast<long>(g_3ds.maxRenderPhaseNs[i] / 100000LL);
			len += std::snprintf(line + len, sizeof(line) - len, " %s=%ld.%ld/%ld.%ld",
			                     kRenderPhaseNames[i], avg10 / 10, avg10 % 10, max10 / 10, max10 % 10);
		}
		MC_LOG_INFO("3ds.perf", "renderPhase(ms)%s\n", line);

		// Named tick phases, per-call average and window maximum, whole
		// milliseconds; phases that never reached 1 ms at their worst are left
		// out. line is re-terminated first so an empty table cannot reprint the
		// render-phase text above it.
		line[0] = '\0';
		len = 0;
		for (int i = 0; i < g_3ds.tickPhaseCount && len < static_cast<int>(sizeof(line)) - 40; i++)
		{
			const DsTickPhase& slot = g_3ds.tickPhases[i];
			if (slot.maxNs < 1000000LL)
				continue;
			len += std::snprintf(line + len, sizeof(line) - len, " %s=%ld/%ld",
			                     slot.name,
			                     static_cast<long>(slot.count ? slot.sumNs / slot.count / 1000000LL : 0),
			                     static_cast<long>(slot.maxNs / 1000000LL));
		}
		MC_LOG_INFO("3ds.perf", "tickPhase(ms avg/max)%s\n", line);

		static const char* const kPopulatePhaseNames[] = {
			"total", "lakes", "dungeons", "fillers", "ores", "decoration",
			"springs", "snow", "structures" };
		static_assert(sizeof(kPopulatePhaseNames) / sizeof(kPopulatePhaseNames[0]) == static_cast<size_t>(kPopulatePhaseSlots),
		              "populate phase name table must match PlatformPopulatePhase");
		line[0] = '\0';
		len = 0;
		for (int i = 0; i < kPopulatePhaseSlots && len < static_cast<int>(sizeof(line)) - 40; i++)
		{
			if (g_3ds.populatePhaseCount[i] == 0)
				continue;
			len += std::snprintf(line + len, sizeof(line) - len, " %s=%ld/%ld",
			                     kPopulatePhaseNames[i],
			                     static_cast<long>(g_3ds.populatePhaseNs[i] / g_3ds.populatePhaseCount[i] / 1000000LL),
			                     static_cast<long>(g_3ds.maxPopulatePhaseNs[i] / 1000000LL));
		}
		MC_LOG_INFO("3ds.perf", "populatePhase(ms avg/max)%s\n", line);
	}

	// World-step spans: count:avg/max for the once-per-step ones, avg/max per
	// tick for the ones that run every tick.
	MC_LOG_INFO("3ds.perf", "chunkLoad=%d:%ld/%ldms generate=%d:%ld/%ldms populate=%d:%ld/%ldms mesh=%d:%ld/%ldms unloadSave=%d:%ld/%ldms tickUpdates=%ld/%ldms queue=%ld/%ld mobSpawn=%ld/%ldms saveInfo=%ld/%ldms mapStorage=%ld/%ldms\n",
	            g_3ds.chunkLoadCount,
	            static_cast<long>(g_3ds.chunkLoadCount ? g_3ds.chunkLoadNs / g_3ds.chunkLoadCount / 1000000LL : 0),
	            static_cast<long>(g_3ds.maxChunkLoadNs / 1000000LL),
	            g_3ds.generateCount,
	            static_cast<long>(g_3ds.generateCount ? g_3ds.generateNs / g_3ds.generateCount / 1000000LL : 0),
	            static_cast<long>(g_3ds.maxGenerateNs / 1000000LL),
	            g_3ds.populateCount,
	            static_cast<long>(g_3ds.populateCount ? g_3ds.populateNs / g_3ds.populateCount / 1000000LL : 0),
	            static_cast<long>(g_3ds.maxPopulateNs / 1000000LL),
	            g_3ds.meshCount,
	            static_cast<long>(g_3ds.meshCount ? g_3ds.meshNs / g_3ds.meshCount / 1000000LL : 0),
	            static_cast<long>(g_3ds.maxMeshNs / 1000000LL),
	            g_3ds.unloadSaveCount,
	            static_cast<long>(g_3ds.unloadSaveCount ? g_3ds.unloadSaveNs / g_3ds.unloadSaveCount / 1000000LL : 0),
	            static_cast<long>(g_3ds.maxUnloadSaveNs / 1000000LL),
	            static_cast<long>(g_3ds.ticks ? g_3ds.tickUpdatesNs / g_3ds.ticks / 1000000LL : 0),
	            static_cast<long>(g_3ds.maxTickUpdatesNs / 1000000LL),
	            static_cast<long>(g_3ds.ticks ? g_3ds.tickQueueSum / g_3ds.ticks : 0),
	            static_cast<long>(g_3ds.tickQueueMax),
	            static_cast<long>(g_3ds.ticks ? g_3ds.mobSpawnNs / g_3ds.ticks / 1000000LL : 0),
	            static_cast<long>(g_3ds.maxMobSpawnNs / 1000000LL),
	            static_cast<long>(g_3ds.ticks ? g_3ds.saveInfoNs / g_3ds.ticks / 1000000LL : 0),
	            static_cast<long>(g_3ds.maxSaveInfoNs / 1000000LL),
	            static_cast<long>(g_3ds.ticks ? g_3ds.mapStorageNs / g_3ds.ticks / 1000000LL : 0),
	            static_cast<long>(g_3ds.maxMapStorageNs / 1000000LL));
#endif
	resetWindow(now);
}
}
