#include "Explosion.h"
#include "java/Arithmetic.h"
#include "DamageSource.h"

#include "World.h"
#include "java/Math.h"
#include "Entity.h"
#include "Block.h"
#include "MathHelper.h"
#include "AxisAlignedBB.h"
#include "Vec3D.h"
#include "ChunkPosition.h"
#include <cmath>
#include <vector>
#include <cstdint>
#include "BlockFire.h"
#include "platform/PlatformTuning.h"


std::size_t ChunkPosTupleHash::operator()(const ChunkPosTuple &p) const
{
    const std::uint32_t h = static_cast<std::uint32_t>(p.x) * UINT32_C(0x88f9fa) +
                            static_cast<std::uint32_t>(p.y) * UINT32_C(0xef88b) +
                            static_cast<std::uint32_t>(p.z);
    return static_cast<std::size_t>(h);
}

Explosion::Explosion(World *world, Entity *entity, double x, double y, double z, float size) :
    isFlaming(false),
    explosionX(x),
    explosionY(y),
    explosionZ(z),
    exploder(entity),
    explosionSize(size),
    worldObj(world)
{
}

void Explosion::doExplosionA()
{
    float f = explosionSize;
    int_t i = 16;

#if PLATFORM_FLOAT_EXPLOSION_MATH
    const int_t originBlockX = MathHelper::floor_double(explosionX);
    const int_t originBlockY = MathHelper::floor_double(explosionY);
    const int_t originBlockZ = MathHelper::floor_double(explosionZ);
    const float originFractionX = static_cast<float>(explosionX - static_cast<double>(originBlockX));
    const float originFractionY = static_cast<float>(explosionY - static_cast<double>(originBlockY));
    const float originFractionZ = static_cast<float>(explosionZ - static_cast<double>(originBlockZ));
#endif

    for (int_t j = 0; j < i; j++)
    {
        for (int_t l = 0; l < i; l++)
        {
            for (int_t j1 = 0; j1 < i; j1++)
            {
                if (j != 0 && j != i - 1 && l != 0 && l != i - 1 && j1 != 0 && j1 != i - 1)
                    continue;

#if PLATFORM_FLOAT_EXPLOSION_MATH
                float d  = ((float)j  / ((float)i - 1.0f)) * 2.0f - 1.0f;
                float d1 = ((float)l  / ((float)i - 1.0f)) * 2.0f - 1.0f;
                float d2 = ((float)j1 / ((float)i - 1.0f)) * 2.0f - 1.0f;
                const float d3 = MathHelper::sqrt_float(d * d + d1 * d1 + d2 * d2);
                const float invLength = 1.0f / d3;
                d  *= invLength;
                d1 *= invLength;
                d2 *= invLength;

                float f1 = explosionSize * (0.7f + worldObj->rand.nextFloat() * 0.6f);
                float rayX = originFractionX;
                float rayY = originFractionY;
                float rayZ = originFractionZ;
                const float f2 = 0.3f;

                while (f1 > 0.0f)
                {
                    const int_t j4 = JavaArithmetic::intAdd(originBlockX, MathHelper::floor_float(rayX));
                    const int_t k4 = JavaArithmetic::intAdd(originBlockY, MathHelper::floor_float(rayY));
                    const int_t l4 = JavaArithmetic::intAdd(originBlockZ, MathHelper::floor_float(rayZ));
                    const int_t i5 = worldObj->getBlockId(j4, k4, l4);

                    if (i5 > 0)
                        f1 -= (Block::blocksList[i5]->getExplosionResistance(exploder) + 0.3f) * f2;

                    if (f1 > 0.0f)
                        destroyedBlockPositions.add({j4, k4, l4});

                    rayX += d * f2;
                    rayY += d1 * f2;
                    rayZ += d2 * f2;
                    f1 -= f2 * 0.75f;
                }
#else
                double d  = ((float)j  / ((float)i - 1.0f)) * 2.0f - 1.0f;
                double d1 = ((float)l  / ((float)i - 1.0f)) * 2.0f - 1.0f;
                double d2 = ((float)j1 / ((float)i - 1.0f)) * 2.0f - 1.0f;
                double d3 = JavaMath::sqrt(d * d + d1 * d1 + d2 * d2);
                d  /= d3;
                d1 /= d3;
                d2 /= d3;

                float f1 = explosionSize * (0.7f + worldObj->rand.nextFloat() * 0.6f);
                double d5 = explosionX;
                double d7 = explosionY;
                double d9 = explosionZ;
                float f2 = 0.3f;

                while (true)
                {
                    if (f1 <= 0.0f)
                        break;

                    int_t j4 = MathHelper::floor_double(d5);
                    int_t k4 = MathHelper::floor_double(d7);
                    int_t l4 = MathHelper::floor_double(d9);
                    int_t i5 = worldObj->getBlockId(j4, k4, l4);

                    if (i5 > 0)
                        f1 -= (Block::blocksList[i5]->getExplosionResistance(exploder) + 0.3f) * f2;

                    if (f1 > 0.0f)
                        destroyedBlockPositions.add({j4, k4, l4});

                    d5 += d  * (double)f2;
                    d7 += d1 * (double)f2;
                    d9 += d2 * (double)f2;
                    f1 -= f2 * 0.75f;
                }
#endif
            }
        }
    }

    explosionSize *= 2.0f;
    int_t k  = MathHelper::floor_double(explosionX - (double)explosionSize - 1.0);
    int_t i1 = MathHelper::floor_double(explosionX + (double)explosionSize + 1.0);
    int_t k1 = MathHelper::floor_double(explosionY - (double)explosionSize - 1.0);
    int_t l1 = MathHelper::floor_double(explosionY + (double)explosionSize + 1.0);
    int_t i2 = MathHelper::floor_double(explosionZ - (double)explosionSize - 1.0);
    int_t j2 = MathHelper::floor_double(explosionZ + (double)explosionSize + 1.0);

    const auto& list = worldObj->getEntitiesWithinAABBExcludingEntity(
        exploder, AxisAlignedBB::getBoundingBoxFromPool((double)k, (double)k1, (double)i2, (double)i1, (double)l1, (double)j2));
    std::vector<Entity *> entities(list.begin(), list.end());

    Vec3D *vec3d = Vec3D::createVector(explosionX, explosionY, explosionZ);

    for (Entity *entity : entities)
    {
        double d4 = entity->getDistance(explosionX, explosionY, explosionZ) / (double)explosionSize;
        if (d4 <= 1.0)
        {
            double d6  = entity->posX - explosionX;
            double d8  = entity->posY - explosionY;
            double d10 = entity->posZ - explosionZ;
            double d11 = MathHelper::sqrt_double(d6 * d6 + d8 * d8 + d10 * d10);
            d6  /= d11;
            d8  /= d11;
            d10 /= d11;
            double d12 = worldObj->getBlockDensity(vec3d, entity->boundingBox);
            double d13 = (1.0 - d4) * d12;
            entity->attackEntityFrom(DamageSource::explosion,
                JavaArithmetic::doubleToInt(((d13 * d13 + d13) / 2.0) * 8.0 * (double)explosionSize + 1.0));
            double d14 = d13;
            entity->motionX += d6  * d14;
            entity->motionY += d8  * d14;
            entity->motionZ += d10 * d14;
        }
    }

    explosionSize = f;

    std::vector<ChunkPosTuple> blockList = destroyedBlockPositions.valuesInIterationOrder();
}

