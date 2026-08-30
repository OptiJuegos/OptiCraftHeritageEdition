#include "SpawnerAnimals.h"
#include "java/Arithmetic.h"

#include "platform/PlatformTuning.h"

#include <functional>
#include <map>
#include <typeindex>
#include <cmath>
#include <cstdio>
#include <memory>

#include "World.h"
#include "Chunk.h"
#include "Entity.h"
#include "EntityPlayer.h"
#include "EntityLiving.h"
#include "ChunkPosition.h"
#include "ChunkCoordIntPair.h"
#include "ChunkCoordinates.h"
#include "EnumCreatureType.h"
#include "SpawnListEntry.h"
#include "BiomeGenBase.h"
#include "MathHelper.h"
#include "Material.h"
#include "BlockBed.h"
#include "PathFinder.h"
#include "PathEntity.h"
#include "PathPoint.h"

// Entity-specific headers for creatureSpecificInit
#include "EntitySpider.h"
#include "EntitySkeleton.h"
#include "EntitySheep.h"
#include "EntityZombie.h"
#include "EntityOcelot.h"
#include "WorldChunkManager.h"
#include "WorldHeight.h"

// Construct-on-first-use: avoids static init order fiasco when EntityList registers
// factories before this TU's statics are initialized.
static std::map<std::type_index, std::function<EntityLiving*(World*)>>& getEntityFactory()
{
    static std::map<std::type_index, std::function<EntityLiving*(World*)>> instance;
    return instance;
}

void SpawnerAnimals_registerFactory(std::type_index idx, std::function<EntityLiving*(World*)> factory)
{
    getEntityFactory()[idx] = factory;
}

JavaHashSet<ChunkCoordIntPair, SpawnerChunkHash, SpawnerChunkEqual> SpawnerAnimals::eligibleChunksForSpawning;
JavaHashSet<ChunkCoordIntPair, SpawnerChunkHash, SpawnerChunkEqual> SpawnerAnimals::interiorChunksForSpawning;

#if PLATFORM_BOUNDED_WORLD
// True when the 3x3-block neighbourhood of column (x,z) lies entirely in
// chunks that are ALREADY generated. Every world query in the spawn path
// (isBlockNormalCube, getBlockMaterial, getCanSpawnHere's light/collision
// checks) goes through getBlockId -> provideChunk, which force-generates a
// missing chunk. On PC the whole eligible area is always loaded so this never
// triggers, but the PS2 cache is ~5x5 and the cluster spread (+-6 blocks per
// sub-attempt, cumulative) walks off it: each stray probe generated a chunk
// (hundreds of KB of transient + resident allocations) that the unloader
// evicted next tick. That generate/evict loop was the RAM burst behind the
// idle "Out of memory" (mallocUsed +350KB in seconds whenever mobSpawn ran).
static bool spawnColumnResident(World *world, int_t x, int_t z)
{
    const int_t cx0 = JavaArithmetic::intShr(JavaArithmetic::intSub(x, 1), 4);
    const int_t cx1 = JavaArithmetic::intShr(JavaArithmetic::intAdd(x, 1), 4);
    const int_t cz0 = JavaArithmetic::intShr(JavaArithmetic::intSub(z, 1), 4);
    const int_t cz1 = JavaArithmetic::intShr(JavaArithmetic::intAdd(z, 1), 4);
    for (int_t cx = cx0; cx <= cx1; cx++)
        for (int_t cz = cz0; cz <= cz1; cz++)
            if (!world->chunkExists(cx, cz))
                return false;
    return true;
}
#endif

ChunkPosition SpawnerAnimals::getRandomSpawningPointInChunk(World *world, int_t i, int_t j)
{
    Chunk *chunk = world->getChunkFromChunkCoords(i, j);
    int_t k = JavaArithmetic::intAdd(JavaArithmetic::intMul(i, 16), world->rand.nextInt(16));
    int_t top = chunk == nullptr ? 128 : std::max<int_t>(128, chunk->getTopFilledSegment());
    if (top <= 0)
        top = 128;
    int_t l = world->rand.nextInt(top);
    int_t i1 = JavaArithmetic::intAdd(JavaArithmetic::intMul(j, 16), world->rand.nextInt(16));
    return ChunkPosition(k, l, i1);
}

