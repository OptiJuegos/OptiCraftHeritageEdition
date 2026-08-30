#pragma once

#include <functional>
#include <typeindex>
#include <vector>
#include "java/Type.h"
#include "java/HashSet.h"
#include "ChunkCoordIntPair.h"

class World;
class EntityLiving;
class EnumCreatureType;
class Random;

struct SpawnerChunkHash
{
    std::uint32_t operator()(const ChunkCoordIntPair &pair) const
    {
        return static_cast<std::uint32_t>(pair.hashCode());
    }
};

struct SpawnerChunkEqual
{
    bool operator()(const ChunkCoordIntPair &a, const ChunkCoordIntPair &b) const
    {
        return a == b;
    }
};

// Register a factory so SpawnerAnimals can instantiate that entity type by type_index.
// Must be called before performSpawning / performSleepSpawning.
void SpawnerAnimals_registerFactory(std::type_index idx, std::function<EntityLiving *(World *)> factory);

// net.minecraft.src.SpawnerAnimals
class SpawnerAnimals
{
public:
    SpawnerAnimals() = default;

    static int_t performSpawning(World *world, bool spawnHostiles, bool spawnPeaceful);
    static void performWorldGenSpawning(World *world, class BiomeGenBase *biome,
                                        int_t x, int_t z, int_t width, int_t depth, Random &random);
    static bool performSleepSpawning(World *world, std::vector<class EntityPlayer*> &list);
    static bool canCreatureTypeSpawnAtLocation(const EnumCreatureType &type, World *world,
                                                int_t i, int_t j, int_t k);

protected:
    static class ChunkPosition getRandomSpawningPointInChunk(World *world, int_t chunkX, int_t chunkZ);

private:
    static void creatureSpecificInit(EntityLiving *entity, World *world, float x, float y, float z);

    static JavaHashSet<ChunkCoordIntPair, SpawnerChunkHash, SpawnerChunkEqual> eligibleChunksForSpawning;
    static JavaHashSet<ChunkCoordIntPair, SpawnerChunkHash, SpawnerChunkEqual> interiorChunksForSpawning;
    static const int_t nightSpawnEntityCount = 3;
};