void Explosion::doExplosionB(bool flag)
{
    Random &random = worldObj->rand;
#if PLATFORM_FLOAT_EXPLOSION_MATH
    const int_t originBlockX = MathHelper::floor_double(explosionX);
    const int_t originBlockY = MathHelper::floor_double(explosionY);
    const int_t originBlockZ = MathHelper::floor_double(explosionZ);
    const float originFractionX = static_cast<float>(explosionX - static_cast<double>(originBlockX));
    const float originFractionY = static_cast<float>(explosionY - static_cast<double>(originBlockY));
    const float originFractionZ = static_cast<float>(explosionZ - static_cast<double>(originBlockZ));
#endif
    worldObj->playSoundEffect(explosionX, explosionY, explosionZ, "random.explode", 4.0f,
        (1.0f + random.nextFloatDifference() * 0.2f) * 0.7f);
    worldObj->spawnParticle("hugeexplosion", explosionX, explosionY, explosionZ, 0.0, 0.0, 0.0);

    std::vector<ChunkPosTuple> blockList = destroyedBlockPositions.valuesInIterationOrder();

    for (int_t i = (int_t)blockList.size() - 1; i >= 0; i--)
    {
        int_t j  = blockList[i].x;
        int_t k  = blockList[i].y;
        int_t l  = blockList[i].z;
        int_t i1 = worldObj->getBlockId(j, k, l);

        if (flag)
        {
#if PLATFORM_FLOAT_EXPLOSION_MATH
            const float d3 = static_cast<float>(JavaArithmetic::intSub(j, originBlockX)) - originFractionX + random.nextFloat();
            const float d4 = static_cast<float>(JavaArithmetic::intSub(k, originBlockY)) - originFractionY + random.nextFloat();
            const float d5 = static_cast<float>(JavaArithmetic::intSub(l, originBlockZ)) - originFractionZ + random.nextFloat();
            const float d6 = MathHelper::sqrt_float(d3 * d3 + d4 * d4 + d5 * d5);
            const float invDistance = 1.0f / d6;
            const float d7 = (0.5f / (d6 / explosionSize + 0.1f)) * (random.nextFloatProduct() + 0.3f);
            const float velocityX = d3 * invDistance * d7;
            const float velocityY = d4 * invDistance * d7;
            const float velocityZ = d5 * invDistance * d7;
            const double particleX = explosionX + static_cast<double>(d3);
            const double particleY = explosionY + static_cast<double>(d4);
            const double particleZ = explosionZ + static_cast<double>(d5);
            worldObj->spawnParticle("explode", (particleX + explosionX) / 2.0, (particleY + explosionY) / 2.0, (particleZ + explosionZ) / 2.0, velocityX, velocityY, velocityZ);
            worldObj->spawnParticle("smoke", particleX, particleY, particleZ, velocityX, velocityY, velocityZ);
#else
            double d  = (float)j + random.nextFloat();
            double d1 = (float)k + random.nextFloat();
            double d2 = (float)l + random.nextFloat();
            double d3 = d - explosionX;
            double d4 = d1 - explosionY;
            double d5 = d2 - explosionZ;
            double d6 = MathHelper::sqrt_double(d3 * d3 + d4 * d4 + d5 * d5);
            d3 /= d6;
            d4 /= d6;
            d5 /= d6;
            double d7 = 0.5 / (d6 / (double)explosionSize + 0.1);
            d7 *= random.nextFloatProduct() + 0.3f;
            d3 *= d7;
            d4 *= d7;
            d5 *= d7;
            worldObj->spawnParticle("explode", (d + explosionX * 1.0) / 2.0, (d1 + explosionY * 1.0) / 2.0, (d2 + explosionZ * 1.0) / 2.0, d3, d4, d5);
            worldObj->spawnParticle("smoke", d, d1, d2, d3, d4, d5);
#endif
        }

        if (i1 > 0)
        {
            Block::blocksList[i1]->dropBlockAsItemWithChance(worldObj, j, k, l, worldObj->getBlockMetadata(j, k, l), 0.3f);
            worldObj->setBlockWithNotify(j, k, l, 0);
            Block::blocksList[i1]->onBlockDestroyedByExplosion(worldObj, j, k, l);
        }
    }

    if (isFlaming)
    {
        for (int_t i = (int_t)blockList.size() - 1; i >= 0; i--)
        {
            const int_t x = blockList[i].x;
            const int_t y = blockList[i].y;
            const int_t z = blockList[i].z;
            const int_t blockId = worldObj->getBlockId(x, y, z);
            const int_t belowBlockId = worldObj->getBlockId(x, y - 1, z);

            if (blockId == 0 && Block::opaqueCubeLookup[belowBlockId] && explosionRNG.nextInt(3) == 0)
                worldObj->setBlockWithNotify(x, y, z, Block::fire->blockID);
        }
    }
}