int_t SpawnerAnimals::performSpawning(World *world, bool spawnHostiles, bool spawnPeaceful)
{
    if (!spawnHostiles && !spawnPeaceful)
        return 0;

#if PLATFORM_CONSOLE_LOW || PLATFORM_WII
    int_t liveMobs = world->countEntities(EnumCreatureTypeTag::monster_tag)
                   + world->countEntities(EnumCreatureTypeTag::creature_tag)
                   + world->countEntities(EnumCreatureTypeTag::waterCreature_tag);

    static int_t s_spawnPhase = 0;
    if (++s_spawnPhase < PLATFORM_MOB_SPAWN_INTERVAL_TICKS)
        return 0;
    s_spawnPhase = 0;

    if (liveMobs >= PLATFORM_MAX_LIVE_MOBS)
        return 0;
#endif

    eligibleChunksForSpawning.clear();
    interiorChunksForSpawning.clear();

    for (EntityPlayer *player : world->playerEntities)
    {
        if (player == nullptr)
            continue;

        const int_t playerChunkX = MathHelper::floor_double(player->posX / 16.0);
        const int_t playerChunkZ = MathHelper::floor_double(player->posZ / 16.0);
#if PLATFORM_BOUNDED_WORLD
        const int_t radius = PLATFORM_CHUNK_CACHE_RADIUS;
#else
        const int_t radius = 8;
#endif

        for (int_t dx = -radius; dx <= radius; ++dx)
        {
            for (int_t dz = -radius; dz <= radius; ++dz)
            {
                const bool border = dx == -radius || dx == radius || dz == -radius || dz == radius;
                const ChunkCoordIntPair pair(JavaArithmetic::intAdd(dx, playerChunkX),
                                             JavaArithmetic::intAdd(dz, playerChunkZ));
                eligibleChunksForSpawning.add(pair);
                if (!border)
                    interiorChunksForSpawning.add(pair);
            }
        }
    }

    int_t totalSpawned = 0;
    const ChunkCoordinates spawnPoint = world->getSpawnPoint();
    const EnumCreatureType *creatureTypes[] = {
        &EnumCreatureType::monster,
        &EnumCreatureType::creature,
        &EnumCreatureType::waterCreature
    };
    const std::vector<ChunkCoordIntPair> eligibleChunks =
        eligibleChunksForSpawning.valuesInIterationOrder();

    for (const EnumCreatureType *creatureTypePtr : creatureTypes)
    {
        const EnumCreatureType &creatureType = *creatureTypePtr;
        if ((!creatureType.getPeacefulCreature() || spawnPeaceful) &&
            (creatureType.getPeacefulCreature() || spawnHostiles) &&
            world->countEntities(creatureType.getCreatureTag()) <=
                creatureType.getMaxNumberOfCreature() * static_cast<int_t>(eligibleChunksForSpawning.size()) / 256)
        {
            for (const ChunkCoordIntPair &chunkPos : eligibleChunks)
            {
                if (!interiorChunksForSpawning.contains(chunkPos))
                    continue;

                const ChunkPosition spawnPos = getRandomSpawningPointInChunk(world, chunkPos.chunkXPos, chunkPos.chunkZPos);
                const int_t baseX = spawnPos.x;
                const int_t baseY = spawnPos.y;
                const int_t baseZ = spawnPos.z;

#if PLATFORM_BOUNDED_WORLD
                if (!spawnColumnResident(world, baseX, baseZ))
                    continue;
#endif
                if (world->isBlockNormalCube(baseX, baseY, baseZ) ||
                    world->getBlockMaterial(baseX, baseY, baseZ) != creatureType.getCreatureMaterial())
                    continue;

                int_t spawnedInGroup = 0;
                for (int_t attempt = 0; attempt < 3; ++attempt)
                {
                    int_t x = baseX;
                    int_t y = baseY;
                    int_t z = baseZ;
                    const int_t spread = 6;
                    SpawnListEntry *spawnEntry = nullptr;

                    for (int_t subAttempt = 0; subAttempt < 4; ++subAttempt)
                    {
                        x = world->rand.nextIntOffset(x, spread);
                        y = world->rand.nextIntOffset(y, 1);
                        z = world->rand.nextIntOffset(z, spread);

#if PLATFORM_BOUNDED_WORLD
                        if (!spawnColumnResident(world, x, z))
                            continue;
#endif
                        if (!canCreatureTypeSpawnAtLocation(creatureType, world, x, y, z))
                            continue;

                        const float spawnX = static_cast<float>(x) + 0.5f;
                        const float spawnY = static_cast<float>(y);
                        const float spawnZ = static_cast<float>(z) + 0.5f;
                        if (world->getClosestPlayer(spawnX, spawnY, spawnZ, 24.0) != nullptr)
                            continue;

                        const float dx = spawnX - static_cast<float>(spawnPoint.x);
                        const float dy = spawnY - static_cast<float>(spawnPoint.y);
                        const float dz = spawnZ - static_cast<float>(spawnPoint.z);
                        if (dx * dx + dy * dy + dz * dz < 576.0f)
                            continue;

                        if (spawnEntry == nullptr)
                        {
                            spawnEntry = world->getRandomMob(creatureType, x, y, z);
                            if (spawnEntry == nullptr)
                                break;
                        }

                        const auto factory = getEntityFactory().find(spawnEntry->entityClass);
                        if (factory == getEntityFactory().end())
                            return totalSpawned;

                        EntityLiving *entity = factory->second(world);
                        if (entity == nullptr)
                            return totalSpawned;

                        entity->setLocationAndAngles(spawnX, spawnY, spawnZ,
                                                     world->rand.nextFloat() * 360.0f, 0.0f);
                        if (entity->getCanSpawnHere())
                        {
                            if (world->spawnEntityInWorld(entity))
                            {
                                ++spawnedInGroup;
                                creatureSpecificInit(entity, world, spawnX, spawnY, spawnZ);
#if PLATFORM_CONSOLE_LOW || PLATFORM_WII
                                ++liveMobs;
#endif
                                if (spawnedInGroup >= entity->getMaxSpawnedInChunk())
                                    goto next_chunk;
                            }
                            else
                            {
                                delete entity;
                            }
                        }
                        else
                        {
                            delete entity;
                        }

                        totalSpawned += spawnedInGroup;
#if PLATFORM_CONSOLE_LOW || PLATFORM_WII
                        if (liveMobs >= PLATFORM_MAX_LIVE_MOBS)
                            return totalSpawned;
#endif
                    }
                }

            next_chunk:
                ;
            }
        }
    }

    return totalSpawned;
}


