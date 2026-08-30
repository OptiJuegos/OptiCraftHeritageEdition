#pragma once

#ifdef PS2_PLATFORM

// ChunkProviderGenerate::populate attribution. TOTAL increments the number of
// decorated chunks; the remaining values partition one call so the periodic
// PS2 frame line can identify the synchronous hitch precisely.
enum Ps2PopulatePhase
{
	PS2_POP_TOTAL = 0,
	PS2_POP_LAKES,
	PS2_POP_DUNGEONS,
	PS2_POP_FILLERS,
	PS2_POP_ORES,
	PS2_POP_DECORATION,
	PS2_POP_SPRINGS,
	PS2_POP_SNOW,
	PS2_POP_PHASE_COUNT
};

extern "C" void ps2_perf_add_populate_phase(int phase, long long ns);

#endif // PS2_PLATFORM
