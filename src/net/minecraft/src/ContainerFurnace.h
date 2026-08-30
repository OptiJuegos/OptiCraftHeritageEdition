#pragma once

#include "Container.h"

class InventoryPlayer;
class TileEntityFurnace;
class ItemStack;
class EntityPlayer;

// net.minecraft.src.ContainerFurnace
class ContainerFurnace : public Container
{
public:
	ContainerFurnace(InventoryPlayer *inventoryplayer, TileEntityFurnace *tileentityfurnace);
	~ContainerFurnace() override;

	void updateCraftingResults() override;
	void updateProgressBar(int_t id, int_t value) override;
	bool isUsableByPlayer(EntityPlayer *entityplayer) override;
	ItemStack *getStackInSlot(int_t i) override;

private:
	TileEntityFurnace *furnace;
	bool ownsFurnace = false;
	int_t cookTime = 0;
	int_t burnTime = 0;
	int_t itemBurnTime = 0;
};
