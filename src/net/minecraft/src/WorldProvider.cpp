#include "WorldProvider.h"
#include "World.h"
#include "WorldChunkManager.h"
#include "ChunkProviderGenerate.h"
#include "Block.h"
#include "BlockGrass.h"
#include "BiomeGenBase.h"
#include "WorldProviderHell.h"
#include "WorldProviderSurface.h"
#include "WorldProviderEnd.h"
#include "WorldInfo.h"
#include "WorldType.h"
#include "ChunkProviderFlat.h"
#include "WorldChunkManagerHell.h"
#include <cmath>
#include "java/Arithmetic.h"
#include "java/Math.h"

WorldProvider::WorldProvider()
    : terrainType(nullptr), isNether(false), isHellWorld(false), hasNoSky(false), worldType(0)
{
    for (int_t i = 0; i < 16; i++) lightBrightnessTable[i] = 0.0f;
    for (int_t i = 0; i < 4; i++) colorsSunriseSunset[i] = 0.0f;
}

WorldProvider::~WorldProvider()
{
    delete worldChunkMgr;
}

void WorldProvider::registerWorld(World *world)
{
    worldObj = world;
    terrainType = worldObj != nullptr && worldObj->getWorldInfo() != nullptr
        ? worldObj->getWorldInfo()->getTerrainType() : WorldType::DEFAULT;
    registerWorldChunkManager();
    generateLightBrightnessTable();
}

void WorldProvider::generateLightBrightnessTable()
{
    float f = 0.0f;
    for (int_t i = 0; i <= 15; i++)
    {
        float f1 = 1.0f - (float)i / 15.0f;
        lightBrightnessTable[i] = ((1.0f - f1) / (f1 * 3.0f + 1.0f)) * (1.0f - f) + f;
    }
}

void WorldProvider::registerWorldChunkManager()
{
    if (terrainType == WorldType::FLAT)
        worldChunkMgr = new WorldChunkManagerHell(BiomeGenBase::plains, 0.5f, 0.5f);
    else
        worldChunkMgr = new WorldChunkManager(worldObj);
}

IChunkProvider *WorldProvider::getChunkProvider()
{
    const bool flat = terrainType == WorldType::FLAT;
    const bool features = worldObj != nullptr && worldObj->getWorldInfo() != nullptr
        ? worldObj->getWorldInfo()->isMapFeaturesEnabled() : true;
    return flat
        ? static_cast<IChunkProvider *>(new ChunkProviderFlat(worldObj, worldObj->getRandomSeed(), features))
        : static_cast<IChunkProvider *>(new ChunkProviderGenerate(worldObj, worldObj->getRandomSeed(), features));
}

bool WorldProvider::canCoordinateBeSpawn(int_t i, int_t j)
{
    int_t k = worldObj->getFirstUncoveredBlock(i, j);
    return k == Block::grass->blockID;
}

float WorldProvider::calculateCelestialAngle(long_t l, float f)
{
    int_t i = (int_t)(l % 24000LL);
    float f1 = ((float)i + f) / 24000.0f - 0.25f;
    if (f1 < 0.0f) f1++;
    if (f1 > 1.0f) f1--;
    float f2 = f1;
    f1 = 1.0f - (float)((JavaMath::cos((double)f1 * 3.1415926535897931) + 1.0) / 2.0);
    f1 = f2 + (f1 - f2) / 3.0f;
    return f1;
}

float *WorldProvider::calcSunriseSunsetColors(float f, float f1)
{
    float f2 = 0.4f;
    float f3 = MathHelper::cos(f * 3.1415927f * 2.0f) - 0.0f;
    if (f3 >= -f2 && f3 <= f2)
    {
        float f5 = (f3 / f2) * 0.5f + 0.5f;
        float f6 = 1.0f - (1.0f - MathHelper::sin(f5 * 3.1415927f)) * 0.99f;
        f6 *= f6;
        colorsSunriseSunset[0] = f5 * 0.3f + 0.7f;
        colorsSunriseSunset[1] = f5 * f5 * 0.7f + 0.2f;
        colorsSunriseSunset[2] = f5 * f5 * 0.0f + 0.2f;
        colorsSunriseSunset[3] = f6;
        return colorsSunriseSunset;
    }
    return nullptr;
}

Vec3D *WorldProvider::getFogColor(float f, float f1)
{
    float f2 = MathHelper::cos(f * 3.1415927f * 2.0f) * 2.0f + 0.5f;
    if (f2 < 0.0f) f2 = 0.0f;
    if (f2 > 1.0f) f2 = 1.0f;
    float f3 = 0.7529412f, f4 = 0.8470588f, f5 = 1.0f;
    f3 *= f2 * 0.94f + 0.06f;
    f4 *= f2 * 0.94f + 0.06f;
    f5 *= f2 * 0.91f + 0.09f;
    return Vec3D::createVector(f3, f4, f5);
}

bool WorldProvider::canRespawnHere()  { return true; }

int_t WorldProvider::getMoonPhase(long_t worldTime, float)
{
    return JavaArithmetic::longToInt(worldTime / 24000LL) % 8;
}

bool WorldProvider::func_48217_e() { return true; }
float WorldProvider::getCloudHeight() { return 128.0f; }
bool WorldProvider::isSkyColored() { return true; }
ChunkCoordinates *WorldProvider::getEntrancePortalLocation() { return nullptr; }

int_t WorldProvider::getAverageGroundLevel()
{
    return terrainType == WorldType::FLAT ? 4 : 64;
}

bool WorldProvider::getWorldHasNoSky()
{
    return terrainType != WorldType::FLAT && !hasNoSky;
}

double WorldProvider::getVoidFogYFactor()
{
    return terrainType == WorldType::FLAT ? 1.0 : 1.0 / 32.0;
}

bool WorldProvider::hasVoidParticles() { return func_48217_e(); }
bool WorldProvider::func_48218_b(int_t, int_t) { return false; }

WorldProvider *WorldProvider::getProviderForDimension(int_t i)
{
    if (i == -1) return new WorldProviderHell();
    if (i ==  0) return new WorldProviderSurface();
    if (i ==  1) return new WorldProviderEnd();
    return nullptr;
}
