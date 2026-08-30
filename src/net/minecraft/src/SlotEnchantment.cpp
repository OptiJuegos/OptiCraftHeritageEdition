#include "SlotEnchantment.h"

SlotEnchantment::SlotEnchantment(ContainerEnchantment *owner, IInventory *inventory, int_t slot, int_t x, int_t y)
	: Slot(inventory, slot, x, y), container(owner)
{
}

bool SlotEnchantment::isItemValid(ItemStack *stack)
{
	(void)stack;
	return true;
}
