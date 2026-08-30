#pragma once

#include "Container.h"

class InventoryPlayer;
class InventoryCrafting;
class IInventory;
class ItemStack;
class EntityPlayer;

// net.minecraft.src.ContainerPlayer
class ContainerPlayer : public Container
{
public:
	ContainerPlayer(InventoryPlayer *inventoryplayer);
	ContainerPlayer(InventoryPlayer *inventoryplayer, bool_t flag);
	~ContainerPlayer() override;

	void onCraftMatrixChanged(IInventory *iinventory) override;
	void onCraftGuiClosed(EntityPlayer *entityplayer) override;
	bool isUsableByPlayer(EntityPlayer *entityplayer) override;
	ItemStack *getStackInSlot(int_t i) override;

	InventoryCrafting *craftMatrix;
	IInventory *craftResult;
	bool isSinglePlayer;
};
