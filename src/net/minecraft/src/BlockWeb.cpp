#include "BlockWeb.h"
#include "Material.h"
#include "Entity.h"
#include "Item.h"

BlockWeb::BlockWeb(int_t i, int_t j) : Block(i, j, Material::web)
{
}

void BlockWeb::onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity)
{
    entity->isInWeb = true;
}

bool BlockWeb::isOpaqueCube() { return false; }

AxisAlignedBB *BlockWeb::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    return nullptr;
}

int_t BlockWeb::getRenderType() { return 1; }
bool BlockWeb::renderAsNormalBlock() { return false; }

int_t BlockWeb::idDropped(int_t i, Random &random)
{
    return Item::silk->shiftedIndex;
}
