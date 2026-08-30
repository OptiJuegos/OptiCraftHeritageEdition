#pragma once

#include "MapGenBase.h"
#include "platform/PlatformTuning.h"

#if PLATFORM_FLOAT_CAVE_GENERATION
using MapGenCavesHellReal = float;
#else
using MapGenCavesHellReal = double;
#endif

// net.minecraft.src.MapGenCavesHell
class MapGenCavesHell : public MapGenBase
{
public:
	MapGenCavesHell();

protected:
	void generateChunk(World *world, int_t i, int_t j, int_t k, int_t l, byte_t abyte0[]) override;

private:
	void generateCaveNode(int_t i, int_t j, byte_t abyte0[], MapGenCavesHellReal x, MapGenCavesHellReal y, MapGenCavesHellReal z);
	void generateCaveRoom(int_t i, int_t j, byte_t abyte0[], MapGenCavesHellReal x, MapGenCavesHellReal y, MapGenCavesHellReal z,
		float width, float yaw, float pitch, int_t startStep, int_t endStep, MapGenCavesHellReal verticalScale);
};
