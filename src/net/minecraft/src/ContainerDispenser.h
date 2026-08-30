#pragma once

#include "Container.h"

class IInventory;
class TileEntityDispenser;
class EntityPlayer;

// net.minecraft.src.ContainerDispenser
class ContainerDispenser : public Container
{
public:
	ContainerDispenser(IInventory *iinventory, TileEntityDispenser *tileentitydispenser);
	~ContainerDispenser() override;

	bool isUsableByPlayer(EntityPlayer *entityplayer) override;
	ItemStack *getStackInSlot(int_t slotIndex) override;

private:
	TileEntityDispenser *dispenser;
	bool ownsDispenser = false;
};
