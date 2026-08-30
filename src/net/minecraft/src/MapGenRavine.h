#pragma once

#include "MapGenBase.h"
#include "platform/PlatformTuning.h"

#if PLATFORM_FLOAT_CAVE_GENERATION
using MapGenRavineReal = float;
#else
using MapGenRavineReal = double;
#endif

// net.minecraft.src.MapGenRavine
class MapGenRavine : public MapGenBase
{
public:
    MapGenRavine();

protected:
    void generateChunk(World *world, int_t sourceChunkX, int_t sourceChunkZ,
                       int_t targetChunkX, int_t targetChunkZ, byte_t blocks[]) override;

private:
    void generateRavine(long_t seed, int_t chunkX, int_t chunkZ, byte_t blocks[],
                        MapGenRavineReal x, MapGenRavineReal y, MapGenRavineReal z, float width,
                        float yaw, float pitch, int_t startStep, int_t endStep,
                        MapGenRavineReal verticalScale);

    float ravineShape[1024];
};
