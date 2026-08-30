#include "ItemBucket.h"
#include "java/Math.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "Vec3D.h"
#include "MathHelper.h"
#include "World.h"
#include "MovingObjectPosition.h"
#include "EnumMovingObjectType.h"
#include "Material.h"
#include "WorldProvider.h"
#include "Block.h"
#include "EntityCow.h"
#include <memory>

ItemBucket::ItemBucket(int i, int j)
    : Item(i), isFull(j) {
    maxStackSize = 1;
}

ItemStack* ItemBucket::onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
    float f = 1.0f;
    float f1 = entityplayer->prevRotationPitch + (entityplayer->rotationPitch - entityplayer->prevRotationPitch) * f;
    float f2 = entityplayer->prevRotationYaw + (entityplayer->rotationYaw - entityplayer->prevRotationYaw) * f;
    double d = entityplayer->prevPosX + (entityplayer->posX - entityplayer->prevPosX) * (double)f;
    double d1 = (entityplayer->prevPosY + (entityplayer->posY - entityplayer->prevPosY) * (double)f + 1.6200000000000001) - (double)entityplayer->yOffset;
    double d2 = entityplayer->prevPosZ + (entityplayer->posZ - entityplayer->prevPosZ) * (double)f;
    Vec3D* vec3d = Vec3D::createVector(d, d1, d2);
    float f3 = MathHelper::cos(-f2 * 0.017453292f - 3.1415927f);
    float f4 = MathHelper::sin(-f2 * 0.017453292f - 3.1415927f);
    float f5 = -MathHelper::cos(-f1 * 0.017453292f);
    float f6 = MathHelper::sin(-f1 * 0.017453292f);
    float f7 = f4 * f5;
    float f8 = f6;
    float f9 = f3 * f5;
    double d3 = 5.0;
    Vec3D* vec3d1 = vec3d->addVector((double)f7 * d3, (double)f8 * d3, (double)f9 * d3);
    std::unique_ptr<MovingObjectPosition> movingobjectposition(world->rayTraceBlocks_do(vec3d, vec3d1, isFull == 0));
    if (movingobjectposition == nullptr) {
        return itemstack;
    }
    if (movingobjectposition->typeOfHit == EnumMovingObjectType::TILE) {
        int i = movingobjectposition->blockX;
        int j = movingobjectposition->blockY;
        int k = movingobjectposition->blockZ;
        if (!world->canMineBlock(entityplayer, i, j, k)) {
            return itemstack;
        }
        if (isFull == 0) {
            if (!entityplayer->canPlayerEdit(i, j, k)) {
                return itemstack;
            }
            if (world->getBlockMaterial(i, j, k) == Material::water && world->getBlockMetadata(i, j, k) == 0) {
                world->setBlockWithNotify(i, j, k, 0);
                if (entityplayer->capabilities.isCreativeMode) {
                    return itemstack;
                }
                return new ItemStack(Item::bucketWater);
            }
            if (world->getBlockMaterial(i, j, k) == Material::lava && world->getBlockMetadata(i, j, k) == 0) {
                world->setBlockWithNotify(i, j, k, 0);
                if (entityplayer->capabilities.isCreativeMode) {
                    return itemstack;
                }
                return new ItemStack(Item::bucketLava);
            }
        } else {
            if (isFull < 0) {
                return new ItemStack(Item::bucketEmpty);
            }
            if (movingobjectposition->sideHit == 0) j--;
            if (movingobjectposition->sideHit == 1) j++;
            if (movingobjectposition->sideHit == 2) k--;
            if (movingobjectposition->sideHit == 3) k++;
            if (movingobjectposition->sideHit == 4) i--;
            if (movingobjectposition->sideHit == 5) i++;
            if (!entityplayer->canPlayerEdit(i, j, k)) {
                return itemstack;
            }
            if (world->isAirBlock(i, j, k) || !world->getBlockMaterial(i, j, k)->isSolid()) {
                if (world->worldProvider->isHellWorld && isFull == Block::waterMoving->blockID) {
                    world->playSoundEffect(d + 0.5, d1 + 0.5, d2 + 0.5, "random.fizz", 0.5f, 2.6f + world->rand.nextFloatDifference() * 0.8f);
                    for (int l = 0; l < 8; l++) {
                        const double particleX = (double)i + Math::random();
                        const double particleY = (double)j + Math::random();
                        const double particleZ = (double)k + Math::random();
                        world->spawnParticle("largesmoke", particleX, particleY, particleZ, 0.0, 0.0, 0.0);
                    }
                } else {
                    world->setBlockAndMetadataWithNotify(i, j, k, isFull, 0);
                }
                if (entityplayer->capabilities.isCreativeMode) {
                    return itemstack;
                }
                return new ItemStack(Item::bucketEmpty);
            }
        }
    // entityHit is null on a TILE hit, and rayTraceBlocks_do only ever produces
    // TILE hits, so this branch is unreachable today — but it is the same
    // null-unsafe `instanceof` translation that crashed EntityCreeper::onDeath,
    // so guard it rather than leave the trap armed.
    } else if (isFull == 0 && movingobjectposition->entityHit != nullptr
               && dynamic_cast<EntityCow *>(movingobjectposition->entityHit) != nullptr) {
        return new ItemStack(Item::bucketMilk);
    }
    return itemstack;
}
