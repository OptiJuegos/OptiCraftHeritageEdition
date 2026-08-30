#include "client/ClientProfiler.h"

#include "platform/ClientProfilerBackend.h"
#include "platform/Profiler.h"

namespace ClientProfiler
{
void frameBegin()
{
    ClientProfilerBackend::frameBegin();
}

void ticks(long long ns, int ticksThisFrame)
{
    ClientProfilerBackend::ticks(ns, ticksThisFrame);
}

void lighting(long long ns)
{
    ClientProfilerBackend::lighting(ns);
}

void displayUpdate(long long ns)
{
    ClientProfilerBackend::displayUpdate(ns);
}

void render(long long ns)
{
    ClientProfilerBackend::render(ns);
}

void tickPhase(const char* name, long long ns)
{
    platformProfileTickPhase(name, ns);
}

void frameEnd(long long frameNs, long long tickNs, long long renderNs,
              int ticksThisFrame, int chunkUpdates,
              World* world, RenderGlobal* renderGlobal)
{
    ClientProfilerBackend::frameEnd(frameNs, tickNs, renderNs,
                                    ticksThisFrame, chunkUpdates,
                                    world, renderGlobal);
}
}