void SpawnerAnimals::performWorldGenSpawning(World *world, BiomeGenBase *biome,
                                              int_t x, int_t z, int_t width, int_t depth,
                                              Random &random)
{
    if (world == nullptr || biome == nullptr || width <= 0 || depth <= 0)
        return;

    std::vector<SpawnListEntry> *spawnList = biome->getSpawnableList(EnumCreatureType::creature);
    if (spawnList == nullptr || spawnList->empty())
        return;

    while (random.nextFloat() < biome->getSpawningChance())
    {
        int_t totalWeight = 0;
        for (const SpawnListEntry &entry : *spawnList)
            totalWeight += entry.spawnRarityRate;
        if (totalWeight <= 0)
            return;

        int_t choice = world->rand.nextInt(totalWeight);
        const SpawnListEntry *selected = &spawnList->front();
        for (const SpawnListEntry &entry : *spawnList)
        {
            choice -= entry.spawnRarityRate;
            if (choice < 0)
            {
                selected = &entry;
                break;
            }
        }

        const int_t groupCount = selected->minGroupCount +
            random.nextInt(1 + selected->maxGroupCount - selected->minGroupCount);
        int_t spawnX = x + random.nextInt(width);
        int_t spawnZ = z + random.nextInt(depth);
        const int_t originX = spawnX;
        const int_t originZ = spawnZ;

        for (int_t member = 0; member < groupCount; ++member)
        {
            bool spawned = false;
            for (int_t attempt = 0; !spawned && attempt < 4; ++attempt)
            {
                const int_t spawnY = world->getTopSolidOrLiquidBlock(spawnX, spawnZ);
                if (canCreatureTypeSpawnAtLocation(EnumCreatureType::creature, world, spawnX, spawnY, spawnZ))
                {
                    const float fx = static_cast<float>(spawnX) + 0.5f;
                    const float fy = static_cast<float>(spawnY);
                    const float fz = static_cast<float>(spawnZ) + 0.5f;
                    auto factory = getEntityFactory().find(selected->entityClass);
                    if (factory != getEntityFactory().end())
                    {
                        EntityLiving *entity = factory->second(world);
                        if (entity != nullptr)
                        {
                            entity->setLocationAndAngles(static_cast<double>(fx), static_cast<double>(fy),
                                                         static_cast<double>(fz), random.nextFloat() * 360.0f, 0.0f);
                            if (world->spawnEntityInWorld(entity))
                            {
                                creatureSpecificInit(entity, world, fx, fy, fz);
                                spawned = true;
                            }
                            else
                            {
                                delete entity;
                            }
                        }
                    }
                }

                spawnX = JavaArithmetic::intAdd(spawnX, random.nextIntDifference(5));
                spawnZ = JavaArithmetic::intAdd(spawnZ, random.nextIntDifference(5));
                while (spawnX < x || spawnX >= x + width || spawnZ < z || spawnZ >= z + depth)
                {
                    spawnX = JavaArithmetic::intAdd(originX, random.nextIntDifference(5));
                    spawnZ = JavaArithmetic::intAdd(originZ, random.nextIntDifference(5));
                }
            }
        }
    }
}

