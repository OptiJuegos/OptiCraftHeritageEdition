#ifdef WII_PLATFORM

#include "net/minecraft/src/RenderGlobal.h"

#include "net/minecraft/src/EntityLiving.h"
#include "net/minecraft/src/WorldRenderer.h"
#include "platform/PlatformTuning.h"
#include "platform/RenderTerrainAPI.h"

void RenderGlobal::evictWiiMeshCache(EntityLiving *entityliving)
{
    if (worldRenderers == nullptr || entityliving == nullptr)
        return;

    const unsigned int listBytes = renderTerrainLiveBytes();
    if (listBytes < PLATFORM_WII_GXLIST_EVICT_HIGH_WATER_BYTES)
        return;

    const float keepSq = PLATFORM_WII_GXLIST_EVICT_KEEP_RADIUS_BLOCKS *
                         PLATFORM_WII_GXLIST_EVICT_KEEP_RADIUS_BLOCKS;
    for (int released = 0; released < PLATFORM_WII_GXLIST_EVICT_MAX_PER_FRAME; ++released)
    {
        WorldRenderer *victim = nullptr;
        float farthest = keepSq;
        const int_t total = renderChunksWide * renderChunksTall * renderChunksDeep;
        for (int_t i = 0; i < total; ++i)
        {
            WorldRenderer *candidate = worldRenderers[i];
            if (candidate == nullptr || candidate->needsUpdate || candidate->isInFrustum)
                continue;

            // A section with nothing recorded cannot be a victim, and admitting
            // one does not merely waste a slot -- it ends the whole sweep. The
            // scan keeps a single farthest candidate and the loop below breaks
            // as soon as releasing it reports nothing freed, so one empty
            // section farther away than the geometry we wanted to evict is
            // enough to return having freed zero bytes with the cache still over
            // its high-water mark. In a 17x16x17 grid most sections are air
            // above the terrain, they publish both skip flags once meshed, and
            // they sit out of frustum at the edges -- so the farthest candidate
            // was almost always one of them.
            if (!candidate->holdsRecordedTerrain())
                continue;

            const float distance = candidate->distanceToEntitySquared(entityliving);
            if (distance > farthest)
            {
                farthest = distance;
                victim = candidate;
            }
        }

        if (victim == nullptr || !victim->releaseDisplayListsForCache())
            break;
    }
}

#endif // WII_PLATFORM
