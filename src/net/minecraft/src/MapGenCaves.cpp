#include "MapGenCaves.h"

#include "BiomeGenBase.h"
#include "Block.h"
#include "BlockGrass.h"
#include "MathHelper.h"
#include "World.h"
#include "WorldChunkManager.h"
#include "java/Arithmetic.h"
#include "platform/PlatformTuning.h"

#include <cmath>

namespace
{
using CaveReal = MapGenCaveReal;
constexpr float PI_F = 3.14159265358979323846f;

#if PLATFORM_FLOAT_CAVE_GENERATION
static inline int_t caveFloor(CaveReal value)
{
    return MathHelper::floor_float(value);
}
#else
static inline int_t caveFloor(CaveReal value)
{
    return MathHelper::floor_double(value);
}
#endif

template <typename T>
static inline CaveReal caveReal(T value)
{
    return static_cast<CaveReal>(value);
}
}

MapGenCaves::MapGenCaves()
{
}

void MapGenCaves::generateLargeCaveNode(long_t seed, int_t chunkX, int_t chunkZ,
                                        byte_t blocks[], CaveReal x, CaveReal y, CaveReal z)
{
    generateCaveNode(seed, chunkX, chunkZ, blocks, x, y, z,
                     1.0f + rand.nextFloat() * 6.0f, 0.0f, 0.0f,
                     -1, -1, caveReal(0.5));
}

