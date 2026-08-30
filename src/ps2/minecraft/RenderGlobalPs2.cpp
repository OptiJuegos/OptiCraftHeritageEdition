#ifdef PS2_PLATFORM

#include "net/minecraft/src/RenderGlobal.h"
#include "net/minecraft/src/WorldRenderer.h"
#include "platform/RenderTerrainStaging.h"

size_t RenderGlobal::terrainMeshRamBytes() const
{
    RenderTerrainMeshRam ram;
    terrainMeshRamBreakdown(ram);
    return ram.total();
}

void RenderGlobal::terrainMeshRamBreakdown(RenderTerrainMeshRam &out) const
{
    out.stagingPool += renderTerrainStagingBytes();
    out.stagingPool += WorldRenderer::sharedOpaquePublishScratchRamBytes();

    if (worldRenderers == nullptr)
        return;

    const int_t total = renderChunksWide * renderChunksTall * renderChunksDeep;
    for (int_t i = 0; i < total; ++i)
    {
        if (worldRenderers[i] != nullptr)
            worldRenderers[i]->addTerrainMeshRam(out);
    }
}

#endif // PS2_PLATFORM
