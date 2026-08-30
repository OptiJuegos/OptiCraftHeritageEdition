#include "BlockEndPortal.h"

#include "Entity.h"
#include "EntityPlayer.h"
#include "IBlockAccess.h"
#include "TileEntityEndPortal.h"
#include "World.h"
#include "WorldProvider.h"
#include "java/Random.h"

bool BlockEndPortal::bossDefeated = false;

BlockEndPortal::BlockEndPortal(int_t id, Material *material)
    : BlockContainer(id, 0, material)
{
    setLightValue(1.0f);
}

TileEntity *BlockEndPortal::getBlockEntity()
{
    return new TileEntityEndPortal();
}

void BlockEndPortal::setBlockBoundsBasedOnState(IBlockAccess *, int_t, int_t, int_t)
{
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f / 16.0f, 1.0f);
}

bool BlockEndPortal::shouldSideBeRendered(IBlockAccess *access, int_t x, int_t y, int_t z, int_t side)
{
    return side == 0 && BlockContainer::shouldSideBeRendered(access, x, y, z, side);
}

void BlockEndPortal::getCollidingBoundingBoxes(World *, int_t, int_t, int_t, AxisAlignedBB *,
                                               std::vector<AxisAlignedBB *> &)
{
}

bool BlockEndPortal::isOpaqueCube()
{
    return false;
}

bool BlockEndPortal::renderAsNormalBlock()
{
    return false;
}

int_t BlockEndPortal::quantityDropped(Random &)
{
    return 0;
}

void BlockEndPortal::onEntityCollidedWithBlock(World *world, int_t, int_t, int_t, Entity *entity)
{
    if (entity == nullptr || entity->ridingEntity != nullptr || entity->riddenByEntity != nullptr || world->multiplayerWorld)
        return;
    EntityPlayer *player = dynamic_cast<EntityPlayer *>(entity);
    if (player != nullptr)
        player->travelToTheEnd(1);
}

void BlockEndPortal::randomDisplayTick(World *world, int_t x, int_t y, int_t z, Random &random)
{
    const double particleX = static_cast<double>(static_cast<float>(x) + random.nextFloat());
    const double particleZ = static_cast<double>(static_cast<float>(z) + random.nextFloat());
    world->spawnParticle("smoke", particleX, static_cast<double>(static_cast<float>(y) + 0.8f), particleZ,
                         0.0, 0.0, 0.0);
}

int_t BlockEndPortal::getRenderType()
{
    return -1;
}

void BlockEndPortal::onBlockAdded(World *world, int_t x, int_t y, int_t z)
{
    if (!bossDefeated && world->worldProvider != nullptr && world->worldProvider->worldType != 0)
        world->setBlockWithNotify(x, y, z, 0);
}
