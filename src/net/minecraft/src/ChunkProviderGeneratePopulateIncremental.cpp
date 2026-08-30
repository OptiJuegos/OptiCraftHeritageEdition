#include "ChunkProviderGenerate.h"

#include <algorithm>

#if PLATFORM_INCREMENTAL_POPULATE

#include "BiomeDecorator.h"
#include "BiomeGenBase.h"
#include "Block.h"
#include "BlockSand.h"
#include "MapGenMineshaft.h"
#include "MapGenStronghold.h"
#include "MapGenVillage.h"
#include "SpawnerAnimals.h"
#include "World.h"
#include "WorldChunkManager.h"
#include "WorldGenDungeons.h"
#include "WorldGenLakes.h"
#include "java/Arithmetic.h"
#include "java/System.h"
#include "platform/Profiler.h"
#include "platform/PlatformTuning.h"

namespace
{
    class InstantSandGuard
    {
    public:
        InstantSandGuard() { BlockSand::fallInstantly = true; }
        ~InstantSandGuard() { BlockSand::fallInstantly = false; }
    };
}

void ChunkProviderGenerate::beginPopulateTask(int_t i, int_t j)
{
    const long long startNs = System::nanoTime();
    PopulateTask task;
    task.active = true;
    task.chunkX = i;
    task.chunkZ = j;
    task.blockX = JavaArithmetic::intMul(i, 16);
    task.blockZ = JavaArithmetic::intMul(j, 16);
    task.biome = worldObj->getWorldChunkManager()->getBiomeGenAt(JavaArithmetic::intAdd(task.blockX, 16), JavaArithmetic::intAdd(task.blockZ, 16));

    task.random.setSeed(worldObj->getRandomSeed());
    const long_t xMultiplier = (task.random.nextLong() / 2LL) * 2LL + 1LL;
    const long_t zMultiplier = (task.random.nextLong() / 2LL) * 2LL + 1LL;
    const ulong_t seedBits = static_cast<ulong_t>(static_cast<long_t>(i)) * static_cast<ulong_t>(xMultiplier)
                           + static_cast<ulong_t>(static_cast<long_t>(j)) * static_cast<ulong_t>(zMultiplier)
                           ^ static_cast<ulong_t>(worldObj->getRandomSeed());
    task.random.setSeed(JavaArithmetic::longFromBits(seedBits));
    task.totalNs = System::nanoTime() - startNs;
    populateTask = task;
}

void ChunkProviderGenerate::finishPopulateTask()
{
    if (populateTask.decorationStarted && populateTask.biome != nullptr &&
        populateTask.biome->biomeDecorator != nullptr)
    {
        populateTask.biome->biomeDecorator->finishDecoration();
    }
#if defined(PS2_RENDER_STATS) || (defined(WII_PLATFORM) && MC_LOG_LEVEL >= 2)
    platformProfilePopulatePhase(PlatformPopulatePhase::Total, populateTask.totalNs);
#endif
    populateTask.active = false;
    populateTask.decorationStarted = false;
    populateTask.stage = PopulateStage::Done;
    populateTask.index = 0;
}

