#include "ItemSoup.h"

#include "Item.h"
#include "ItemStack.h"

ItemSoup::ItemSoup(int id, int heal) :
	ItemFood(id, heal, false)
{
	maxStackSize = 1;
}

ItemStack *ItemSoup::onFoodEaten(ItemStack *itemstack, World *world, EntityPlayer *entityplayer)
{
	ItemFood::onFoodEaten(itemstack, world, entityplayer);
	return new ItemStack(Item::bowlEmpty);
}
