#include "platform/Log.h"
#include "platform/PlatformTuning.h"
#include "MetadataChunkBlock.h"
#include "java/Arithmetic.h"

#include "World.h"
#include "Chunk.h"
#include "WorldHeight.h"
#include "Block.h"
#include "EnumSkyBlock.h"
#include <cstdio>

MetadataChunkBlock::MetadataChunkBlock(EnumSkyBlock *skyBlock, int_t minX, int_t minY, int_t minZ,
                                       int_t maxX, int_t maxY, int_t maxZ) :
    skyBlock(skyBlock),
    minX(minX), minY(minY), minZ(minZ),
    maxX(maxX), maxY(maxY), maxZ(maxZ)
{
}

void MetadataChunkBlock::updateLight(World *world)
{
    int_t dx = (maxX - minX) + 1;
    int_t dy = (maxY - minY) + 1;
    int_t dz = (maxZ - minZ) + 1;
    int_t total = dx * dy * dz;

    if (total > 32768)
    {
        MC_LOG_WARN("chunk", "Light too large, skipping!\n");
        return;
    }

    int_t prevChunkX = 0, prevChunkZ = 0;
    bool hasPrev = false;
    bool prevValid = false;

#if PLATFORM_CONSOLE_LOW
    // Resolve the current chunk once and reuse it for the hot light/block/sky
    // reads below. Neighbor light probes cross chunk borders frequently, so the
    // one-entry cache is keyed by chunk coordinates rather than by world column.
    Chunk *cachedChunk = nullptr;
    int_t cachedChunkX = 0, cachedChunkZ = 0;
    bool cacheValid = false;
    auto resolveChunk = [&](int_t lx, int_t lz) -> Chunk *
    {
        if (lx < -30000000 || lz < -30000000 || lx >= 30000000 || lz >= 30000000)
            return nullptr;

        const int_t cx = JavaArithmetic::intShr(lx, 4);
        const int_t cz = JavaArithmetic::intShr(lz, 4);
        if (!cacheValid || cx != cachedChunkX || cz != cachedChunkZ)
        {
            cachedChunkX = cx;
            cachedChunkZ = cz;
            cacheValid = true;
            cachedChunk = world->blockExists(lx, 64, lz)
                ? world->getChunkFromChunkCoords(cx, cz)
                : nullptr;
        }
        return cachedChunk;
    };

    auto fastSavedLight = [&](int_t lx, int_t ly, int_t lz) -> int_t
    {
        if (ly < 0) ly = 0;
        else if (ly >= WorldHeight::HEIGHT) ly = WorldHeight::MAX_Y;

        Chunk *chunk = resolveChunk(lx, lz);
        return chunk != nullptr
            ? chunk->getSavedLightValue(const_cast<EnumSkyBlock*>(skyBlock), lx & 0xf, ly, lz & 0xf)
            : skyBlock->defaultLightValue;
    };

    auto fastBlockId = [&](int_t lx, int_t ly, int_t lz) -> int_t
    {
        if (ly < 0 || ly >= WorldHeight::HEIGHT)
            return 0;
        Chunk *chunk = resolveChunk(lx, lz);
        return chunk != nullptr ? chunk->getBlockID(lx & 0xf, ly, lz & 0xf) : 0;
    };

    auto fastCanSeeSky = [&](int_t lx, int_t ly, int_t lz) -> bool
    {
        if (ly < 0)
            return false;
        if (ly >= WorldHeight::HEIGHT)
            return true;
        Chunk *chunk = resolveChunk(lx, lz);
        return chunk != nullptr && chunk->canBlockSeeTheSky(lx & 0xf, ly, lz & 0xf);
    };

    auto fastSetLight = [&](int_t lx, int_t ly, int_t lz, int_t lightValue)
    {
        if (ly < 0 || ly >= WorldHeight::HEIGHT)
            return;
        Chunk *chunk = resolveChunk(lx, lz);
        if (chunk == nullptr)
            return;

        chunk->setLightValue(const_cast<EnumSkyBlock*>(skyBlock), lx & 0xf, ly, lz & 0xf, lightValue);
        world->func_48464_p(lx, ly, lz);
    };

    auto fastNeighborPropagation = [&](int_t lx, int_t ly, int_t lz, int_t lightValue)
    {
        if (ly < 0 || ly >= WorldHeight::HEIGHT)
            return;
        Chunk *chunk = resolveChunk(lx, lz);
        if (chunk == nullptr)
            return;

        if (skyBlock == EnumSkyBlock::Sky)
        {
            if (chunk->canBlockSeeTheSky(lx & 0xf, ly, lz & 0xf))
                lightValue = 15;
        }
        else if (skyBlock == EnumSkyBlock::Block)
        {
            const int_t blockId = chunk->getBlockID(lx & 0xf, ly, lz & 0xf);
            if (Block::lightValue[blockId] > lightValue)
                lightValue = Block::lightValue[blockId];
        }

        if (chunk->getSavedLightValue(const_cast<EnumSkyBlock*>(skyBlock), lx & 0xf, ly, lz & 0xf) != lightValue)
            world->scheduleLightingUpdate(const_cast<EnumSkyBlock*>(skyBlock), lx, ly, lz, lx, ly, lz);
    };
#else
    auto fastSavedLight = [&](int_t lx, int_t ly, int_t lz) -> int_t
    {
        return world->getSavedLightValue(const_cast<EnumSkyBlock*>(skyBlock), lx, ly, lz);
    };
    auto fastBlockId = [&](int_t lx, int_t ly, int_t lz) -> int_t
    {
        return world->getBlockId(lx, ly, lz);
    };
    auto fastCanSeeSky = [&](int_t lx, int_t ly, int_t lz) -> bool
    {
        return world->canExistingBlockSeeTheSky(lx, ly, lz);
    };
    auto fastSetLight = [&](int_t lx, int_t ly, int_t lz, int_t lightValue)
    {
        world->setLightValue(const_cast<EnumSkyBlock*>(skyBlock), lx, ly, lz, lightValue);
    };
    auto fastNeighborPropagation = [&](int_t lx, int_t ly, int_t lz, int_t lightValue)
    {
        world->neighborLightPropagationChanged(const_cast<EnumSkyBlock*>(skyBlock), lx, ly, lz, lightValue);
    };
#endif

    for (int_t x = minX; x <= maxX; x++)
    {
        for (int_t z = minZ; z <= maxZ; z++)
        {
            int_t chunkX = JavaArithmetic::intShr(x, 4);
            int_t chunkZ = JavaArithmetic::intShr(z, 4);
            bool valid = false;

            if (hasPrev && chunkX == prevChunkX && chunkZ == prevChunkZ)
            {
                valid = prevValid;
            }
            else
            {
                valid = world->doChunksNearChunkExist(x, 0, z, 1);
                if (valid)
                {
                    Chunk *chunk = world->getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
                    if (chunk->isEmptyChunk())
                        valid = false;
                }
                prevValid = valid;
                prevChunkX = chunkX;
                prevChunkZ = chunkZ;
                hasPrev = true;
            }

            if (!valid) continue;

            int_t localMinY = minY;
            int_t localMaxY = maxY;
            if (localMinY < 0) localMinY = 0;
            if (localMaxY >= WorldHeight::HEIGHT) localMaxY = WorldHeight::MAX_Y;

            for (int_t y = localMinY; y <= localMaxY; y++)
            {
                int_t currentLight = fastSavedLight(x, y, z);
                int_t newLight = 0;
                int_t blockId = fastBlockId(x, y, z);
                int_t opacity = Block::lightOpacity[blockId];
                if (opacity == 0) opacity = 1;

                if (skyBlock == EnumSkyBlock::Sky)
                {
                    if (fastCanSeeSky(x, y, z))
                        newLight = 15;
                }
                else if (skyBlock == EnumSkyBlock::Block)
                {
                    newLight = Block::lightValue[blockId];
                }

                if (opacity < 15 || newLight > 0)
                {
                    int_t n = fastSavedLight(x - 1, y, z);
                    int_t best = n;
                    n = fastSavedLight(x + 1, y, z);
                    if (n > best) best = n;
                    n = fastSavedLight(x, y - 1, z);
                    if (n > best) best = n;
                    n = fastSavedLight(x, y + 1, z);
                    if (n > best) best = n;
                    n = fastSavedLight(x, y, z - 1);
                    if (n > best) best = n;
                    n = fastSavedLight(x, y, z + 1);
                    if (n > best) best = n;

                    best -= opacity;
                    if (best < 0) best = 0;
                    if (newLight > best) best = newLight;
                    newLight = best;
                }

                if (currentLight == newLight) continue;

                fastSetLight(x, y, z, newLight);
                int_t spread = newLight - 1;
                if (spread < 0) spread = 0;

                fastNeighborPropagation(x - 1, y, z, spread);
                fastNeighborPropagation(x, y - 1, z, spread);
                fastNeighborPropagation(x, y, z - 1, spread);

                if (x + 1 >= maxX)
                    fastNeighborPropagation(x + 1, y, z, spread);
                if (y + 1 >= maxY)
                    fastNeighborPropagation(x, y + 1, z, spread);
                if (z + 1 >= maxZ)
                    fastNeighborPropagation(x, y, z + 1, spread);
            }
        }
    }
}