bool ChunkProviderGenerate::advancePopulateTask()
{
    if (!populateTask.active)
        return true;

    InstantSandGuard sandGuard;
    PopulateTask &task = populateTask;
    Random &random = task.random;
    const int_t blockX = task.blockX;
    const int_t blockZ = task.blockZ;

    for (;;)
    {
        const long long startNs = System::nanoTime();
        PlatformPopulatePhase profilePhase = PlatformPopulatePhase::Decoration;
        bool didWork = true;

        switch (task.stage)
        {
        case PopulateStage::Structures:
            if (mapFeaturesEnabled)
            {
                mineshaftGenerator->generateStructuresInChunk(worldObj, random, task.chunkX, task.chunkZ);
                task.villageGenerated = villageGenerator->generateStructuresInChunk(
                    worldObj, random, task.chunkX, task.chunkZ);
                strongholdGenerator->generateStructuresInChunk(worldObj, random, task.chunkX, task.chunkZ);
            }
            task.stage = PopulateStage::WaterLake;
            break;

        case PopulateStage::WaterLake:
            profilePhase = PlatformPopulatePhase::Lakes;
            if (!task.villageGenerated && random.nextInt(4) == 0)
            {
                const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockX, random.nextInt(16)), 8);
                const int_t y = random.nextInt(128);
                const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockZ, random.nextInt(16)), 8);
                WorldGenLakes(Block::waterStill->blockID).generate(worldObj, random, x, y, z);
            }
            task.stage = PopulateStage::LavaLake;
            break;

        case PopulateStage::LavaLake:
            profilePhase = PlatformPopulatePhase::Lakes;
            if (!task.villageGenerated && random.nextInt(8) == 0)
            {
                const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockX, random.nextInt(16)), 8);
                const int_t y = random.nextInt(random.nextInt(120) + 8);
                const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockZ, random.nextInt(16)), 8);
                if (y < 63 || random.nextInt(10) == 0)
                    WorldGenLakes(Block::lavaStill->blockID).generate(worldObj, random, x, y, z);
            }
            task.stage = PopulateStage::Dungeons;
            task.index = 0;
            break;

        case PopulateStage::Dungeons:
            profilePhase = PlatformPopulatePhase::Dungeons;
            if (task.index < PLATFORM_POPULATE_DUNGEONS)
            {
                const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockX, random.nextInt(16)), 8);
                const int_t y = random.nextInt(128);
                const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockZ, random.nextInt(16)), 8);
                WorldGenDungeons().generate(worldObj, random, x, y, z);
                ++task.index;
            }
            else
            {
                task.stage = PopulateStage::BiomeDecoration;
                task.index = 0;
                didWork = false;
            }
            break;

        case PopulateStage::BiomeDecoration:
            if (task.biome == nullptr || task.biome->biomeDecorator == nullptr)
            {
                task.stage = PopulateStage::Spawning;
                didWork = false;
                break;
            }
            if (!task.decorationStarted)
            {
                task.biome->biomeDecorator->beginDecoration(worldObj, random, blockX, blockZ);
                task.decorationStarted = true;
            }
            if (task.biome->biomeDecorator->advanceDecoration())
            {
                task.biome->biomeDecorator->finishDecoration();
                task.decorationStarted = false;
                task.stage = PopulateStage::Spawning;
                didWork = false;
            }
            break;

        case PopulateStage::Spawning:
            if (task.biome != nullptr)
            {
                SpawnerAnimals::performWorldGenSpawning(
                    worldObj, task.biome, JavaArithmetic::intAdd(blockX, 8), JavaArithmetic::intAdd(blockZ, 8), 16, 16, random);
            }
            task.stage = PopulateStage::Snow;
            break;

        case PopulateStage::Snow:
            profilePhase = PlatformPopulatePhase::Snow;
            if (!PLATFORM_POPULATE_SNOW_PASS)
            {
                task.stage = PopulateStage::Done;
                break;
            }
            {
                const int_t endIndex = std::min<int_t>(256, task.index + PLATFORM_POPULATE_SNOW_COLUMNS_PER_STEP);
                for (; task.index < endIndex; ++task.index)
                {
                    const int_t localX = task.index >> 4;
                    const int_t localZ = task.index & 15;
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockX, 8), localX);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockZ, 8), localZ);
                    const int_t y = worldObj->getPrecipitationHeight(x, z);
                    bool icePlaced = false;
                    bool snowPlaced = false;
                    int notifyCalls = 0;
                    if (worldObj->isBlockHydratedDirectly(x, y - 1, z))
                    {
                        ++notifyCalls;
                        icePlaced = worldObj->setBlockWithNotify(x, y - 1, z, Block::ice->blockID);
                    }
                    if (worldObj->canSnowAt(x, y, z))
                    {
                        ++notifyCalls;
                        snowPlaced = worldObj->setBlockWithNotify(x, y, z, Block::snow->blockID);
                    }
#if defined(PS2_RENDER_STATS)
                    platformProfileSnowColumn(icePlaced, snowPlaced, notifyCalls);
#endif
                }
                if (task.index >= 256)
                {
                    task.stage = PopulateStage::Done;
                    task.index = 0;
                }
            }
            break;

        case PopulateStage::Done:
            finishPopulateTask();
            return true;
        }

        if (!didWork)
            continue;

        const long long elapsedNs = System::nanoTime() - startNs;
        task.totalNs += elapsedNs;
#if defined(PS2_RENDER_STATS) || (defined(WII_PLATFORM) && MC_LOG_LEVEL >= 2)
        platformProfilePopulatePhase(profilePhase, elapsedNs);
#endif
        if (task.stage == PopulateStage::Done)
        {
            finishPopulateTask();
            return true;
        }
        return false;
    }
}

bool ChunkProviderGenerate::populateStep(IChunkProvider *ichunkprovider, int_t i, int_t j)
{
    (void)ichunkprovider;
    if (!populateTask.active)
        beginPopulateTask(i, j);
    else if (populateTask.chunkX != i || populateTask.chunkZ != j)
        return false;

    return advancePopulateTask();
}

#endif // PLATFORM_INCREMENTAL_POPULATE
