#include "TileEntityChest.h"

#include "Block.h"
#include "EntityPlayer.h"
#include "ItemStack.h"
#include "NBTTagCompound.h"
#include "NBTTagList.h"
#include "World.h"

TileEntityChest::TileEntityChest()
	: adjacentChestChecked(false),
	  adjacentChestZNeg(nullptr),
	  adjacentChestXPos(nullptr),
	  adjacentChestXNeg(nullptr),
	  adjacentChestZPos(nullptr),
	  lidAngle(0.0f),
	  prevLidAngle(0.0f),
	  numUsingPlayers(0),
	  ticksSinceSync(0)
{
	for (int_t i = 0; i < 36; ++i)
		chestContents[i] = nullptr;
}

TileEntityChest::~TileEntityChest()
{
	for (int_t i = 0; i < 36; ++i)
	{
		delete chestContents[i];
		chestContents[i] = nullptr;
	}
}

int_t TileEntityChest::getSizeInventory()
{
	return 27;
}

ItemStack *TileEntityChest::getStackInSlot(int_t i)
{
	return i >= 0 && i < getSizeInventory() ? chestContents[i] : nullptr;
}

ItemStack *TileEntityChest::getStackInSlotOnClosing(int_t i)
{
	if (i < 0 || i >= getSizeInventory() || chestContents[i] == nullptr)
		return nullptr;

	ItemStack *stack = chestContents[i];
	chestContents[i] = nullptr;
	return stack;
}

ItemStack *TileEntityChest::decrStackSize(int_t i, int_t amount)
{
	if (i < 0 || i >= getSizeInventory() || chestContents[i] == nullptr)
		return nullptr;

	if (chestContents[i]->stackSize <= amount)
	{
		ItemStack *stack = chestContents[i];
		chestContents[i] = nullptr;
		onInventoryChanged();
		return stack;
	}

	ItemStack *stack = chestContents[i]->splitStack(amount);
	if (chestContents[i]->stackSize == 0)
	{
		delete chestContents[i];
		chestContents[i] = nullptr;
	}
	onInventoryChanged();
	return stack;
}

void TileEntityChest::setInventorySlotContents(int_t i, ItemStack *itemstack)
{
	if (i < 0 || i >= getSizeInventory())
	{
		delete itemstack;
		return;
	}

	if (chestContents[i] != itemstack)
		delete chestContents[i];
	chestContents[i] = itemstack;
	if (itemstack != nullptr && itemstack->stackSize > getInventoryStackLimit())
		itemstack->stackSize = getInventoryStackLimit();
	onInventoryChanged();
}

std::string TileEntityChest::getInvName()
{
	return "container.chest";
}

void TileEntityChest::readFromNBT(NBTTagCompound *compound)
{
	TileEntity::readFromNBT(compound);
	for (int_t i = 0; i < 36; ++i)
	{
		delete chestContents[i];
		chestContents[i] = nullptr;
	}

	NBTTagList *items = compound->getTagList("Items");
	for (int_t i = 0; i < items->tagCount(); ++i)
	{
		NBTTagCompound *itemTag = dynamic_cast<NBTTagCompound *>(items->tagAt(i));
		if (itemTag == nullptr)
			continue;
		int_t slot = itemTag->getByte("Slot") & 0xff;
		if (slot >= 0 && slot < getSizeInventory())
		{
			ItemStack *loaded = ItemStack::loadItemStackFromNBT(itemTag);
			delete chestContents[slot];
			chestContents[slot] = loaded;
		}
	}
}

void TileEntityChest::writeToNBT(NBTTagCompound *compound)
{
	TileEntity::writeToNBT(compound);
	NBTTagList *items = new NBTTagList();
	for (int_t i = 0; i < getSizeInventory(); ++i)
	{
		if (chestContents[i] == nullptr)
			continue;
		NBTTagCompound *itemTag = new NBTTagCompound();
		itemTag->setByte("Slot", (char)i);
		chestContents[i]->writeToNBT(itemTag);
		items->setTag(itemTag);
	}
	compound->setTag("Items", items);
}

int_t TileEntityChest::getInventoryStackLimit()
{
	return 64;
}

bool TileEntityChest::canInteractWith(EntityPlayer *player)
{
	if (worldObj == nullptr || worldObj->getBlockTileEntity(xCoord, yCoord, zCoord) != this)
		return false;
	return player->getDistanceSq((double)xCoord + 0.5, (double)yCoord + 0.5, (double)zCoord + 0.5) <= 64.0;
}