bool SpawnerAnimals::canCreatureTypeSpawnAtLocation(const EnumCreatureType &type, World *world, int_t i, int_t j, int_t k)
{
    if (type.getCreatureMaterial() == Material::water)
        return world->getBlockMaterial(i, j, k)->getIsLiquid() && !world->isBlockNormalCube(i, j + 1, k);

    const int_t blockBelow = world->getBlockId(i, j - 1, k);
    return Block::isNormalCube(blockBelow)
        && blockBelow != Block::bedrock->blockID
        && !world->isBlockNormalCube(i, j, k)
        && !world->getBlockMaterial(i, j, k)->getIsLiquid()
        && !world->isBlockNormalCube(i, j + 1, k);
}

void SpawnerAnimals::creatureSpecificInit(EntityLiving *entity, World *world, float x, float y, float z)
{
    if (dynamic_cast<EntitySpider *>(entity) != nullptr && world->rand.nextInt(100) == 0)
    {
        EntitySkeleton *skeleton = new EntitySkeleton(world);
        skeleton->setLocationAndAngles((double)x, (double)y, (double)z, entity->rotationYaw, 0.0f);
        if (world->entityJoinedWorld(skeleton))
            skeleton->mountEntity(entity);
        else
            delete skeleton;
    }
    else if (EntitySheep *sheep = dynamic_cast<EntitySheep *>(entity))
    {
        sheep->setFleeceColor(EntitySheep::getRandomFleeceColor(world->rand));
    }
    else if (dynamic_cast<EntityOcelot *>(entity) != nullptr && world->rand.nextInt(7) == 0)
    {
        for (int_t kitten = 0; kitten < 2; ++kitten)
        {
            EntityOcelot *ocelot = new EntityOcelot(world);
            ocelot->setLocationAndAngles(static_cast<double>(x), static_cast<double>(y), static_cast<double>(z),
                                         entity->rotationYaw, 0.0f);
            ocelot->setGrowingAge(-24000);
            if (!world->spawnEntityInWorld(ocelot))
                delete ocelot;
        }
    }
}

