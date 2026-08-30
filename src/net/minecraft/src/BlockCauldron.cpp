#include "BlockCauldron.h"

#include <memory>

#include "EntityItem.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "Material.h"
#include "World.h"
#include "java/Random.h"

BlockCauldron::BlockCauldron(int_t id)
    : Block(id, Material::iron)
{
    blockIndexInTexture = 154;
}

int_t BlockCauldron::getBlockTextureFromSideAndMetadata(int_t side, int_t)
{
    return side == 1 ? 138 : (side == 0 ? 155 : 154);
}

void BlockCauldron::getCollidingBoundingBoxes(World *world, int_t x, int_t y, int_t z, AxisAlignedBB *mask,
                                              std::vector<AxisAlignedBB *> &boxes)
{
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 5.0f / 16.0f, 1.0f);
    Block::getCollidingBoundingBoxes(world, x, y, z, mask, boxes);
    const float edge = 2.0f / 16.0f;
    setBlockBounds(0.0f, 0.0f, 0.0f, edge, 1.0f, 1.0f);
    Block::getCollidingBoundingBoxes(world, x, y, z, mask, boxes);
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, edge);
    Block::getCollidingBoundingBoxes(world, x, y, z, mask, boxes);
    setBlockBounds(1.0f - edge, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    Block::getCollidingBoundingBoxes(world, x, y, z, mask, boxes);
    setBlockBounds(0.0f, 0.0f, 1.0f - edge, 1.0f, 1.0f, 1.0f);
    Block::getCollidingBoundingBoxes(world, x, y, z, mask, boxes);
    setBlockBoundsForItemRender();
}

void BlockCauldron::setBlockBoundsForItemRender()
{
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

bool BlockCauldron::isOpaqueCube()
{
    return false;
}

int_t BlockCauldron::getRenderType()
{
    return 24;
}

bool BlockCauldron::renderAsNormalBlock()
{
    return false;
}

bool BlockCauldron::blockActivated(World *world, int_t x, int_t y, int_t z, EntityPlayer *player)
{
    if (world->multiplayerWorld)
        return true;

    ItemStack *held = player->inventory->getCurrentItem();
    if (held == nullptr)
        return true;

    const int level = world->getBlockMetadata(x, y, z);
    if (Item::bucketWater != nullptr && held->itemID == Item::bucketWater->shiftedIndex)
    {
        if (level < 3)
        {
            if (!player->capabilities.isCreativeMode)
            {
                player->inventory->setInventorySlotContents(player->inventory->currentItem, new ItemStack(Item::bucketEmpty));
            }
            world->setBlockMetadataWithNotify(x, y, z, 3);
        }
        return true;
    }

    if (Item::glassBottle != nullptr && Item::potion != nullptr && held->itemID == Item::glassBottle->shiftedIndex && level > 0)
    {
        std::unique_ptr<ItemStack> potion(new ItemStack(Item::potion, 1, 0));
        if (!player->inventory->addItemStackToInventory(potion.get()))
        {
            EntityItem *item = new EntityItem(world, static_cast<double>(x) + 0.5, static_cast<double>(y) + 1.5,
                                              static_cast<double>(z) + 0.5, potion.release());
            if (!world->spawnEntityInWorld(item))
                delete item;
        }
        --held->stackSize;
        if (held->stackSize <= 0)
        {
            player->inventory->setInventorySlotContents(player->inventory->currentItem, nullptr);
        }
        world->setBlockMetadataWithNotify(x, y, z, level - 1);
        return true;
    }

    return true;
}

int_t BlockCauldron::idDropped(int_t, Random &)
{
    return Item::cauldron != nullptr ? Item::cauldron->shiftedIndex : 0;
}
