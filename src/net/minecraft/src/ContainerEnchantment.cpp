#include "ContainerEnchantment.h"

#include "Block.h"
#include "EnchantmentData.h"
#include "EnchantmentHelper.h"
#include "EntityPlayer.h"
#include "ICrafting.h"
#include "IInventory.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "Slot.h"
#include "SlotEnchantment.h"
#include "SlotEnchantmentTable.h"
#include "World.h"

ContainerEnchantment::ContainerEnchantment(InventoryPlayer *inventory, World *world, int_t x, int_t y, int_t z)
	: tableInventory(new SlotEnchantmentTable(this, "Enchant", 1)), nameSeed(0), worldPointer(world), posX(x), posY(y), posZ(z)
{
	enchantLevels[0] = 0;
	enchantLevels[1] = 0;
	enchantLevels[2] = 0;
	addSlot(new SlotEnchantment(this, tableInventory, 0, 25, 47));

	for (int_t row = 0; row < 3; ++row)
	{
		for (int_t column = 0; column < 9; ++column)
			addSlot(new Slot(inventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));
	}
	for (int_t column = 0; column < 9; ++column)
		addSlot(new Slot(inventory, column, 8 + column * 18, 142));
}

ContainerEnchantment::~ContainerEnchantment()
{
	delete tableInventory;
	tableInventory = nullptr;
}

void ContainerEnchantment::updateCraftingResults()
{
	Container::updateCraftingResults();
	for (ICrafting *crafter : crafters)
	{
		if (crafter == nullptr)
			continue;
		crafter->sendProgressBarUpdate(this, 0, enchantLevels[0]);
		crafter->sendProgressBarUpdate(this, 1, enchantLevels[1]);
		crafter->sendProgressBarUpdate(this, 2, enchantLevels[2]);
	}
}

void ContainerEnchantment::updateProgressBar(int_t id, int_t value)
{
	if (id >= 0 && id <= 2)
		enchantLevels[id] = value;
	else
		Container::updateProgressBar(id, value);
}

void ContainerEnchantment::onCraftMatrixChanged(IInventory *inventory)
{
	if (inventory != tableInventory || worldPointer == nullptr)
		return;

	ItemStack *stack = inventory->getStackInSlot(0);
	if (stack != nullptr && stack->isItemEnchantable())
	{
		nameSeed = rand.nextLong();
		if (!worldPointer->multiplayerWorld)
		{
			int_t bookshelfCount = 0;
			for (int_t dz = -1; dz <= 1; ++dz)
			{
				for (int_t dx = -1; dx <= 1; ++dx)
				{
					if ((dz == 0 && dx == 0) || !worldPointer->isAirBlock(posX + dx, posY, posZ + dz) ||
						!worldPointer->isAirBlock(posX + dx, posY + 1, posZ + dz))
						continue;

					if (Block::bookShelf != nullptr && worldPointer->getBlockId(posX + dx * 2, posY, posZ + dz * 2) == Block::bookShelf->blockID)
						++bookshelfCount;
					if (Block::bookShelf != nullptr && worldPointer->getBlockId(posX + dx * 2, posY + 1, posZ + dz * 2) == Block::bookShelf->blockID)
						++bookshelfCount;

					if (dx != 0 && dz != 0)
					{
						if (Block::bookShelf != nullptr && worldPointer->getBlockId(posX + dx * 2, posY, posZ + dz) == Block::bookShelf->blockID)
							++bookshelfCount;
						if (Block::bookShelf != nullptr && worldPointer->getBlockId(posX + dx * 2, posY + 1, posZ + dz) == Block::bookShelf->blockID)
							++bookshelfCount;
						if (Block::bookShelf != nullptr && worldPointer->getBlockId(posX + dx, posY, posZ + dz * 2) == Block::bookShelf->blockID)
							++bookshelfCount;
						if (Block::bookShelf != nullptr && worldPointer->getBlockId(posX + dx, posY + 1, posZ + dz * 2) == Block::bookShelf->blockID)
							++bookshelfCount;
					}
				}
			}

			for (int_t option = 0; option < 3; ++option)
				enchantLevels[option] = EnchantmentHelper::calcItemStackEnchantability(rand, option, bookshelfCount, stack);
			updateCraftingResults();
		}
	}
	else
	{
		enchantLevels[0] = 0;
		enchantLevels[1] = 0;
		enchantLevels[2] = 0;
	}
}

bool ContainerEnchantment::enchantItem(EntityPlayer *player, int_t option)
{
	if (player == nullptr || option < 0 || option >= 3 || worldPointer == nullptr)
		return false;
	ItemStack *stack = tableInventory->getStackInSlot(0);
	if (enchantLevels[option] <= 0 || stack == nullptr ||
		(player->experienceLevel < enchantLevels[option] && !player->capabilities.isCreativeMode))
		return false;

	if (!worldPointer->multiplayerWorld)
	{
		std::vector<EnchantmentData> enchantments = EnchantmentHelper::buildEnchantmentList(rand, stack, enchantLevels[option]);
		if (!enchantments.empty())
		{
			player->removeExperience(enchantLevels[option]);
			for (const EnchantmentData &entry : enchantments)
				stack->addEnchantment(entry.enchantmentobj, entry.enchantmentLevel);
			onCraftMatrixChanged(tableInventory);
		}
	}
	return true;
}

void ContainerEnchantment::onCraftGuiClosed(EntityPlayer *player)
{
	Container::onCraftGuiClosed(player);
	if (worldPointer == nullptr || worldPointer->multiplayerWorld || player == nullptr)
		return;
	ItemStack *stack = tableInventory->getStackInSlotOnClosing(0);
	if (stack != nullptr)
		player->dropPlayerItem(stack);
}

bool ContainerEnchantment::isUsableByPlayer(EntityPlayer *player)
{
	if (worldPointer == nullptr || player == nullptr || Block::enchantmentTable == nullptr)
		return false;
	if (worldPointer->getBlockId(posX, posY, posZ) != Block::enchantmentTable->blockID)
		return false;
	return player->getDistanceSq((double)posX + 0.5, (double)posY + 0.5, (double)posZ + 0.5) <= 64.0;
}

ItemStack *ContainerEnchantment::getStackInSlot(int_t slotIndex)
{
	if (slotIndex < 0 || slotIndex >= static_cast<int_t>(slots.size()))
		return nullptr;
	Slot *slot = slots[slotIndex];
	if (slot == nullptr || !slot->getHasStack())
		return nullptr;

	ItemStack *source = slot->getStack();
	ItemStack *result = source->copy();
	if (slotIndex != 0 || !mergeItemStack(source, 1, 37, true))
	{
		delete result;
		return nullptr;
	}

	ItemStack *orphaned = nullptr;
	if (source->stackSize == 0)
	{
		orphaned = slot->takeStack();
	}
	else
		slot->onSlotChanged();

	if (source->stackSize == result->stackSize)
	{
		delete orphaned;
		delete result;
		return nullptr;
	}

	slot->onPickupFromSlot(source);
	delete orphaned;
	return result;
}
