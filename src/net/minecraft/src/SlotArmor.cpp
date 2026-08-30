#include "SlotArmor.h"

#include "ContainerPlayer.h"
#include "IInventory.h"
#include "ItemStack.h"
#include "ItemArmor.h"
#include "Item.h"
#include "Block.h"

SlotArmor::SlotArmor(ContainerPlayer *containerplayer, IInventory *iinventory, int_t i, int_t j, int_t k, int_t l) :
	Slot(iinventory, i, j, k),
	inventory(containerplayer),
	armorType(l)
{
}

int_t SlotArmor::getSlotStackLimit()
{
	return 1;
}

bool SlotArmor::isItemValid(ItemStack *itemstack)
{
	if(itemstack == nullptr || !itemstack->isValid())
	{
		return false;
	}
	Item *item = itemstack->getItem();
	ItemArmor *armor = dynamic_cast<ItemArmor *>(item);
	if(armor != nullptr)
	{
		return armor->armorType == armorType;
	}
	if(item != nullptr && item->shiftedIndex == Block::pumpkin->blockID)
	{
		return armorType == 0;
	} else
	{
		return false;
	}
}

int_t SlotArmor::getBackgroundIconIndex()
{
	return 15 + armorType * 16;
}
