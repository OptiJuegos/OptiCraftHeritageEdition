#ifndef RENDERLIST_H
#define RENDERLIST_H

#include <vector>
#include "java/Type.h"
#include "platform/PlatformConfig.h"

class WorldRenderer;

// net.minecraft.src.RenderList
#if !PLATFORM_PS2
class RenderList {
public:
    RenderList();

    void setup(int i, int j, int k, double d, double d1, double d2);
    bool matchesPos(int i, int j, int k);
    void addTerrainRenderer(WorldRenderer *renderer, int_t pass);
    void render();
    void reset();

private:
    int originX;
    int originY;
    int originZ;
    double viewerX;
    double viewerY;
    double viewerZ;
#if PLATFORM_PC
    std::vector<int_t> displayListIds;
#elif PLATFORM_WII
    struct TerrainRenderEntry
    {
        WorldRenderer *renderer;
        int_t pass;
    };
    std::vector<TerrainRenderEntry> terrainEntries;

    // Which half of the list one submission sweep covers. The opaque sections
    // that need no alpha test are batched apart from everything else so GX can
    // be told, once per group, whether early depth rejection is legal.
    enum class TerrainGroup
    {
        EarlyDepth,
        Remaining
    };
    void submitTerrainGroup(TerrainGroup group);
#endif
    bool initialized;
};
#endif

#endif
