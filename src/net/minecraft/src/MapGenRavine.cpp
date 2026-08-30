#include "MapGenRavine.h"

#include "BiomeGenBase.h"
#include "Block.h"
#include "BlockGrass.h"
#include "MathHelper.h"
#include "World.h"
#include "WorldChunkManager.h"
#include "java/Arithmetic.h"
#include "java/Random.h"

#include <cmath>

namespace
{
using RavineReal = MapGenRavineReal;
constexpr float PI_F = 3.14159265358979323846f;

#if PLATFORM_FLOAT_CAVE_GENERATION
static inline int_t ravineFloor(RavineReal value)
{
    return MathHelper::floor_float(value);
}
#else
static inline int_t ravineFloor(RavineReal value)
{
    return MathHelper::floor_double(value);
}
#endif

template <typename T>
static inline RavineReal ravineReal(T value)
{
    return static_cast<RavineReal>(value);
}
}

MapGenRavine::MapGenRavine()
{
    for (float &value : ravineShape)
        value = 1.0f;
}

void MapGenRavine::generateRavine(long_t seed, int_t chunkX, int_t chunkZ, byte_t blocks[],
                                   RavineReal x, RavineReal y, RavineReal z, float width,
                                   float yaw, float pitch, int_t startStep, int_t endStep,
                                   RavineReal verticalScale)
{
    Random random(seed);
    const int_t chunkBlockX = JavaArithmetic::intMul(chunkX, 16);
    const int_t chunkBlockZ = JavaArithmetic::intMul(chunkZ, 16);
    const RavineReal centerX = ravineReal(JavaArithmetic::intAdd(chunkBlockX, 8));
    const RavineReal centerZ = ravineReal(JavaArithmetic::intAdd(chunkBlockZ, 8));
    float yawVelocity = 0.0f;
    float pitchVelocity = 0.0f;

    if (endStep <= 0)
    {
        const int_t maxSteps = JavaArithmetic::intSub(JavaArithmetic::intMul(range, 16), 16);
        endStep = maxSteps - random.nextInt(maxSteps / 4);
    }

    bool singleBranch = false;
    if (startStep == -1)
    {
        startStep = endStep / 2;
        singleBranch = true;
    }

    float shape = 1.0f;
    for (int_t i = 0; i < 128; ++i)
    {
        if (i == 0 || random.nextInt(3) == 0)
            shape = 1.0f + random.nextFloat() * random.nextFloat();
        ravineShape[i] = shape * shape;
    }

    for (; startStep < endStep; ++startStep)
    {
        RavineReal horizontalRadius = ravineReal(1.5) + ravineReal(
            MathHelper::sin(static_cast<float>(startStep) * PI_F /
                            static_cast<float>(endStep)) * width);
        RavineReal verticalRadius = horizontalRadius * verticalScale;
        horizontalRadius *= ravineReal(random.nextFloat()) * ravineReal(0.25) + ravineReal(0.75);
        verticalRadius *= ravineReal(random.nextFloat()) * ravineReal(0.25) + ravineReal(0.75);

        const float cosPitch = MathHelper::cos(pitch);
        const float sinPitch = MathHelper::sin(pitch);
        x += ravineReal(MathHelper::cos(yaw) * cosPitch);
        y += ravineReal(sinPitch);
        z += ravineReal(MathHelper::sin(yaw) * cosPitch);
        pitch *= 0.7f;
        pitch += pitchVelocity * 0.05f;
        yaw += yawVelocity * 0.05f;
        pitchVelocity *= 0.8f;
        yawVelocity *= 0.5f;
        pitchVelocity += random.nextFloatDifferenceTimesNextFloat() * 2.0f;
        yawVelocity += random.nextFloatDifferenceTimesNextFloat() * 4.0f;

        if (!singleBranch && random.nextInt(4) == 0)
            continue;

        const RavineReal dx = x - centerX;
        const RavineReal dz = z - centerZ;
        const RavineReal stepsRemaining = ravineReal(endStep - startStep);
        const RavineReal maxReach = ravineReal(width + 2.0f + 16.0f);
        if (dx * dx + dz * dz - stepsRemaining * stepsRemaining > maxReach * maxReach)
            return;

        if (x < centerX - ravineReal(16.0) - horizontalRadius * ravineReal(2.0) ||
            z < centerZ - ravineReal(16.0) - horizontalRadius * ravineReal(2.0) ||
            x > centerX + ravineReal(16.0) + horizontalRadius * ravineReal(2.0) ||
            z > centerZ + ravineReal(16.0) + horizontalRadius * ravineReal(2.0))
            continue;

        int_t minX = JavaArithmetic::intSub(JavaArithmetic::intSub(ravineFloor(x - horizontalRadius), chunkBlockX), 1);
        int_t maxX = JavaArithmetic::intAdd(JavaArithmetic::intSub(ravineFloor(x + horizontalRadius), chunkBlockX), 1);
        int_t minY = JavaArithmetic::intSub(ravineFloor(y - verticalRadius), 1);
        int_t maxY = JavaArithmetic::intAdd(ravineFloor(y + verticalRadius), 1);
        int_t minZ = JavaArithmetic::intSub(JavaArithmetic::intSub(ravineFloor(z - horizontalRadius), chunkBlockZ), 1);
        int_t maxZ = JavaArithmetic::intAdd(JavaArithmetic::intSub(ravineFloor(z + horizontalRadius), chunkBlockZ), 1);

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
                        if (blocks[index] == static_cast<byte_t>(Block::waterMoving->blockID) ||
                            blocks[index] == static_cast<byte_t>(Block::waterStill->blockID))
                            hitWater = true;

                        if (localY != minY - 1 && localX != minX && localX != maxX - 1 &&
                            localZ != minZ && localZ != maxZ - 1)
                            localY = minY;
                    }
                }
            }
        }

        if (hitWater)
            continue;

        for (int_t localX = minX; localX < maxX; ++localX)
        {
            const RavineReal normalizedX = (ravineReal(JavaArithmetic::intAdd(localX, chunkBlockX)) + ravineReal(0.5) - x) / horizontalRadius;
            const RavineReal normalizedXSquared = normalizedX * normalizedX;
            for (int_t localZ = minZ; localZ < maxZ; ++localZ)
            {
                const RavineReal normalizedZ = (ravineReal(JavaArithmetic::intAdd(localZ, chunkBlockZ)) + ravineReal(0.5) - z) / horizontalRadius;
                const RavineReal horizontalDistanceSquared =
                    normalizedXSquared + normalizedZ * normalizedZ;
                int_t index = (localX * 16 + localZ) * 128 + maxY;
                bool removedGrass = false;

                if (horizontalDistanceSquared >= ravineReal(1.0))
                    continue;

                for (int_t localY = maxY - 1; localY >= minY; --localY)
                {
                    const RavineReal normalizedY = (ravineReal(localY) + ravineReal(0.5) - y) / verticalRadius;
                    if (horizontalDistanceSquared * ravineReal(ravineShape[localY]) +
                        normalizedY * normalizedY / ravineReal(6.0) < ravineReal(1.0))
                    {
                        const byte_t block = blocks[index];
                        if (block == static_cast<byte_t>(Block::grass->blockID))
                            removedGrass = true;

                        if (block == static_cast<byte_t>(Block::stone->blockID) ||
                            block == static_cast<byte_t>(Block::dirt->blockID) ||
                            block == static_cast<byte_t>(Block::grass->blockID))
                        {
                            if (localY < 10)
                                blocks[index] = static_cast<byte_t>(Block::lavaMoving->blockID);
                            else
                            {
                                blocks[index] = 0;
                                if (removedGrass && index > 0 && blocks[index - 1] == static_cast<byte_t>(Block::dirt->blockID))
                                {
                                    BiomeGenBase *biome = worldObj->getWorldChunkManager()->getBiomeGenAt(
                                        JavaArithmetic::intAdd(localX, chunkBlockX), JavaArithmetic::intAdd(localZ, chunkBlockZ));
                                    blocks[index - 1] = biome->topBlock;
                                }
                            }
                        }
                    }
                    --index;
                }
            }
        }

        if (singleBranch)
            break;
    }
}

void MapGenRavine::generateChunk(World *world, int_t sourceChunkX, int_t sourceChunkZ,
                                  int_t targetChunkX, int_t targetChunkZ, byte_t blocks[])
{
    if (rand.nextInt(50) != 0)
        return;

    const RavineReal x = ravineReal(JavaArithmetic::intAdd(JavaArithmetic::intMul(sourceChunkX, 16), rand.nextInt(16)));
    const RavineReal y = ravineReal(rand.nextInt(rand.nextInt(40) + 8) + 20);
    const RavineReal z = ravineReal(JavaArithmetic::intAdd(JavaArithmetic::intMul(sourceChunkZ, 16), rand.nextInt(16)));

    const float yaw = rand.nextFloat() * PI_F * 2.0f;
    const float pitch = (rand.nextFloat() - 0.5f) * 2.0f / 8.0f;
	const float widthBase = rand.nextFloat();
	const float width = (widthBase * 2.0f + rand.nextFloat()) * 2.0f;
    generateRavine(rand.nextLong(), targetChunkX, targetChunkZ, blocks,
                   x, y, z, width, yaw, pitch, 0, 0, ravineReal(3.0));
}
