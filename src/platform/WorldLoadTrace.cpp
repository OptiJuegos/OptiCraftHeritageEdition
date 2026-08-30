#include "platform/WorldLoadTrace.h"

#include "platform/Diagnostics.h"
#include "platform/Log.h"
#include "java/System.h"

#include <cstdio>

namespace
{
// The deepest real chain is startWorld -> World ctor -> initialSpawn ->
// generateSpawnPoint -> canCoordinateBeSpawn -> prepareChunk -> generate ->
// provideChunk -> phase, so this has headroom over it and still costs a fixed
// amount of static state. Overflowing must not lose the path, so pushes past the
// top are counted and popped symmetrically instead of being dropped.
constexpr int kMaxDepth = 12;

struct Stage
{
    const char* name;
    long_t startNs;
    long freeKb;
    bool isStep;
};

Stage s_stack[kMaxDepth];
int s_depth = 0;
int s_overflow = 0;

bool s_active = false;
long_t s_loadStartNs = 0;

// Rebuilt on every push and pop so a reader -- the OOM screen, a debugger, a
// watchdog -- never has to walk the stack itself.
char s_path[192] = {};

void rebuildPath()
{
    std::size_t used = 0;
    s_path[0] = '\0';
    for (int i = 0; i < s_depth; ++i)
    {
        const char* name = s_stack[i].name != nullptr ? s_stack[i].name : "?";
        const int written = std::snprintf(s_path + used, sizeof(s_path) - used,
                                          i == 0 ? "%s" : ".%s", name);
        if (written <= 0 || (std::size_t)written >= sizeof(s_path) - used)
            return;
        used += (std::size_t)written;
    }
}

// Two spaces per level, so nested stages read as a tree in debug.log.
const char* indent(int depth)
{
    static const char spaces[] = "                        ";
    const int width = depth * 2;
    const int max = (int)sizeof(spaces) - 1;
    return spaces + (max - (width < max ? width : max));
}
}

namespace WorldLoadTrace
{

void beginLoad(const char* what)
{
    s_depth = 0;
    s_overflow = 0;
    s_active = true;
    s_loadStartNs = System::nanoTime();
    rebuildPath();

    MC_LOG_DEBUG("worldload", "=== %s begin ===\n", what != nullptr ? what : "world load");
    platformMemoryCheckpoint("worldload begin");
}

void endLoad()
{
    if (!s_active)
        return;

    // Close whatever is still open so the last step reports its duration like
    // every other one, instead of vanishing when the stack is reset.
    popTo(0);

    const long ms = elapsedMs();
    s_active = false;
    rebuildPath();

    MC_LOG_DEBUG("worldload", "=== world load end, %ldms ===\n", ms);
    platformMemoryCheckpoint("worldload end");
}

bool active()
{
    return s_active;
}

void begin(const char* stage)
{
    // Outside a load the trace is inert. Sites that also run during play --
    // provideChunk above all -- can therefore be marked up unconditionally
    // instead of each guarding on active(); the counter keeps end() balanced.
    if (!s_active || s_depth >= kMaxDepth)
    {
        ++s_overflow;
        return;
    }

    Stage& entry = s_stack[s_depth];
    entry.name = stage;
    entry.startNs = System::nanoTime();
    entry.freeKb = platformHeapFreeKb();
    entry.isStep = false;
    ++s_depth;
    rebuildPath();

    // Entry lines double the cost of the trace on a console, where each line is
    // an fopen/fclose. They are what names the stage a hang stopped inside, so
    // they belong to the trace level the hang-trace build already turns on.
    MC_LOG_TRACE("worldload", "%s> %s\n", indent(s_depth - 1), stage != nullptr ? stage : "?");
}

int depth()
{
    return s_depth + s_overflow;
}

void popTo(int target)
{
    if (target < 0)
        target = 0;
    while (depth() > target)
        end();
}

void step(const char* name)
{
    if (s_overflow == 0 && s_depth > 0 && s_stack[s_depth - 1].isStep)
        end();

    begin(name);

    if (s_overflow == 0 && s_depth > 0)
        s_stack[s_depth - 1].isStep = true;
}

void end()
{
    if (s_overflow > 0)
    {
        --s_overflow;
        return;
    }
    if (s_depth <= 0)
        return;

    --s_depth;
    const Stage& entry = s_stack[s_depth];
    const long ms = (long)((System::nanoTime() - entry.startNs) / 1000000LL);
    const long freeKb = platformHeapFreeKb();

    if (freeKb >= 0 && entry.freeKb >= 0)
    {
        MC_LOG_DEBUG("worldload", "%s%-22s %5ldms free=%ldKB (%+ldKB)\n",
                     indent(s_depth), entry.name != nullptr ? entry.name : "?",
                     ms, freeKb, freeKb - entry.freeKb);
    }
    else
    {
        MC_LOG_DEBUG("worldload", "%s%-22s %5ldms\n",
                     indent(s_depth), entry.name != nullptr ? entry.name : "?", ms);
    }

    rebuildPath();
}

const char* currentStage()
{
    return s_path;
}

long elapsedMs()
{
    if (!s_active)
        return 0;
    return (long)((System::nanoTime() - s_loadStartNs) / 1000000LL);
}

}
