#pragma once

#include "MapGenBase.h"
#include "platform/PlatformTuning.h"

#if PLATFORM_FLOAT_CAVE_GENERATION
using MapGenCaveReal = float;
#else
using MapGenCaveReal = double;
#endif

// net.minecraft.src.MapGenCaves
class MapGenCaves : public MapGenBase
{
public:
    MapGenCaves();

protected:
    void generateChunk(World *world, int_t sourceChunkX, int_t sourceChunkZ,
                       int_t targetChunkX, int_t targetChunkZ, byte_t blocks[]) override;

private:
    void generateLargeCaveNode(long_t seed, int_t chunkX, int_t chunkZ, byte_t blocks[],
                               MapGenCaveReal x, MapGenCaveReal y, MapGenCaveReal z);
    void generateCaveNode(long_t seed, int_t chunkX, int_t chunkZ, byte_t blocks[],
                          MapGenCaveReal x, MapGenCaveReal y, MapGenCaveReal z,
                          float width, float yaw, float pitch, int_t startStep,
                          int_t endStep, MapGenCaveReal verticalScale);
};
