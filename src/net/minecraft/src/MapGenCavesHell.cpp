#include "MapGenCavesHell.h"
#include "java/Arithmetic.h"

#include <cmath>
#include "World.h"
#include "Block.h"
#include "MathHelper.h"
#include "BlockGrass.h"

namespace
{
using CaveHellReal = MapGenCavesHellReal;

#if PLATFORM_FLOAT_CAVE_GENERATION
static inline int_t caveHellFloor(CaveHellReal value)
{
    return MathHelper::floor_float(value);
}
#else
static inline int_t caveHellFloor(CaveHellReal value)
{
    return MathHelper::floor_double(value);
}
#endif

template <typename T>
static inline CaveHellReal caveHellReal(T value)
{
    return static_cast<CaveHellReal>(value);
}
}

MapGenCavesHell::MapGenCavesHell()
{
}

void MapGenCavesHell::generateCaveNode(int_t i, int_t j, byte_t abyte0[], CaveHellReal x, CaveHellReal y, CaveHellReal z)
{
    generateCaveRoom(i, j, abyte0, x, y, z,
        1.0f + rand.nextFloat() * 6.0f, 0.0f, 0.0f, -1, -1, caveHellReal(0.5));
}

void MapGenCavesHell::generateCaveRoom(int_t i, int_t j, byte_t abyte0[], CaveHellReal x, CaveHellReal y, CaveHellReal z,
    float width, float yaw, float pitch, int_t startStep, int_t endStep, CaveHellReal verticalScale)
{
    const int_t chunkBlockX = JavaArithmetic::intMul(i, 16);
    const int_t chunkBlockZ = JavaArithmetic::intMul(j, 16);
    const CaveHellReal centerX = caveHellReal(JavaArithmetic::intAdd(chunkBlockX, 8));
    const CaveHellReal centerZ = caveHellReal(JavaArithmetic::intAdd(chunkBlockZ, 8));
    float yawVelocity = 0.0f;
    float pitchVelocity = 0.0f;
    Random random(rand.nextLong());

    if (endStep <= 0)
    {
        const int_t maxSteps = JavaArithmetic::intSub(JavaArithmetic::intMul(range, 16), 16);
        endStep = JavaArithmetic::intSub(maxSteps, random.nextInt(maxSteps / 4));
    }

    bool singleBranch = false;
    if (startStep == -1)
    {
        startStep = endStep / 2;
        singleBranch = true;
    }

    const int_t branchStep = random.nextInt(endStep / 2) + endStep / 4;
    const bool gentlePitch = random.nextInt(6) == 0;

    for (; startStep < endStep; ++startStep)
    {
        const float radiusOffset = MathHelper::sin((static_cast<float>(startStep) * 3.1415927f) / static_cast<float>(endStep)) * width * 1.0f;
        const CaveHellReal radius = caveHellReal(1.5) + caveHellReal(radiusOffset);
        const CaveHellReal verticalRadius = radius * verticalScale;
        const float cosPitch = MathHelper::cos(pitch);
        const float sinPitch = MathHelper::sin(pitch);
        x += caveHellReal(MathHelper::cos(yaw) * cosPitch);
        y += caveHellReal(sinPitch);
        z += caveHellReal(MathHelper::sin(yaw) * cosPitch);

        pitch *= gentlePitch ? 0.92f : 0.7f;
        pitch += pitchVelocity * 0.1f;
        yaw += yawVelocity * 0.1f;
        pitchVelocity *= 0.9f;
        yawVelocity *= 0.75f;
        pitchVelocity += random.nextFloatDifferenceTimesNextFloat() * 2.0f;
        yawVelocity += random.nextFloatDifferenceTimesNextFloat() * 4.0f;

        if (!singleBranch && startStep == branchStep && width > 1.0f)
        {
            generateCaveRoom(i, j, abyte0, x, y, z,
                random.nextFloat() * 0.5f + 0.5f, yaw - 1.5707964f, pitch / 3.0f,
                startStep, endStep, caveHellReal(1.0));
            generateCaveRoom(i, j, abyte0, x, y, z,
                random.nextFloat() * 0.5f + 0.5f, yaw + 1.5707964f, pitch / 3.0f,
                startStep, endStep, caveHellReal(1.0));
            return;
        }

        if (!singleBranch && random.nextInt(4) == 0)
            continue;

        const CaveHellReal deltaX = x - centerX;
        const CaveHellReal deltaZ = z - centerZ;
        const CaveHellReal remaining = caveHellReal(endStep - startStep);
        const CaveHellReal maxReach = caveHellReal(width + 2.0f + 16.0f);
        if ((deltaX * deltaX + deltaZ * deltaZ) - remaining * remaining > maxReach * maxReach)
            return;

        if (x < centerX - caveHellReal(16.0) - radius * caveHellReal(2.0) ||
            z < centerZ - caveHellReal(16.0) - radius * caveHellReal(2.0) ||
            x > centerX + caveHellReal(16.0) + radius * caveHellReal(2.0) ||
            z > centerZ + caveHellReal(16.0) + radius * caveHellReal(2.0))
            continue;

        int_t minX = JavaArithmetic::intSub(JavaArithmetic::intSub(caveHellFloor(x - radius), chunkBlockX), 1);
        int_t maxX = JavaArithmetic::intAdd(JavaArithmetic::intSub(caveHellFloor(x + radius), chunkBlockX), 1);
        int_t minY = JavaArithmetic::intSub(caveHellFloor(y - verticalRadius), 1);
        int_t maxY = JavaArithmetic::intAdd(caveHellFloor(y + verticalRadius), 1);
        int_t minZ = JavaArithmetic::intSub(JavaArithmetic::intSub(caveHellFloor(z - radius), chunkBlockZ), 1);
        int_t maxZ = JavaArithmetic::intAdd(JavaArithmetic::intSub(caveHellFloor(z + radius), chunkBlockZ), 1);

        if (minX < 0) minX = 0;
        if (maxX > 16) maxX = 16;
        if (minY < 1) minY = 1;
        if (maxY > 120) maxY = 120;
        if (minZ < 0) minZ = 0;
        if (maxZ > 16) maxZ = 16;

        bool foundLava = false;
        for (int_t localX = minX; !foundLava && localX < maxX; ++localX)
        {
            for (int_t localZ = minZ; !foundLava && localZ < maxZ; ++localZ)
            {
                for (int_t blockY = maxY + 1; !foundLava && blockY >= minY - 1; --blockY)
                {
                    const int_t index = (localX * 16 + localZ) * 128 + blockY;
                    if (blockY < 0 || blockY >= 128)
                        continue;
                    if (abyte0[index] == Block::lavaMoving->blockID || abyte0[index] == Block::lavaStill->blockID)
                        foundLava = true;
                    if (blockY != minY - 1 && localX != minX && localX != maxX - 1 && localZ != minZ && localZ != maxZ - 1)
                        blockY = minY;
                }
            }
        }

        if (foundLava)
            continue;

#if PLATFORM_FLOAT_CAVE_GENERATION
        const CaveHellReal inverseRadius = caveHellReal(1.0) / radius;
        const CaveHellReal inverseVerticalRadius = caveHellReal(1.0) / verticalRadius;
#endif
        for (int_t localX = minX; localX < maxX; ++localX)
        {
#if PLATFORM_FLOAT_CAVE_GENERATION
            const CaveHellReal normalizedX = (caveHellReal(JavaArithmetic::intAdd(localX, chunkBlockX)) + caveHellReal(0.5) - x) * inverseRadius;
#else
            const CaveHellReal normalizedX = (caveHellReal(JavaArithmetic::intAdd(localX, chunkBlockX)) + caveHellReal(0.5) - x) / radius;
#endif
            const CaveHellReal normalizedXSq = normalizedX * normalizedX;
            for (int_t localZ = minZ; localZ < maxZ; ++localZ)
            {
#if PLATFORM_FLOAT_CAVE_GENERATION
                const CaveHellReal normalizedZ = (caveHellReal(JavaArithmetic::intAdd(localZ, chunkBlockZ)) + caveHellReal(0.5) - z) * inverseRadius;
#else
                const CaveHellReal normalizedZ = (caveHellReal(JavaArithmetic::intAdd(localZ, chunkBlockZ)) + caveHellReal(0.5) - z) / radius;
#endif
#if PLATFORM_FLOAT_CAVE_GENERATION
                const CaveHellReal horizontalSq = normalizedXSq + normalizedZ * normalizedZ;
#endif
                int_t index = (localX * 16 + localZ) * 128 + maxY;
                for (int_t blockY = maxY - 1; blockY >= minY; --blockY)
                {
#if PLATFORM_FLOAT_CAVE_GENERATION
                    const CaveHellReal normalizedY = (caveHellReal(blockY) + caveHellReal(0.5) - y) * inverseVerticalRadius;
                    const bool insideCave = horizontalSq + normalizedY * normalizedY < caveHellReal(1.0);
#else
                    const CaveHellReal normalizedY = (caveHellReal(blockY) + caveHellReal(0.5) - y) / verticalRadius;
                    const bool insideCave = normalizedX * normalizedX + normalizedY * normalizedY + normalizedZ * normalizedZ < caveHellReal(1.0);
#endif
                    if (normalizedY > caveHellReal(-0.7) && insideCave)
                    {
                        const byte_t blockId = abyte0[index];
                        if (blockId == Block::netherrack->blockID || blockId == Block::dirt->blockID || blockId == Block::grass->blockID)
                            abyte0[index] = 0;
                    }
                    --index;
                }
            }
        }

        if (singleBranch)
            break;
    }
}

