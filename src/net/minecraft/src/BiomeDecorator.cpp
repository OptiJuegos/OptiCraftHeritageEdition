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

BiomeDecorator::BiomeDecorator(BiomeGenBase *biomeValue)
    : generateLakes(true), currentWorld(nullptr), randomGenerator(nullptr),
      chunk_X(0), chunk_Z(0), biome(biomeValue), decorationStage(DecorationStage::Done),
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

void BiomeDecorator::beginDecoration(World *world, Random &random, int_t chunkX, int_t chunkZ)
{
    if (currentWorld != nullptr)
        throw std::runtime_error("Already decorating!!");
    currentWorld = world;
    randomGenerator = &random;
    chunk_X = chunkX;
    chunk_Z = chunkZ;
    decorationStage = DecorationStage::Dirt;
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
    decorationStage = stage;
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
                treeCount = treesPerChunk;
                if (randomGenerator->nextInt(10) == 0) ++treeCount;
                nextStage(DecorationStage::Trees); break;
            case DecorationStage::Trees:
                if (decorationIndex < treeCount)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    WorldGenerator *generator = biome->getRandomWorldGenForTrees(*randomGenerator);
                    if (generator != nullptr)
                    {
                        generator->setScale(1.0, 1.0, 1.0);
                        generator->generate(currentWorld, *randomGenerator, x, currentWorld->getHeightValue(x, z), z);
                    }
                    biome->releaseWorldGenForTrees(generator);
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
                if (decorationIndex < flowersPerChunk)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t y = randomGenerator->nextInt(128);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    plantYellowGen->generate(currentWorld, *randomGenerator, x, y, z);
                    decorationStage = DecorationStage::RedFlowers; return false;
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
                if (decorationIndex < grassPerChunk)
                {
                    const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_X, randomGenerator->nextInt(16)), 8);
                    const int_t y = randomGenerator->nextInt(128);
                    const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(chunk_Z, randomGenerator->nextInt(16)), 8);
                    std::unique_ptr<WorldGenerator> generator(biome->func_48410_b(*randomGenerator));
                    if (generator != nullptr) generator->generate(currentWorld, *randomGenerator, x, y, z);
                    ++decorationIndex; return false;
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
