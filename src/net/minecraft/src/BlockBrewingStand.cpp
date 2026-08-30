#include "BlockBrewingStand.h"

#include "AxisAlignedBB.h"
#include "EntityItem.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "Material.h"
#include "TileEntityBrewingStand.h"
#include "World.h"
#include "java/Random.h"

BlockBrewingStand::BlockBrewingStand(int_t id)
    : BlockContainer(id, Material::iron)
{
    blockIndexInTexture = 157;
}

bool BlockBrewingStand::isOpaqueCube()
{
    return false;
}

int_t BlockBrewingStand::getRenderType()
{
    return 25;
}

TileEntity *BlockBrewingStand::getBlockEntity()
{
    return new TileEntityBrewingStand();
}

bool BlockBrewingStand::renderAsNormalBlock()
{
    return false;
}

void BlockBrewingStand::getCollidingBoundingBoxes(World *world, int_t x, int_t y, int_t z, AxisAlignedBB *mask,
                                                  std::vector<AxisAlignedBB *> &boxes)
{
    setBlockBounds(7.0f / 16.0f, 0.0f, 7.0f / 16.0f, 9.0f / 16.0f, 14.0f / 16.0f, 9.0f / 16.0f);
    BlockContainer::getCollidingBoundingBoxes(world, x, y, z, mask, boxes);
    setBlockBoundsForItemRender();
    BlockContainer::getCollidingBoundingBoxes(world, x, y, z, mask, boxes);
}

void BlockBrewingStand::setBlockBoundsForItemRender()
{
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 2.0f / 16.0f, 1.0f);
}

bool BlockBrewingStand::blockActivated(World *world, int_t x, int_t y, int_t z, EntityPlayer *player)
{
    if (world->multiplayerWorld)
        return true;
    TileEntityBrewingStand *stand = dynamic_cast<TileEntityBrewingStand *>(world->getBlockTileEntity(x, y, z));
    if (stand != nullptr)
        player->displayGUIBrewingStand(stand);
    return true;
}

void BlockBrewingStand::randomDisplayTick(World *world, int_t x, int_t y, int_t z, Random &random)
{
    const double px = static_cast<double>(static_cast<float>(x) + 0.4f + random.nextFloat() * 0.2f);
    const double py = static_cast<double>(static_cast<float>(y) + 0.7f + random.nextFloat() * 0.3f);
    const double pz = static_cast<double>(static_cast<float>(z) + 0.4f + random.nextFloat() * 0.2f);
    world->spawnParticle("smoke", px, py, pz, 0.0, 0.0, 0.0);
}

void BlockBrewingStand::onBlockRemoval(World *world, int_t x, int_t y, int_t z)
{
    TileEntityBrewingStand *stand = dynamic_cast<TileEntityBrewingStand *>(world->getBlockTileEntity(x, y, z));
    if (stand != nullptr)
    {
        for (int slot = 0; slot < stand->getSizeInventory(); ++slot)
        {
            ItemStack *stack = stand->getStackInSlot(slot);
            if (stack == nullptr)
                continue;
            const float ox = random.nextFloat() * 0.8f + 0.1f;
            const float oy = random.nextFloat() * 0.8f + 0.1f;
            const float oz = random.nextFloat() * 0.8f + 0.1f;
            while (stack->stackSize > 0)
            {
                int amount = random.nextInt(21) + 10;
                if (amount > stack->stackSize)
                    amount = stack->stackSize;
                stack->stackSize -= amount;
                EntityItem *item = new EntityItem(world,
                                                  static_cast<double>(x) + ox,
                                                  static_cast<double>(y) + oy,
                                                  static_cast<double>(z) + oz,
                                                  new ItemStack(stack->itemID, amount, stack->getItemDamage()));
                const float spread = 0.05f;
                item->motionX = static_cast<float>(random.nextGaussian()) * spread;
                item->motionY = static_cast<float>(random.nextGaussian()) * spread + 0.2f;
                item->motionZ = static_cast<float>(random.nextGaussian()) * spread;
                if (!world->spawnEntityInWorld(item))
                    delete item;
            }
        }
    }
    BlockContainer::onBlockRemoval(world, x, y, z);
}

int_t BlockBrewingStand::idDropped(int_t, Random &)
{
    return Item::brewingStand != nullptr ? Item::brewingStand->shiftedIndex : 0;
}
