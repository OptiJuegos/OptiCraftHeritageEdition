#include "GenLayer.h"

#include "GenLayerAddIsland.h"
#include "GenLayerAddMushroomIsland.h"
#include "GenLayerAddSnow.h"
#include "GenLayerBiome.h"
#include "GenLayerFuzzyZoom.h"
#include "GenLayerHills.h"
#include "GenLayerIsland.h"
#include "GenLayerRiver.h"
#include "GenLayerRiverInit.h"
#include "GenLayerRiverMix.h"
#include "GenLayerShore.h"
#include "GenLayerSmooth.h"
#include "GenLayerSwampRivers.h"
#include "GenLayerVoronoiZoom.h"
#include "GenLayerZoom.h"
#include "WorldType.h"
#include "java/Arithmetic.h"

namespace
{
constexpr long_t GEN_LAYER_MULTIPLIER = 6364136223846793005LL;
constexpr long_t GEN_LAYER_ADDEND = 1442695040888963407LL;
}

long_t GenLayer::mixSeed(long_t seed, long_t addend)
{
    const long_t multiplied = JavaArithmetic::longMul(seed, GEN_LAYER_MULTIPLIER);
    const long_t factor = JavaArithmetic::longAdd(multiplied, GEN_LAYER_ADDEND);
    return JavaArithmetic::longAdd(JavaArithmetic::longMul(seed, factor), addend);
}

GenLayer::GenLayer(long_t seed)
    : worldGenSeed(0),
      chunkSeed(0),
      baseSeed(seed)
{
    baseSeed = mixSeed(baseSeed, seed);
    baseSeed = mixSeed(baseSeed, seed);
    baseSeed = mixSeed(baseSeed, seed);
}

void GenLayer::initWorldGenSeed(long_t worldSeedValue)
{
    worldGenSeed = worldSeedValue;
    if (parent)
        parent->initWorldGenSeed(worldSeedValue);

    worldGenSeed = mixSeed(worldGenSeed, baseSeed);
    worldGenSeed = mixSeed(worldGenSeed, baseSeed);
    worldGenSeed = mixSeed(worldGenSeed, baseSeed);
}

void GenLayer::initChunkSeed(long_t x, long_t z)
{
    chunkSeed = worldGenSeed;
    chunkSeed = mixSeed(chunkSeed, x);
    chunkSeed = mixSeed(chunkSeed, z);
    chunkSeed = mixSeed(chunkSeed, x);
    chunkSeed = mixSeed(chunkSeed, z);
}

int_t GenLayer::nextInt(int_t bound)
{
    const long_t shifted = JavaArithmetic::longShr(chunkSeed, 24);
    int_t value = static_cast<int_t>(shifted % static_cast<long_t>(bound));
    if (value < 0)
        value += bound;

    chunkSeed = mixSeed(chunkSeed, worldGenSeed);
    return value;
}

std::array<GenLayer::Ptr, 3> GenLayer::func_48425_a(long_t worldSeed, WorldType *worldType)
{
    Ptr layer = std::make_shared<GenLayerIsland>(1LL);
    layer = std::make_shared<GenLayerFuzzyZoom>(2000LL, layer);
    layer = std::make_shared<GenLayerAddIsland>(1LL, layer);
    layer = std::make_shared<GenLayerZoom>(2001LL, layer);
    layer = std::make_shared<GenLayerAddIsland>(2LL, layer);
    layer = std::make_shared<GenLayerAddSnow>(2LL, layer);
    layer = std::make_shared<GenLayerZoom>(2002LL, layer);
    layer = std::make_shared<GenLayerAddIsland>(3LL, layer);
    layer = std::make_shared<GenLayerZoom>(2003LL, layer);
    layer = std::make_shared<GenLayerAddIsland>(4LL, layer);
    Ptr mushroomLayer = std::make_shared<GenLayerAddMushroomIsland>(5LL, layer);

    constexpr int_t biomeSize = 4;

    Ptr riverLayer = GenLayerZoom::func_35515_a(1000LL, mushroomLayer, 0);
    riverLayer = std::make_shared<GenLayerRiverInit>(100LL, riverLayer);
    riverLayer = GenLayerZoom::func_35515_a(1000LL, riverLayer, biomeSize + 2);
    riverLayer = std::make_shared<GenLayerRiver>(1LL, riverLayer);
    riverLayer = std::make_shared<GenLayerSmooth>(1000LL, riverLayer);

    Ptr biomeLayer = GenLayerZoom::func_35515_a(1000LL, mushroomLayer, 0);
    biomeLayer = std::make_shared<GenLayerBiome>(200LL, biomeLayer, worldType);
    biomeLayer = GenLayerZoom::func_35515_a(1000LL, biomeLayer, 2);
    biomeLayer = std::make_shared<GenLayerHills>(1000LL, biomeLayer);

    for (int_t i = 0; i < biomeSize; ++i)
    {
        biomeLayer = std::make_shared<GenLayerZoom>(1000LL + i, biomeLayer);
        if (i == 0)
            biomeLayer = std::make_shared<GenLayerAddIsland>(3LL, biomeLayer);
        if (i == 1)
            biomeLayer = std::make_shared<GenLayerShore>(1000LL, biomeLayer);
        if (i == 1)
            biomeLayer = std::make_shared<GenLayerSwampRivers>(1000LL, biomeLayer);
    }

    biomeLayer = std::make_shared<GenLayerSmooth>(1000LL, biomeLayer);
    Ptr riverMix = std::make_shared<GenLayerRiverMix>(100LL, biomeLayer, riverLayer);
    Ptr voronoi = std::make_shared<GenLayerVoronoiZoom>(10LL, riverMix);
    riverMix->initWorldGenSeed(worldSeed);
    voronoi->initWorldGenSeed(worldSeed);

    return {riverMix, voronoi, riverMix};
}