void MapGenCaves::generateCaveNode(long_t seed, int_t chunkX, int_t chunkZ,
                                   byte_t blocks[], CaveReal x, CaveReal y, CaveReal z,
                                   float width, float yaw, float pitch, int_t startStep,
                                   int_t endStep, CaveReal verticalScale)
{
    const int_t chunkBlockX = JavaArithmetic::intMul(chunkX, 16);
    const int_t chunkBlockZ = JavaArithmetic::intMul(chunkZ, 16);
    const CaveReal chunkCenterX = caveReal(JavaArithmetic::intAdd(chunkBlockX, 8));
    const CaveReal chunkCenterZ = caveReal(JavaArithmetic::intAdd(chunkBlockZ, 8));
    float yawVelocity = 0.0f;
    float pitchVelocity = 0.0f;
    Random nodeRandom(seed);

    if (endStep <= 0)
    {
        const int_t maxSteps = JavaArithmetic::intSub(JavaArithmetic::intMul(range, 16), 16);
        endStep = JavaArithmetic::intSub(maxSteps, nodeRandom.nextInt(maxSteps / 4));
    }

    bool isLargeNode = false;
    if (startStep == -1)
    {
        startStep = endStep / 2;
        isLargeNode = true;
    }

    const int_t branchStep = nodeRandom.nextInt(endStep / 2) + endStep / 4;
    const bool gentlePitch = nodeRandom.nextInt(6) == 0;

    for (; startStep < endStep; ++startStep)
    {
        const CaveReal horizontalRadius = caveReal(1.5) + caveReal(
            MathHelper::sin(static_cast<float>(startStep) * PI_F /
                            static_cast<float>(endStep)) * width);
        const CaveReal verticalRadius = horizontalRadius * verticalScale;
        const float cosPitch = MathHelper::cos(pitch);
        const float sinPitch = MathHelper::sin(pitch);
        x += caveReal(MathHelper::cos(yaw) * cosPitch);
        y += caveReal(sinPitch);
        z += caveReal(MathHelper::sin(yaw) * cosPitch);

        pitch *= gentlePitch ? 0.92f : 0.7f;
        pitch += pitchVelocity * 0.1f;
        yaw += yawVelocity * 0.1f;
        pitchVelocity *= 0.9f;
        yawVelocity *= 0.75f;
        pitchVelocity += nodeRandom.nextFloatDifferenceTimesNextFloat() * 2.0f;
        yawVelocity += nodeRandom.nextFloatDifferenceTimesNextFloat() * 4.0f;

        if (!isLargeNode && startStep == branchStep && width > 1.0f && endStep > 0)
        {
            const long_t leftSeed = nodeRandom.nextLong();
            const float leftWidth = nodeRandom.nextFloat() * 0.5f + 0.5f;
            generateCaveNode(leftSeed, chunkX, chunkZ, blocks, x, y, z,
                             leftWidth, yaw - PI_F * 0.5f, pitch / 3.0f,
                             startStep, endStep, caveReal(1.0));

            const long_t rightSeed = nodeRandom.nextLong();
            const float rightWidth = nodeRandom.nextFloat() * 0.5f + 0.5f;
            generateCaveNode(rightSeed, chunkX, chunkZ, blocks, x, y, z,
                             rightWidth, yaw + PI_F * 0.5f, pitch / 3.0f,
                             startStep, endStep, caveReal(1.0));
            return;
        }

        if (!isLargeNode && nodeRandom.nextInt(4) == 0)
            continue;

        const CaveReal dx = x - chunkCenterX;
        const CaveReal dz = z - chunkCenterZ;
        const CaveReal remaining = caveReal(endStep - startStep);
        const CaveReal maxDistance = caveReal(width + 2.0f + 16.0f);
        if (dx * dx + dz * dz - remaining * remaining > maxDistance * maxDistance)
            return;

        if (x < chunkCenterX - caveReal(16.0) - horizontalRadius * caveReal(2.0) ||
            z < chunkCenterZ - caveReal(16.0) - horizontalRadius * caveReal(2.0) ||
            x > chunkCenterX + caveReal(16.0) + horizontalRadius * caveReal(2.0) ||
            z > chunkCenterZ + caveReal(16.0) + horizontalRadius * caveReal(2.0))
            continue;

        int_t minX = JavaArithmetic::intSub(
            JavaArithmetic::intSub(caveFloor(x - horizontalRadius), chunkBlockX), 1);
        int_t maxX = JavaArithmetic::intAdd(
            JavaArithmetic::intSub(caveFloor(x + horizontalRadius), chunkBlockX), 1);
        int_t minY = JavaArithmetic::intSub(caveFloor(y - verticalRadius), 1);
        int_t maxY = JavaArithmetic::intAdd(caveFloor(y + verticalRadius), 1);
        int_t minZ = JavaArithmetic::intSub(
            JavaArithmetic::intSub(caveFloor(z - horizontalRadius), chunkBlockZ), 1);
        int_t maxZ = JavaArithmetic::intAdd(
            JavaArithmetic::intSub(caveFloor(z + horizontalRadius), chunkBlockZ), 1);

        if (minX < 0) minX = 0;
        if (maxX > 16) maxX = 16;
        if (minY < 1) minY = 1;
        if (maxY > 120) maxY = 120;
        if (minZ < 0) minZ = 0;
        if (maxZ > 16) maxZ = 16;

        bool hitWater = false;
        for (int_t localX = minX; !hitWater && localX < maxX; ++localX)
        {
            for (int_t localZ = minZ; !hitWater && localZ < maxZ; ++localZ)
            {
                for (int_t localY = maxY + 1; !hitWater && localY >= minY - 1; --localY)
                {
                    const int_t index = (localX * 16 + localZ) * 128 + localY;
                    if (localY >= 0 && localY < 128)
                    {
                        if (blocks[index] == Block::waterMoving->blockID ||
                            blocks[index] == Block::waterStill->blockID)
                        {
                            hitWater = true;
                        }

                        if (localY != minY - 1 && localX != minX && localX != maxX - 1 &&
                            localZ != minZ && localZ != maxZ - 1)
                        {
                            localY = minY;
                        }
                    }
                }
            }
        }

        if (hitWater)
            continue;

        for (int_t localX = minX; localX < maxX; ++localX)
        {
            const CaveReal normalizedX =
                (caveReal(JavaArithmetic::intAdd(localX, chunkBlockX)) +
                 caveReal(0.5) - x) / horizontalRadius;
            const CaveReal normalizedXSquared = normalizedX * normalizedX;

            for (int_t localZ = minZ; localZ < maxZ; ++localZ)
            {
                const CaveReal normalizedZ =
                    (caveReal(JavaArithmetic::intAdd(localZ, chunkBlockZ)) +
                     caveReal(0.5) - z) / horizontalRadius;
                const CaveReal normalizedZSquared = normalizedZ * normalizedZ;
                int_t index = (localX * 16 + localZ) * 128 + maxY;
                bool removedGrass = false;

                if (normalizedXSquared + normalizedZSquared >= caveReal(1.0))
                    continue;

                for (int_t localY = maxY - 1; localY >= minY; --localY)
                {
                    const CaveReal normalizedY = (caveReal(localY) + caveReal(0.5) - y) / verticalRadius;
                    if (normalizedY > caveReal(-0.7) &&
                        normalizedXSquared + normalizedY * normalizedY +
                            normalizedZSquared < caveReal(1.0))
                    {
                        const byte_t blockId = blocks[index];
                        if (blockId == Block::grass->blockID)
                            removedGrass = true;

                        if (blockId == Block::stone->blockID || blockId == Block::dirt->blockID ||
                            blockId == Block::grass->blockID)
                        {
                            if (localY < 10)
                            {
                                blocks[index] = static_cast<byte_t>(Block::lavaMoving->blockID);
                            }
                            else
                            {
                                blocks[index] = 0;
                                if (removedGrass && blocks[index - 1] == Block::dirt->blockID)
                                {
                                    BiomeGenBase *biome = worldObj->getWorldChunkManager()->getBiomeGenAt(
                                        JavaArithmetic::intAdd(localX, chunkBlockX),
                                        JavaArithmetic::intAdd(localZ, chunkBlockZ));
                                    blocks[index - 1] = biome != nullptr ? biome->topBlock :
                                        static_cast<byte_t>(Block::grass->blockID);
                                }
                            }
                        }
                    }
                    --index;
                }
            }
        }

        if (isLargeNode)
            break;
    }
}

