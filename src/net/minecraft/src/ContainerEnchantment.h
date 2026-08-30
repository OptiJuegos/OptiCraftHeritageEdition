#pragma once

#include "Container.h"
#include "java/Random.h"

class EntityPlayer;
class IInventory;
class InventoryPlayer;
class ItemStack;
class World;

// net.minecraft.src.ContainerEnchantment
class ContainerEnchantment : public Container
{
public:
	ContainerEnchantment(InventoryPlayer *inventory, World *world, int_t x, int_t y, int_t z);
	~ContainerEnchantment() override;

	void updateCraftingResults() override;
	void updateProgressBar(int_t id, int_t value) override;
	void onCraftMatrixChanged(IInventory *inventory) override;
	bool enchantItem(EntityPlayer *player, int_t option) override;
	void onCraftGuiClosed(EntityPlayer *player) override;
	bool isUsableByPlayer(EntityPlayer *player) override;
	ItemStack *getStackInSlot(int_t slot) override;

	IInventory *tableInventory;
	long_t nameSeed;
	int_t enchantLevels[3];

private:
	World *worldPointer;
	int_t posX;
	int_t posY;
	int_t posZ;
	Random rand;
};
