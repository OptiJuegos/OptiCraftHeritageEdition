#include "BiomeDecorator.h"

#include <stdexcept>

#include "BiomeGenBase.h"
#include "Block.h"
#include "BlockDeadBush.h"
#include "BlockFlower.h"
#include "World.h"
#include "WorldGenBigMushroom.h"
#include "WorldGenCactus.h"
#include "WorldGenClay.h"
#include "WorldGenDeadBush.h"
#include "WorldGenFlowers.h"
#include "WorldGenLiquids.h"
#include "WorldGenMinable.h"
#include "WorldGenPumpkin.h"
#include "WorldGenReed.h"
#include "WorldGenSand.h"
#include "WorldGenWaterlily.h"
#include "WorldGenerator.h"
#include "java/Arithmetic.h"
#include "java/Random.h"
#include "platform/PlatformTuning.h"
#include "platform/WorkProfiler.h"
#include "platform/ExtendedProfiler.h"

namespace
{
    class ChunkLocalStructureAvoidanceScope
    {
    public:
        explicit ChunkLocalStructureAvoidanceScope(World *worldValue)
            : world(worldValue), active(worldValue != nullptr && worldValue->isChunkLocalDecorationActive())
        {
            if (active)
                world->setChunkLocalDecorationStructureAvoidance(true);
        }

        ~ChunkLocalStructureAvoidanceScope()
        {
            if (active)
                world->setChunkLocalDecorationStructureAvoidance(false);
        }

        ChunkLocalStructureAvoidanceScope(const ChunkLocalStructureAvoidanceScope &) = delete;
        ChunkLocalStructureAvoidanceScope &operator=(const ChunkLocalStructureAvoidanceScope &) = delete;

    private:
        World *world;
        bool active;
    };
}

// Profiler label for a stage. Indexed by DecorationStage, so the enum in the
// header stays the only place the stages are enumerated; the static_assert is
// what fails the build if a stage is added without a name. The work profiler
// takes these pointers rather than defining an enum of its own for the same
// reason -- it aggregates by pointer identity, and these are string literals.
const char *BiomeDecorator::stageName(DecorationStage stage)
{
    static const char *const names[] = {
        "Dirt", "Gravel", "Coal", "Iron", "Gold", "Redstone", "Diamond", "Lapis",
        "Sand", "Clay", "GravelAsSand", "TreeSetup", "Trees", "BigMushrooms",
        "YellowFlowers", "RedFlowers", "Grass", "DeadBushes", "WaterLilies",
        "MushroomLoopBrown", "MushroomLoopRed", "BrownMushroom", "RedMushroom",
        "Reeds", "ExtraReeds", "Pumpkin", "Cacti", "WaterSprings", "LavaSprings",
        "Done"
    };
    static_assert(sizeof(names) / sizeof(names[0]) ==
                  static_cast<int>(DecorationStage::Done) + 1,
                  "Decoration stage names must match DecorationStage");

    const int index = static_cast<int>(stage);
    if (index < 0 || index >= (int)(sizeof(names) / sizeof(names[0])))
        return "Unknown";
    return names[index];
}

BiomeDecorator::BiomeDecorator(BiomeGenBase *biomeValue)
    : generateLakes(true), currentWorld(nullptr), randomGenerator(nullptr),
      chunk_X(0), chunk_Z(0), biome(biomeValue), decorationStage(DecorationStage::Done),
      stopStage(DecorationStage::Done),
      decorationIndex(0), treeCount(0), clayGen(new WorldGenClay(4)),
      sandGen(new WorldGenSand(7, Block::sand->blockID)),
      gravelAsSandGen(new WorldGenSand(6, Block::gravel->blockID)),
      dirtGen(new WorldGenMinable(Block::dirt->blockID, 32)),
      gravelGen(new WorldGenMinable(Block::gravel->blockID, 32)),
      coalGen(new WorldGenMinable(Block::oreCoal->blockID, 16)),
      ironGen(new WorldGenMinable(Block::oreIron->blockID, 8)),
      goldGen(new WorldGenMinable(Block::oreGold->blockID, 8)),
      redstoneGen(new WorldGenMinable(Block::oreRedstone->blockID, 7)),
      diamondGen(new WorldGenMinable(Block::oreDiamond->blockID, 7)),
      lapisGen(new WorldGenMinable(Block::oreLapis->blockID, 6)),
      plantYellowGen(new WorldGenFlowers(Block::plantYellow->blockID)),
      plantRedGen(new WorldGenFlowers(Block::plantRed->blockID)),
      mushroomBrownGen(new WorldGenFlowers(Block::mushroomBrown->blockID)),
      mushroomRedGen(new WorldGenFlowers(Block::mushroomRed->blockID)),
      bigMushroomGen(new WorldGenBigMushroom()), reedGen(new WorldGenReed()),
      cactusGen(new WorldGenCactus()), waterlilyGen(new WorldGenWaterlily()),
      waterlilyPerChunk(0), treesPerChunk(0), flowersPerChunk(2), grassPerChunk(1),
      deadBushPerChunk(0), mushroomsPerChunk(0), reedsPerChunk(0), cactiPerChunk(0),
      sandPerChunk(1), sandPerChunk2(3), clayPerChunk(1), bigMushroomsPerChunk(0)
{
}

