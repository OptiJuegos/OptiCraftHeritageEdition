#include "ItemBoat.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "Vec3D.h"
#include "MathHelper.h"
#include "World.h"
#include "MovingObjectPosition.h"
#include "EnumMovingObjectType.h"
#include "Block.h"
#include "EntityBoat.h"
#include "AxisAlignedBB.h"
#include "Entity.h"
#include <memory>

ItemBoat::ItemBoat(int i)
    : Item(i) {
    maxStackSize = 1;
}

ItemStack* ItemBoat::onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
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
    std::unique_ptr<MovingObjectPosition> movingobjectposition(world->rayTraceBlocks_do(vec3d, vec3d1, true));
    if (movingobjectposition == nullptr) {
        return itemstack;
    }

    Vec3D* look = entityplayer->getLook(f);
    const float collisionExpand = 1.0f;
    const auto& entities = world->getEntitiesWithinAABBExcludingEntity(
        entityplayer,
        entityplayer->boundingBox
            ->addCoord(look->xCoord * d3, look->yCoord * d3, look->zCoord * d3)
            ->expand(collisionExpand, collisionExpand, collisionExpand));

    for (Entity* entity : entities) {
        if (entity == nullptr || !entity->canBeCollidedWith()) {
            continue;
        }

        const float collisionBorder = entity->getCollisionBorderSize();
        AxisAlignedBB* collisionBox = entity->boundingBox->expand(
            collisionBorder, collisionBorder, collisionBorder);
        if (collisionBox->isVecInside(vec3d)) {
            return itemstack;
        }
    }

    if (movingobjectposition->typeOfHit == EnumMovingObjectType::TILE) {
        int i = movingobjectposition->blockX;
        int j = movingobjectposition->blockY;
        int k = movingobjectposition->blockZ;
        if (!world->multiplayerWorld) {
            if (world->getBlockId(i, j, k) == Block::snow->blockID) {
                j--;
            }
            EntityBoat *boat = new EntityBoat(world, (float)i + 0.5f, (float)j + 1.0f, (float)k + 0.5f);
            if (!world->entityJoinedWorld(boat))
                delete boat;
        }
        if (!entityplayer->capabilities.isCreativeMode) {
            itemstack->stackSize--;
        }
    }
    return itemstack;
}
