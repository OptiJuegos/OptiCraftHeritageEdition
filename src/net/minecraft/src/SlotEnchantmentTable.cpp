#include "SlotEnchantmentTable.h"

#include "ContainerEnchantment.h"

SlotEnchantmentTable::SlotEnchantmentTable(ContainerEnchantment *owner, const std::string &name, int_t size)
	: InventoryBasic(name, size), container(owner)
{
}

int_t SlotEnchantmentTable::getInventoryStackLimit()
{
	return 1;
}

void SlotEnchantmentTable::onInventoryChanged()
{
	InventoryBasic::onInventoryChanged();
	if (container != nullptr)
		container->onCraftMatrixChanged(this);
}