BiomeDecorator::~BiomeDecorator() = default;

void BiomeDecorator::decorate(World *world, Random &random, int_t chunkX, int_t chunkZ)
{
    beginDecoration(world, random, chunkX, chunkZ);
    try
    {
        decorate();
    }
    catch (...)
    {
        finishDecoration();
        throw;
    }
    finishDecoration();
}

void BiomeDecorator::decorate()
{
    while (!advanceDecoration())
    {
    }
}

void BiomeDecorator::beginDecoration(World *world, Random &random, int_t chunkX, int_t chunkZ,
                                     DecorationPass pass)
{
    if (currentWorld != nullptr)
        throw std::runtime_error("Already decorating!!");
    currentWorld = world;
    randomGenerator = &random;
    chunk_X = chunkX;
    chunk_Z = chunkZ;
    switch (pass)
    {
    case DecorationPass::Vegetation:
        decorationStage = DecorationStage::Dirt;
        stopStage = DecorationStage::WaterSprings;
        break;
    case DecorationPass::Springs:
        decorationStage = DecorationStage::WaterSprings;
        stopStage = DecorationStage::Done;
        break;
    case DecorationPass::Full:
    default:
        decorationStage = DecorationStage::Dirt;
        stopStage = DecorationStage::Done;
        break;
    }
    decorationIndex = 0;
    treeCount = 0;
}

void BiomeDecorator::finishDecoration()
{
    currentWorld = nullptr;
    randomGenerator = nullptr;
    decorationStage = DecorationStage::Done;
    decorationIndex = 0;
    treeCount = 0;
}

void BiomeDecorator::nextStage(DecorationStage stage)
{
    decorationStage = stage == stopStage ? DecorationStage::Done : stage;
    decorationIndex = 0;
}

bool BiomeDecorator::advanceStandardOre1(int_t count, WorldGenerator *generator, int_t minY, int_t maxY)
{
    if (decorationIndex >= count) return false;
    const int_t x = JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16));
    const int_t y = randomGenerator->nextInt(maxY - minY) + minY;
    const int_t z = JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16));
    generator->generate(currentWorld, *randomGenerator, x, y, z);
    ++decorationIndex;
    return true;
}

bool BiomeDecorator::advanceStandardOre2(int_t count, WorldGenerator *generator, int_t centerY, int_t spread)
{
    if (decorationIndex >= count) return false;
    const int_t x = JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16));
    const int_t y = randomGenerator->nextInt(spread) + randomGenerator->nextInt(spread) + centerY - spread;
    const int_t z = JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16));
    generator->generate(currentWorld, *randomGenerator, x, y, z);
    ++decorationIndex;
    return true;
}

void BiomeDecorator::genStandardOre1(int_t count, WorldGenerator *generator, int_t minY, int_t maxY)
{
    for (int_t i = 0; i < count; ++i)
    {
        const int_t x = JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16));
        const int_t y = randomGenerator->nextInt(maxY - minY) + minY;
        const int_t z = JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16));
        generator->generate(currentWorld, *randomGenerator, x, y, z);
    }
}

void BiomeDecorator::genStandardOre2(int_t count, WorldGenerator *generator, int_t centerY, int_t spread)
{
    for (int_t i = 0; i < count; ++i)
    {
        const int_t x = JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16));
        const int_t y = randomGenerator->nextInt(spread) + randomGenerator->nextInt(spread) + centerY - spread;
        const int_t z = JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16));
        generator->generate(currentWorld, *randomGenerator, x, y, z);
    }
}