bool MetadataChunkBlock::tryMerge(int_t inMinX, int_t inMinY, int_t inMinZ,
                                   int_t inMaxX, int_t inMaxY, int_t inMaxZ)
{
    // Exact containment check
    if (inMinX >= minX && inMinY >= minY && inMinZ >= minZ &&
        inMaxX <= maxX && inMaxY <= maxY && inMaxZ <= maxZ)
        return true;

    // Near-containment check with small expansion
    const int_t expand = 1;
    if (inMinX >= minX - expand && inMinY >= minY - expand && inMinZ >= minZ - expand &&
        inMaxX <= maxX + expand && inMaxY <= maxY + expand && inMaxZ <= maxZ + expand)
    {
        int_t oldVol = (maxX - minX) * (maxY - minY) * (maxZ - minZ);

        int_t newMinX = inMinX < minX ? inMinX : minX;
        int_t newMinY = inMinY < minY ? inMinY : minY;
        int_t newMinZ = inMinZ < minZ ? inMinZ : minZ;
        int_t newMaxX = inMaxX > maxX ? inMaxX : maxX;
        int_t newMaxY = inMaxY > maxY ? inMaxY : maxY;
        int_t newMaxZ = inMaxZ > maxZ ? inMaxZ : maxZ;

        int_t newVol = (newMaxX - newMinX) * (newMaxY - newMinY) * (newMaxZ - newMinZ);

        if (newVol - oldVol <= 2)
        {
            minX = newMinX; minY = newMinY; minZ = newMinZ;
            maxX = newMaxX; maxY = newMaxY; maxZ = newMaxZ;
            return true;
        }
    }

    return false;
}