bool SpawnerAnimals::performSleepSpawning(World *world, std::vector<EntityPlayer*> &playerList)
{
    bool spawned = false;
    Pathfinder pathfinder(world);

    static const std::type_index nightSpawnEntities[] = {
        typeid(EntitySpider),
        typeid(EntityZombie),
        typeid(EntitySkeleton)
    };
    static const int_t nightSpawnCount = 3;

    for (EntityPlayer *player : playerList)
    {
        bool foundSpawn = false;
        int_t tries = 0;

        while (tries < 20 && !foundSpawn)
        {
            int_t bx = world->rand.nextIntOffset(MathHelper::floor_double(player->posX), 32);
            int_t bz = world->rand.nextIntOffset(MathHelper::floor_double(player->posZ), 32);
            int_t by = world->rand.nextIntOffset(MathHelper::floor_double(player->posY), 16);

            if (by < 1)   by = 1;
            if (by > WorldHeight::MAX_Y) by = WorldHeight::MAX_Y;

            int_t entityIdx = world->rand.nextInt(nightSpawnCount);
            int_t sy = by;

            while (sy > 2 && !world->isBlockNormalCube(bx, sy - 1, bz))
                sy--;
            while (!canCreatureTypeSpawnAtLocation(EnumCreatureType::monster, world, bx, sy, bz) && sy < by + 16 && sy < WorldHeight::HEIGHT)
                sy++;

            if (sy >= by + 16 || sy >= WorldHeight::HEIGHT)
            {
                tries++;
                continue;
            }

            float fx = (float)bx + 0.5f;
            float fy = (float)sy;
            float fz = (float)bz + 0.5f;

            auto factIt = getEntityFactory().find(nightSpawnEntities[entityIdx]);
            if (factIt == getEntityFactory().end()) { tries++; continue; }

            EntityLiving *entity = factIt->second(world);
            if (entity == nullptr) { tries++; continue; }

            entity->setLocationAndAngles((double)fx, (double)fy, (double)fz, world->rand.nextFloat() * 360.0f, 0.0f);

            bool entityJoined = false;
            if (entity->getCanSpawnHere())
            {
                PathEntity *path = pathfinder.createEntityPathTo(entity, player, 32.0f);
                if (path != nullptr && path->getCurrentPathLength() > 1)
                {
                    PathPoint *tip = path->getFinalPathPoint();
                    if (std::abs((double)tip->xCoord - player->posX) < 1.5 &&
                        std::abs((double)tip->zCoord - player->posZ) < 1.5 &&
                        std::abs((double)tip->yCoord - player->posY) < 1.5)
                    {
                        ChunkCoordinates *bedPos = BlockBed::getNearestEmptyChunkCoordinates(
                            world,
                            MathHelper::floor_double(player->posX),
                            MathHelper::floor_double(player->posY),
                            MathHelper::floor_double(player->posZ), 1);

                        if (bedPos == nullptr)
                            bedPos = new ChunkCoordinates(bx, sy + 1, bz);

                        entity->setLocationAndAngles(
                            (float)bedPos->x + 0.5f, (float)bedPos->y, (float)bedPos->z + 0.5f, 0.0f, 0.0f);

                        entityJoined = world->entityJoinedWorld(entity);
                        if (entityJoined)
                        {
                            creatureSpecificInit(entity, world,
                                (float)bedPos->x + 0.5f, (float)bedPos->y, (float)bedPos->z + 0.5f);

                            player->wakeUpPlayer(true, false, false);
                            entity->playLivingSound();

                            spawned = true;
                            foundSpawn = true;
                        }

                        delete bedPos;
                    }
                }
                delete path;
            }

            if (!entityJoined)
            {
                delete entity;
            }

            tries++;
        }
    }

    return spawned;
}
