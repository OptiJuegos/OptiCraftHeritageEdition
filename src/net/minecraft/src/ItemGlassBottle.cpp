#include "ItemGlassBottle.h"

#include <memory>

#include "EntityPlayer.h"
#include "EnumMovingObjectType.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "Material.h"
#include "MovingObjectPosition.h"
#include "World.h"

ItemGlassBottle::ItemGlassBottle(int id) : Item(id)
{
}

ItemStack *ItemGlassBottle::onItemRightClick(ItemStack *itemstack, World *world, EntityPlayer *player)
{
    std::unique_ptr<MovingObjectPosition> hit(getMovingObjectPositionFromPlayer(world, player, true));
    if (hit == nullptr || hit->typeOfHit != EnumMovingObjectType::TILE)
        return itemstack;

    const int x = hit->blockX;
    const int y = hit->blockY;
    const int z = hit->blockZ;
    if (!world->canMineBlock(player, x, y, z) || !player->canPlayerEdit(x, y, z))
        return itemstack;

    if (world->getBlockMaterial(x, y, z) != Material::water)
        return itemstack;

    --itemstack->stackSize;
    if (itemstack->stackSize <= 0)
        return new ItemStack(Item::potion);

    std::unique_ptr<ItemStack> potionStack(new ItemStack(Item::potion));
    if (!player->inventory->addItemStackToInventory(potionStack.get()))
    {
        player->dropPlayerItem(potionStack.release());
    }
    return itemstack;
}
