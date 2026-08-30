#pragma once

#include "Container.h"

class IInventory;
class ItemStack;
class EntityPlayer;

// net.minecraft.src.ContainerChest
class ContainerChest : public Container
{
public:
	ContainerChest(IInventory *iinventory, IInventory *iinventory1, bool ownsLowerChestInventory = false);
	~ContainerChest() override;

	bool isUsableByPlayer(EntityPlayer *entityplayer) override;
	void onCraftGuiClosed(EntityPlayer *entityplayer) override;
	ItemStack *getStackInSlot(int_t i) override;

private:
	IInventory *lowerChestInventory;
	bool ownsLowerChestInventory;
	int_t numRows;
};