void BiomeDecorator::generateOres()
{
    genStandardOre1(PLATFORM_POPULATE_DIRT_VEINS, dirtGen.get(), 0, 128);
    genStandardOre1(PLATFORM_POPULATE_GRAVEL_VEINS, gravelGen.get(), 0, 128);
    genStandardOre1(20, coalGen.get(), 0, 128);
    genStandardOre1(20, ironGen.get(), 0, 64);
    genStandardOre1(2, goldGen.get(), 0, 32);
    genStandardOre1(8, redstoneGen.get(), 0, 16);
    genStandardOre1(1, diamondGen.get(), 0, 16);
    genStandardOre2(1, lapisGen.get(), 16, 16);
}

bool BiomeDecorator::advanceDecoration()
{
    if (currentWorld == nullptr || randomGenerator == nullptr) return true;

    // Attributed to the stage the call ENDS on, which is the stage that spent
    // the time: the switch below only reaches nextStage() for a stage that
    // produced no work and yielded nothing, so a stage that did work is still
    // the current one when the call unwinds. One measurement point therefore
    // covers all thirty stages without touching a single case.
    //
    // Reading the member in the destructor also keeps the attribution correct
    // on the catch(...) path, which rethrows after finishDecoration().
    struct StageScope
    {
        const DecorationStage *stage;
        std::uint32_t start;
        explicit StageScope(const DecorationStage *s)
            : stage(s), start(platformProfileRenderPhaseBegin()) {}
        ~StageScope() { platformProfileDecorWork(start, stageName(*stage)); }
        StageScope(const StageScope &) = delete;
        StageScope &operator=(const StageScope &) = delete;
    } stageScope(&decorationStage);

    try
    {
        for (;;)
        {
            switch (decorationStage)
            {
            case DecorationStage::Dirt:
                if (advanceStandardOre1(PLATFORM_POPULATE_DIRT_VEINS, dirtGen.get(), 0, 128)) return false;
                nextStage(DecorationStage::Gravel); break;
            case DecorationStage::Gravel:
                if (advanceStandardOre1(PLATFORM_POPULATE_GRAVEL_VEINS, gravelGen.get(), 0, 128)) return false;
                nextStage(DecorationStage::Coal); break;
            case DecorationStage::Coal:
                if (advanceStandardOre1(20, coalGen.get(), 0, 128)) return false;
                nextStage(DecorationStage::Iron); break;
            case DecorationStage::Iron:
                if (advanceStandardOre1(20, ironGen.get(), 0, 64)) return false;
                nextStage(DecorationStage::Gold); break;
            case DecorationStage::Gold:
                if (advanceStandardOre1(2, goldGen.get(), 0, 32)) return false;
                nextStage(DecorationStage::Redstone); break;
            case DecorationStage::Redstone:
                if (advanceStandardOre1(8, redstoneGen.get(), 0, 16)) return false;
                nextStage(DecorationStage::Diamond); break;
            case DecorationStage::Diamond:
                if (advanceStandardOre1(1, diamondGen.get(), 0, 16)) return false;
                nextStage(DecorationStage::Lapis); break;
            case DecorationStage::Lapis:
                if (advanceStandardOre2(1, lapisGen.get(), 16, 16)) return false;
                nextStage(DecorationStage::Sand); break;
            case DecorationStage::Sand:
                if (decorationIndex < sandPerChunk2)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    sandGen->generate(currentWorld, *randomGenerator, x, currentWorld->getTopSolidOrLiquidBlock(x, z), z);
                    ++decorationIndex; return false;
                }
                nextStage(DecorationStage::Clay); break;
            case DecorationStage::Clay:
                if (decorationIndex < clayPerChunk)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    clayGen->generate(currentWorld, *randomGenerator, x, currentWorld->getTopSolidOrLiquidBlock(x, z), z);
                    ++decorationIndex; return false;
                }
                nextStage(DecorationStage::GravelAsSand); break;
            case DecorationStage::GravelAsSand:
                if (decorationIndex < sandPerChunk)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    sandGen->generate(currentWorld, *randomGenerator, x, currentWorld->getTopSolidOrLiquidBlock(x, z), z);
                    ++decorationIndex; return false;
                }
                nextStage(DecorationStage::TreeSetup); break;
            case DecorationStage::TreeSetup:
                if (currentWorld != nullptr && currentWorld->isLimitedWorld())
                {
                    // MCPE 0.6.0 / Pocket Edition classic tree distribution:
                    // In MCPE 0.6.0, every terrestrial biome has natural tree coverage.
                    // Balanced for PS2 performance while capturing the authentic MCPE landscape.
                    if (biome == BiomeGenBase::forest || biome == BiomeGenBase::forestHills)
                    {
                        // Dense forest (6 to 8 trees per chunk)
                        treeCount = 6 + randomGenerator->nextInt(3);
                    }
                    else if (biome == BiomeGenBase::taiga || biome == BiomeGenBase::taigaHills)
                    {
                        // Taiga pine forest (6 to 8 trees per chunk)
                        treeCount = 6 + randomGenerator->nextInt(3);
                    }
                    else if (biome == BiomeGenBase::jungle || biome == BiomeGenBase::jungleHills)
                    {
                        // Jungle grove (7 to 9 trees per chunk)
                        treeCount = 7 + randomGenerator->nextInt(3);
                    }
                    else if (biome == BiomeGenBase::swampland)
                    {
                        // Swamp with vines (2 to 3 trees per chunk)
                        treeCount = 2 + (randomGenerator->nextInt(2) == 0 ? 1 : 0);
                    }
                    else if (biome == BiomeGenBase::extremeHills || biome == BiomeGenBase::extremeHillsEdge)
                    {
                        // MCPE 0.6.0 classic alpine slopes: 1 to 2 trees per chunk
                        treeCount = 1 + (randomGenerator->nextInt(2) == 0 ? 1 : 0);
                    }
                    else if (biome == BiomeGenBase::plains)
                    {
                        // MCPE 0.6.0 / Beta scattered trees across plains (50% 1 tree, 20% 2 trees, 30% 0 trees)
                        const int_t roll = randomGenerator->nextInt(10);
                        if (roll < 5)
                            treeCount = 1;
                        else if (roll < 7)
                            treeCount = 2;
                        else
                            treeCount = 0;
                    }
                    else if (biome == BiomeGenBase::icePlains || biome == BiomeGenBase::iceMountains)
                    {
                        // Cold snow plains: 30% chance of 1 tree
                        treeCount = (randomGenerator->nextInt(10) < 3) ? 1 : 0;
                    }
                    else if (treesPerChunk > 0)
                    {
                        treeCount = std::min<int_t>(treesPerChunk, 6);
                    }
                    else
                    {
                        treeCount = 0;
                    }
                    // The MCPE table above is a desktop-side aesthetic: bounded
                    // consoles bought their populate budget with
                    // PLATFORM_POPULATE_TREES_PER_CHUNK_MAX, and this branch
                    // must honour it exactly like the vanilla one below --
                    // the MCPE profile arrived from main without the cap and
                    // 6-9 trees per chunk tripled the 3DS/PS2 populate cost
                    // in Old worlds (2026-09-29). Desktop (-1) is unchanged.
                    if (PLATFORM_POPULATE_TREES_PER_CHUNK_MAX >= 0 && treeCount > PLATFORM_POPULATE_TREES_PER_CHUNK_MAX)
                        treeCount = PLATFORM_POPULATE_TREES_PER_CHUNK_MAX;
                }
                else
                {
                    treeCount = treesPerChunk;
                    if (PLATFORM_POPULATE_TREES_PER_CHUNK_MAX >= 0 && treeCount > PLATFORM_POPULATE_TREES_PER_CHUNK_MAX)
                        treeCount = PLATFORM_POPULATE_TREES_PER_CHUNK_MAX;
                    if (randomGenerator->nextInt(10) == 0) ++treeCount;
                }

                // Enforce platform tree limit unconditionally across all world types (Infinite, 256x256, 864x864)
                if (PLATFORM_POPULATE_TREES_PER_CHUNK_MAX >= 0 && treeCount > PLATFORM_POPULATE_TREES_PER_CHUNK_MAX)
                    treeCount = PLATFORM_POPULATE_TREES_PER_CHUNK_MAX;

                nextStage(DecorationStage::Trees); break;
            case DecorationStage::Trees:
                if (decorationIndex < treeCount)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    BiomeGenBase *treeBiome = biome;
                    if (currentWorld != nullptr && currentWorld->isLimitedWorld() &&
                        (biome == BiomeGenBase::plains || biome == BiomeGenBase::extremeHills || biome == BiomeGenBase::extremeHillsEdge))
                    {
                        // In MCPE 0.6.0, plains and hills have a mix of oak (75%) and birch (25%)
                        if (randomGenerator->nextInt(4) == 0 && BiomeGenBase::forest != nullptr)
                            treeBiome = BiomeGenBase::forest;
                    }

                    WorldGenerator *generator = treeBiome != nullptr ? treeBiome->getRandomWorldGenForTrees(*randomGenerator) : nullptr;
                    if (generator != nullptr)
                    {
                        generator->setScale(1.0, 1.0, 1.0);
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
                        const PlatformPopulationAccessSnapshot accessStart = platformProfilePopulationAccessSnapshot();
                        const std::uint32_t treeStart = platformProfileRenderPhaseBegin();
#endif
                        const int_t y = currentWorld->getHeightValue(x, z);
                        ChunkLocalStructureAvoidanceScope structureAvoidance(currentWorld);
                        generator->generate(currentWorld, *randomGenerator, x, y, z);
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
                        platformProfileTreeGenerator(treeStart, accessStart, generator);
#endif
                    }
                    if (treeBiome != nullptr)
                        treeBiome->releaseWorldGenForTrees(generator);
                    ++decorationIndex; return false;
                }
                nextStage(DecorationStage::BigMushrooms); break;
            case DecorationStage::BigMushrooms:
                if (decorationIndex < bigMushroomsPerChunk)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    bigMushroomGen->generate(currentWorld, *randomGenerator, x, currentWorld->getHeightValue(x, z), z);
                    ++decorationIndex; return false;
                }
                nextStage(DecorationStage::YellowFlowers); break;
            case DecorationStage::YellowFlowers:
                {
                    int_t maxFlowers = flowersPerChunk;
                    if (currentWorld != nullptr && currentWorld->isLimitedWorld())
                    {
#if PLATFORM_CONSOLE_LOW || PLATFORM_WII
                        maxFlowers = 1;
#else
                        if (biome == BiomeGenBase::extremeHills || biome == BiomeGenBase::extremeHillsEdge)
                            maxFlowers = 2; // MCPE 0.6.0 mountain wildflowers
                        else if (biome == BiomeGenBase::plains)
                            maxFlowers = 4;
#endif
                    }
                    if (decorationIndex < maxFlowers)
                    {
                        const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                        const int_t y = randomGenerator->nextInt(128);
                        const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                        plantYellowGen->generate(currentWorld, *randomGenerator, x, y, z);
                        decorationStage = DecorationStage::RedFlowers; return false;
                    }
                }
                nextStage(DecorationStage::Grass); break;
            case DecorationStage::RedFlowers:
                if (randomGenerator->nextInt(4) == 0)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t y = randomGenerator->nextInt(128);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    plantRedGen->generate(currentWorld, *randomGenerator, x, y, z);
                }
                ++decorationIndex; decorationStage = DecorationStage::YellowFlowers; return false;
            case DecorationStage::Grass:
                {
                    int_t maxGrass = grassPerChunk;
                    if (currentWorld != nullptr && currentWorld->isLimitedWorld())
                    {
#if PLATFORM_CONSOLE_LOW || PLATFORM_WII
                        maxGrass = 2;
#else
                        if (biome == BiomeGenBase::extremeHills || biome == BiomeGenBase::extremeHillsEdge)
                            maxGrass = 4; // MCPE 0.6.0 mountain grass
                        else if (biome == BiomeGenBase::plains)
                            maxGrass = 8;
#endif
                    }
                    // The platform cap stays authoritative in BOTH profiles:
                    // the limited-world escape that used to sit in this
                    // condition is what let the MCPE plains value (8) run
                    // 4x past the bounded consoles' budget; the per-profile
                    // maxGrass above keeps the MCPE shape where no cap exists.
                    if (decorationIndex < maxGrass &&
                        (PLATFORM_POPULATE_GRASS_PER_CHUNK_MAX < 0 || decorationIndex < PLATFORM_POPULATE_GRASS_PER_CHUNK_MAX))
                    {
                        const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                        const int_t y = randomGenerator->nextInt(128);
                        const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                        WorldGenerator *generator = biome->func_48410_b(*randomGenerator);
                        if (generator != nullptr) generator->generate(currentWorld, *randomGenerator, x, y, z);
                        ++decorationIndex; return false;
                    }
                }
                nextStage(DecorationStage::DeadBushes); break;
            case DecorationStage::DeadBushes:
                if (decorationIndex < deadBushPerChunk)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t y = randomGenerator->nextInt(128);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    WorldGenDeadBush(Block::deadBush->blockID).generate(currentWorld, *randomGenerator, x, y, z);
                    ++decorationIndex; return false;
                }
                nextStage(DecorationStage::WaterLilies); break;
            case DecorationStage::WaterLilies:
                if (decorationIndex < waterlilyPerChunk)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    int_t y = randomGenerator->nextInt(128);
                    while (y > 0 && currentWorld->getBlockId(x, y - 1, z) == 0) --y;
                    waterlilyGen->generate(currentWorld, *randomGenerator, x, y, z);
                    ++decorationIndex; return false;
                }
                nextStage(DecorationStage::MushroomLoopBrown); break;
            case DecorationStage::MushroomLoopBrown:
                if (decorationIndex >= mushroomsPerChunk) { nextStage(DecorationStage::BrownMushroom); break; }
                if (randomGenerator->nextInt(4) == 0)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    mushroomBrownGen->generate(currentWorld, *randomGenerator, x, currentWorld->getHeightValue(x, z), z);
                }
                decorationStage = DecorationStage::MushroomLoopRed; return false;
            case DecorationStage::MushroomLoopRed:
                if (randomGenerator->nextInt(8) == 0)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    const int_t y = randomGenerator->nextInt(128);
                    mushroomRedGen->generate(currentWorld, *randomGenerator, x, y, z);
                }
                ++decorationIndex; decorationStage = DecorationStage::MushroomLoopBrown; return false;
            case DecorationStage::BrownMushroom:
                if (randomGenerator->nextInt(4) == 0)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t y = randomGenerator->nextInt(128);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    mushroomBrownGen->generate(currentWorld, *randomGenerator, x, y, z);
                }
                nextStage(DecorationStage::RedMushroom); return false;
            case DecorationStage::RedMushroom:
                if (randomGenerator->nextInt(8) == 0)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t y = randomGenerator->nextInt(128);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    mushroomRedGen->generate(currentWorld, *randomGenerator, x, y, z);
                }
                nextStage(DecorationStage::Reeds); return false;
            case DecorationStage::Reeds:
                if (decorationIndex < reedsPerChunk)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    const int_t y = randomGenerator->nextInt(128);
                    reedGen->generate(currentWorld, *randomGenerator, x, y, z);
                    ++decorationIndex; return false;
                }
                nextStage(DecorationStage::ExtraReeds); break;
            case DecorationStage::ExtraReeds:
                if (decorationIndex < 10)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t y = randomGenerator->nextInt(128);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    reedGen->generate(currentWorld, *randomGenerator, x, y, z);
                    ++decorationIndex; return false;
                }
                nextStage(DecorationStage::Pumpkin); break;
            case DecorationStage::Pumpkin:
                if (randomGenerator->nextInt(32) == 0)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t y = randomGenerator->nextInt(128);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    WorldGenPumpkin().generate(currentWorld, *randomGenerator, x, y, z);
                }
                nextStage(DecorationStage::Cacti); return false;
            case DecorationStage::Cacti:
                if (decorationIndex < cactiPerChunk)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t y = randomGenerator->nextInt(128);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    cactusGen->generate(currentWorld, *randomGenerator, x, y, z);
                    ++decorationIndex; return false;
                }
                nextStage(DecorationStage::WaterSprings); break;
            case DecorationStage::WaterSprings:
                if (generateLakes && decorationIndex < PLATFORM_POPULATE_WATER_SPRINGS)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t y = randomGenerator->nextInt(randomGenerator->nextInt(120) + 8);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    WorldGenLiquids(Block::waterMoving->blockID).generate(currentWorld, *randomGenerator, x, y, z);
                    ++decorationIndex; return false;
                }
                nextStage(DecorationStage::LavaSprings); break;
            case DecorationStage::LavaSprings:
                if (generateLakes && decorationIndex < PLATFORM_POPULATE_LAVA_SPRINGS)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t y = randomGenerator->nextInt(randomGenerator->nextInt(randomGenerator->nextInt(112) + 8) + 8);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    WorldGenLiquids(Block::lavaMoving->blockID).generate(currentWorld, *randomGenerator, x, y, z);
                    ++decorationIndex; return false;
                }
                nextStage(DecorationStage::Done); break;
            case DecorationStage::Done:
                return true;
            }
        }
    }
    catch (...)
    {
        finishDecoration();
        throw;
    }
}
