#include "ItemLilyPad.h"

#include <memory>

#include "Block.h"
#include "BlockLilyPad.h"
#include "EntityPlayer.h"
#include "ItemStack.h"
#include "Material.h"
#include "MovingObjectPosition.h"
#include "PlayerCapabilities.h"
#include "World.h"

ItemLilyPad::ItemLilyPad(int id)
    : ItemColored(id, false)
{
}

ItemStack *ItemLilyPad::onItemRightClick(ItemStack *itemstack, World *world, EntityPlayer *player)
{
    if (itemstack == nullptr || world == nullptr || player == nullptr)
        return itemstack;

    std::unique_ptr<MovingObjectPosition> hit(getMovingObjectPositionFromPlayer(world, player, true));
    if (hit == nullptr || hit->typeOfHit != EnumMovingObjectType::TILE)
        return itemstack;

    int x = hit->blockX;
    int y = hit->blockY;
    int z = hit->blockZ;
    if (!world->canMineBlock(player, x, y, z) || !player->canPlayerEdit(x, y, z))
        return itemstack;

    if (Block::waterlily != nullptr && world->getBlockMaterial(x, y, z) == Material::water &&
        world->getBlockMetadata(x, y, z) == 0 && world->isAirBlock(x, y + 1, z))
    {
        world->setBlockWithNotify(x, y + 1, z, Block::waterlily->blockID);
        if (!player->capabilities.isCreativeMode)
            --itemstack->stackSize;
    }
    return itemstack;
}

int ItemLilyPad::getColorFromDamage(int damage, int)
{
    return Block::waterlily != nullptr ? Block::waterlily->getRenderColor(damage) : 0xffffff;
}