void TileEntityChest::updateContainingBlockInfo()
{
	TileEntity::updateContainingBlockInfo();
	adjacentChestChecked = false;
}

void TileEntityChest::checkForAdjacentChests()
{
	if (adjacentChestChecked || worldObj == nullptr)
		return;

	adjacentChestChecked = true;
	adjacentChestZNeg = nullptr;
	adjacentChestXPos = nullptr;
	adjacentChestXNeg = nullptr;
	adjacentChestZPos = nullptr;

	auto getAdjacent = [this](int_t x, int_t y, int_t z) -> TileEntityChest *
	{
		if (worldObj->getBlockId(x, y, z) != Block::chest->blockID)
			return nullptr;
		return dynamic_cast<TileEntityChest *>(worldObj->getBlockTileEntity(x, y, z));
	};

	adjacentChestXNeg = getAdjacent(xCoord - 1, yCoord, zCoord);
	adjacentChestXPos = getAdjacent(xCoord + 1, yCoord, zCoord);
	adjacentChestZNeg = getAdjacent(xCoord, yCoord, zCoord - 1);
	adjacentChestZPos = getAdjacent(xCoord, yCoord, zCoord + 1);

	if (adjacentChestZNeg != nullptr)
		adjacentChestZNeg->updateContainingBlockInfo();
	if (adjacentChestZPos != nullptr)
		adjacentChestZPos->updateContainingBlockInfo();
	if (adjacentChestXPos != nullptr)
		adjacentChestXPos->updateContainingBlockInfo();
	if (adjacentChestXNeg != nullptr)
		adjacentChestXNeg->updateContainingBlockInfo();
}

void TileEntityChest::updateEntity()
{
	TileEntity::updateEntity();
	checkForAdjacentChests();
	if (worldObj == nullptr)
		return;

	++ticksSinceSync;
	if ((ticksSinceSync % 20) * 4 == 0)
		worldObj->playNoteAt(xCoord, yCoord, zCoord, 1, numUsingPlayers);

	prevLidAngle = lidAngle;
	const float speed = 0.1f;
	if (numUsingPlayers > 0 && lidAngle == 0.0f && adjacentChestZNeg == nullptr && adjacentChestXNeg == nullptr)
	{
		double soundX = (double)xCoord + 0.5;
		double soundZ = (double)zCoord + 0.5;
		if (adjacentChestZPos != nullptr)
			soundZ += 0.5;
		if (adjacentChestXPos != nullptr)
			soundX += 0.5;
		worldObj->playSoundEffect(soundX, (double)yCoord + 0.5, soundZ, "random.chestopen", 0.5f, worldObj->rand.nextFloat() * 0.1f + 0.9f);
	}

	if ((numUsingPlayers == 0 && lidAngle > 0.0f) || (numUsingPlayers > 0 && lidAngle < 1.0f))
	{
		float previous = lidAngle;
		lidAngle += numUsingPlayers > 0 ? speed : -speed;
		if (lidAngle > 1.0f)
			lidAngle = 1.0f;

		if (lidAngle < 0.5f && previous >= 0.5f && adjacentChestZNeg == nullptr && adjacentChestXNeg == nullptr)
		{
			double soundX = (double)xCoord + 0.5;
			double soundZ = (double)zCoord + 0.5;
			if (adjacentChestZPos != nullptr)
				soundZ += 0.5;
			if (adjacentChestXPos != nullptr)
				soundX += 0.5;
			worldObj->playSoundEffect(soundX, (double)yCoord + 0.5, soundZ, "random.chestclosed", 0.5f, worldObj->rand.nextFloat() * 0.1f + 0.9f);
		}
		if (lidAngle < 0.0f)
			lidAngle = 0.0f;
	}
}

void TileEntityChest::onTileEntityPowered(int_t eventId, int_t eventData)
{
	if (eventId == 1)
		numUsingPlayers = eventData;
}

void TileEntityChest::openChest()
{
	++numUsingPlayers;
	if (worldObj != nullptr)
		worldObj->playNoteAt(xCoord, yCoord, zCoord, 1, numUsingPlayers);
}

void TileEntityChest::closeChest()
{
	--numUsingPlayers;
	if (worldObj != nullptr)
		worldObj->playNoteAt(xCoord, yCoord, zCoord, 1, numUsingPlayers);
}

void TileEntityChest::invalidate()
{
	updateContainingBlockInfo();
	checkForAdjacentChests();
	TileEntity::invalidate();
}
