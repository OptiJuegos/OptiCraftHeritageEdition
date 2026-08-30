#pragma once

#include "InventoryBasic.h"

class ContainerEnchantment;

// net.minecraft.src.SlotEnchantmentTable
class SlotEnchantmentTable : public InventoryBasic
{
public:
	SlotEnchantmentTable(ContainerEnchantment *container, const std::string &name, int_t size);
	int_t getInventoryStackLimit() override;
	void onInventoryChanged() override;

private:
	ContainerEnchantment *container;
};