void MapGenCaves::generateChunk(World *, int_t sourceChunkX, int_t sourceChunkZ,
                                int_t targetChunkX, int_t targetChunkZ, byte_t blocks[])
{
    int_t caveCount = rand.nextInt(rand.nextInt(rand.nextInt(40) + 1) + 1);
    if (rand.nextInt(PLATFORM_CAVE_RARITY) != 0)
        caveCount = 0;

    for (int_t caveIndex = 0; caveIndex < caveCount; ++caveIndex)
    {
        const CaveReal x = caveReal(JavaArithmetic::intAdd(
            JavaArithmetic::intMul(sourceChunkX, 16), rand.nextInt(16)));
        const CaveReal y = caveReal(rand.nextInt(rand.nextInt(120) + 8));
        const CaveReal z = caveReal(JavaArithmetic::intAdd(
            JavaArithmetic::intMul(sourceChunkZ, 16), rand.nextInt(16)));

        int_t nodeCount = 1;
        if (rand.nextInt(4) == 0)
        {
            const long_t largeSeed = rand.nextLong();
            generateLargeCaveNode(largeSeed, targetChunkX, targetChunkZ, blocks, x, y, z);
            nodeCount += rand.nextInt(4);
        }

        for (int_t node = 0; node < nodeCount; ++node)
        {
            const float yaw = rand.nextFloat() * PI_F * 2.0f;
            const float pitch = (rand.nextFloat() - 0.5f) * 2.0f / 8.0f;
			const float widthBase = rand.nextFloat();
			float width = widthBase * 2.0f + rand.nextFloat();
            if (rand.nextInt(10) == 0)
                width *= rand.nextFloat() * rand.nextFloat() * 3.0f + 1.0f;

            const long_t seed = rand.nextLong();
            generateCaveNode(seed, targetChunkX, targetChunkZ, blocks, x, y, z,
                             width, yaw, pitch, 0, 0, caveReal(1.0));
        }
    }
}
