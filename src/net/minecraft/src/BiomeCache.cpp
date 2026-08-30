#include "BiomeCache.h"

#include "BiomeCacheBlock.h"
#include "java/System.h"
#include "java/Arithmetic.h"

#include <cstddef>

BiomeCache::BiomeCache(WorldChunkManager *manager)
    : chunkManager(manager),
      lastCleanupTime(0)
{
}

BiomeCache::~BiomeCache() = default;

ulong_t BiomeCache::makeKey(int_t chunkX, int_t chunkZ)
{
    return static_cast<ulong_t>(static_cast<uint_t>(chunkX)) |
           (static_cast<ulong_t>(static_cast<uint_t>(chunkZ)) << 32);
}

BiomeCacheBlock *BiomeCache::getBiomeCacheBlock(int_t x, int_t z)
{
    const int_t chunkX = JavaArithmetic::intShr(x, 4);
    const int_t chunkZ = JavaArithmetic::intShr(z, 4);
    const ulong_t key = makeKey(chunkX, chunkZ);

    auto found = cacheMap.find(key);
    if (found == cacheMap.end())
    {
        auto block = std::make_unique<BiomeCacheBlock>(this, chunkX, chunkZ);
        BiomeCacheBlock *raw = block.get();
        cacheMap.emplace(key, std::move(block));
        cacheOrder.push_back(key);
        found = cacheMap.find(key);
        raw->lastAccessTime = System::currentTimeMillis();
        return raw;
    }

    found->second->lastAccessTime = System::currentTimeMillis();
    return found->second.get();
}

BiomeGenBase *BiomeCache::getBiomeGenAt(int_t x, int_t z)
{
    return getBiomeCacheBlock(x, z)->getBiomeGenAt(x, z);
}

void BiomeCache::cleanupCache()
{
    const long_t now = System::currentTimeMillis();
    const long_t sinceCleanup = JavaArithmetic::longSub(now, lastCleanupTime);
    if (sinceCleanup <= 7500LL && sinceCleanup >= 0LL)
        return;

    lastCleanupTime = now;
    for (std::size_t index = 0; index < cacheOrder.size();)
    {
        const ulong_t key = cacheOrder[index];
        auto found = cacheMap.find(key);
        if (found == cacheMap.end())
        {
            cacheOrder.erase(cacheOrder.begin() + static_cast<std::ptrdiff_t>(index));
            continue;
        }

        const long_t age = JavaArithmetic::longSub(now, found->second->lastAccessTime);
        if (age > 30000LL || age < 0LL)
        {
            cacheMap.erase(found);
            cacheOrder.erase(cacheOrder.begin() + static_cast<std::ptrdiff_t>(index));
        }
        else
        {
            ++index;
        }
    }
}

const std::vector<BiomeGenBase *> &BiomeCache::getCachedBiomes(int_t x, int_t z)
{
    return getBiomeCacheBlock(x, z)->biomes;
}
