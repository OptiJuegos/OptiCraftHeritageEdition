#include "net/minecraft/src/RenderList.h"

#include "net/minecraft/src/WorldRenderer.h"
#include "platform/RenderAPI.h"
#include "platform/RenderTerrainAPI.h"

RenderList::RenderList()
{
    originX = 0;
    originY = 0;
    originZ = 0;
    viewerX = 0.0;
    viewerY = 0.0;
    viewerZ = 0.0;
    initialized = false;
}

void RenderList::setup(int i, int j, int k, double d, double d1, double d2)
{
    initialized = true;
    terrainEntries.clear();
    originX = i;
    originY = j;
    originZ = k;
    viewerX = d;
    viewerY = d1;
    viewerZ = d2;
}

bool RenderList::matchesPos(int i, int j, int k)
{
    return initialized && i == originX && j == originY && k == originZ;
}

void RenderList::addTerrainRenderer(WorldRenderer *renderer, int_t pass)
{
    if (renderer == nullptr || pass < 0 || pass > 1)
        return;
    terrainEntries.push_back({renderer, pass});
}

void RenderList::submitTerrainGroup(TerrainGroup group)
{
    bool nativeBatchActive = false;
    for (const TerrainRenderEntry &entry : terrainEntries)
    {
        WorldRenderer *renderer = entry.renderer;
        if (renderer == nullptr)
            continue;

        // Only the opaque pass splits. Pass 1 is blended, so its submission
        // order is its draw order and must not be rearranged; all of its
        // entries fall into Remaining and keep the order they were added in.
        const bool earlyDepth = entry.pass == 0 && !renderer->terrainPassNeedsAlphaTest(entry.pass);
        if (earlyDepth != (group == TerrainGroup::EarlyDepth))
            continue;

        const int nativeHandle = renderer->getTerrainHandleForPass(entry.pass);
        if (nativeHandle <= 0)
            continue;

        if (!nativeBatchActive)
        {
            nativeBatchActive = renderTerrainBeginChunkBatch(nativeHandle);
            if (!nativeBatchActive)
                continue;
        }

        if (renderTerrainAppendChunk(nativeHandle))
            continue;

        renderTerrainEndChunkBatch();
        nativeBatchActive = false;
    }
    if (nativeBatchActive)
        renderTerrainEndChunkBatch();
}

void RenderList::render()
{
    if (!initialized)
        return;
    if (terrainEntries.empty())
        return;

    const float translateX = static_cast<float>(static_cast<double>(originX) - viewerX);
    const float translateY = static_cast<float>(static_cast<double>(originY) - viewerY);
    const float translateZ = static_cast<float>(static_cast<double>(originZ) - viewerZ);
    renderPushMatrix();
    renderTranslate(translateX, translateY, translateZ);

    // Two sweeps rather than one, because the alpha test is programmed per batch
    // and not per section. The sections that can do without it go first: they
    // draw with early depth rejection, and they lay depth down for the cutout
    // group behind them. Relative order is preserved inside each sweep, so the
    // opaque pass stays front-to-back within each group.
    renderTerrainSetEarlyDepth(true);
    submitTerrainGroup(TerrainGroup::EarlyDepth);
    renderTerrainSetEarlyDepth(false);
    submitTerrainGroup(TerrainGroup::Remaining);

    // CTM atlases are captured separately from the terrain native list and must
    // replay after the GX batch so texture switches do not fragment that batch.
    for (const TerrainRenderEntry &entry : terrainEntries)
    {
        if (entry.renderer != nullptr)
            entry.renderer->renderExtraTerrainMeshes(entry.pass);
    }

    renderPopMatrix();
}

void RenderList::reset()
{
    initialized = false;
    terrainEntries.clear();
}
