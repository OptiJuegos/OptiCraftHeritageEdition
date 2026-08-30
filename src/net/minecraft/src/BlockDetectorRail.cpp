#include "BlockDetectorRail.h"
#include "World.h"
#include "IBlockAccess.h"
#include "AxisAlignedBB.h"
#include "EntityMinecart.h"
#include "Entity.h"
#include <vector>

BlockDetectorRail::BlockDetectorRail(int_t i, int_t j) : BlockRail(i, j, true)
{
    setTickOnLoad(true);
}

int_t BlockDetectorRail::tickRate() { return 20; }
bool  BlockDetectorRail::canProvidePower() { return true; }

void BlockDetectorRail::onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity)
{
    if (world->multiplayerWorld) return;
    int_t l = world->getBlockMetadata(i, j, k);
    if ((l & 8) != 0) return;
    setStateIfMinecartInteractsWithRail(world, i, j, k, l);
}

void BlockDetectorRail::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    if (world->multiplayerWorld) return;
    int_t l = world->getBlockMetadata(i, j, k);
    if ((l & 8) == 0) return;
    setStateIfMinecartInteractsWithRail(world, i, j, k, l);
}

bool BlockDetectorRail::isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    return (iblockaccess->getBlockMetadata(i, j, k) & 8) != 0;
}

bool BlockDetectorRail::isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l)
{
    if ((world->getBlockMetadata(i, j, k) & 8) == 0) return false;
    return l == 1;
}

void BlockDetectorRail::setStateIfMinecartInteractsWithRail(World *world, int_t i, int_t j, int_t k, int_t l)
{
    bool flag  = (l & 8) != 0;
    bool flag1 = false;
    float f    = 0.125f;
    const auto& list = world->getEntitiesWithinAABB(typeid(EntityMinecart),
        AxisAlignedBB::getBoundingBoxFromPool((float)i + f, j, (float)k + f,
                                              (float)(i + 1) - f, (double)j + 0.25, (float)(k + 1) - f));
    if ((int_t)list.size() > 0) flag1 = true;
    if (flag1 && !flag)
    {
        world->setBlockMetadataWithNotify(i, j, k, l | 8);
        world->notifyBlocksOfNeighborChange(i, j, k, blockID);
        world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
        world->markBlocksDirty(i, j, k, i, j, k);
    }
    if (!flag1 && flag)
    {
        world->setBlockMetadataWithNotify(i, j, k, l & 7);
        world->notifyBlocksOfNeighborChange(i, j, k, blockID);
        world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
        world->markBlocksDirty(i, j, k, i, j, k);
    }
    if (flag1) world->scheduleBlockUpdate(i, j, k, blockID, tickRate());
}
