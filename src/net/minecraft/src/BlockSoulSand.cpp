#include "BlockSoulSand.h"
#include "Material.h"
#include "AxisAlignedBB.h"
#include "Entity.h"

BlockSoulSand::BlockSoulSand(int_t i, int_t j) : Block(i, j, Material::sand)
{
}

AxisAlignedBB *BlockSoulSand::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    float f = 0.125f;
    return AxisAlignedBB::getBoundingBoxFromPool(i, j, k, i + 1, (float)(j + 1) - f, k + 1);
}

void BlockSoulSand::onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity)
{
    entity->motionX *= 0.40000000000000002;
    entity->motionZ *= 0.40000000000000002;
}
