#pragma once

#include "Container.h"

class InventoryPlayer;
class InventoryCrafting;
class IInventory;
class World;
class ItemStack;
class EntityPlayer;

// net.minecraft.src.ContainerWorkbench
class ContainerWorkbench : public Container
{
public:
	ContainerWorkbench(InventoryPlayer *inventoryplayer, World *world, int_t i, int_t j, int_t k);
	~ContainerWorkbench() override;

	void onCraftMatrixChanged(IInventory *iinventory) override;
	void onCraftGuiClosed(EntityPlayer *entityplayer) override;
	bool isUsableByPlayer(EntityPlayer *entityplayer) override;
	ItemStack *getStackInSlot(int_t i) override;

	InventoryCrafting *craftMatrix;
	IInventory *craftResult;

private:
	World *world;
	int_t posX;
	int_t posY;
	int_t posZ;
};