void MapGenCavesHell::generateChunk(World *world, int_t i, int_t j, int_t k, int_t l, byte_t abyte0[])
{
	int_t i1 = rand.nextInt(rand.nextInt(rand.nextInt(10) + 1) + 1);
	if (rand.nextInt(5) != 0)
		i1 = 0;

	for (int_t j1 = 0; j1 < i1; j1++)
	{
		const CaveHellReal d = caveHellReal(JavaArithmetic::intAdd(JavaArithmetic::intMul(i, 16), rand.nextInt(16)));
		const CaveHellReal d1 = caveHellReal(rand.nextInt(128));
		const CaveHellReal d2 = caveHellReal(JavaArithmetic::intAdd(JavaArithmetic::intMul(j, 16), rand.nextInt(16)));
		int_t k1 = 1;
		if (rand.nextInt(4) == 0)
		{
			generateCaveNode(k, l, abyte0, d, d1, d2);
			k1 += rand.nextInt(4);
		}
		for (int_t l1 = 0; l1 < k1; l1++)
		{
			float f = rand.nextFloat() * 3.1415927f * 2.0f;
			float f1 = ((rand.nextFloat() - 0.5f) * 2.0f) / 8.0f;
			const float f2First = rand.nextFloat();
			const float f2Second = rand.nextFloat();
			float f2 = f2First * 2.0f + f2Second;
			generateCaveRoom(k, l, abyte0, d, d1, d2, f2 * 2.0f, f, f1, 0, 0, caveHellReal(0.5));
		}
	}
}
