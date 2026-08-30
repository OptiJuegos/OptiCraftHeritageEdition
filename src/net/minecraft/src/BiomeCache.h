#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "java/Type.h"

class BiomeCacheBlock;
class BiomeGenBase;
class WorldChunkManager;

// net.minecraft.src.BiomeCache
class BiomeCache
{
public:
    explicit BiomeCache(WorldChunkManager *manager);
    ~BiomeCache();

    BiomeCacheBlock *getBiomeCacheBlock(int_t x, int_t z);
    BiomeGenBase *getBiomeGenAt(int_t x, int_t z);
    void cleanupCache();
    const std::vector<BiomeGenBase *> &getCachedBiomes(int_t x, int_t z);

    WorldChunkManager *getChunkManager() const { return chunkManager; }

private:
    static ulong_t makeKey(int_t chunkX, int_t chunkZ);

    WorldChunkManager *chunkManager;
    long_t lastCleanupTime;
    std::unordered_map<ulong_t, std::unique_ptr<BiomeCacheBlock>> cacheMap;
    std::vector<ulong_t> cacheOrder;
};
