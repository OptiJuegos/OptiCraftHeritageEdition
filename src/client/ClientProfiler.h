#pragma once

class World;
class RenderGlobal;

namespace ClientProfiler
{
    void frameBegin();
    void ticks(long long ns, int ticksThisFrame);
    void lighting(long long ns);
    void displayUpdate(long long ns);
    void render(long long ns);
    void tickPhase(const char* name, long long ns);
    void frameEnd(long long frameNs, long long tickNs, long long renderNs,
                  int ticksThisFrame, int chunkUpdates,
                  World* world, RenderGlobal* renderGlobal);
}
