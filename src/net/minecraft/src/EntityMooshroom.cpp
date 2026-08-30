#include "EntityMooshroom.h"

#include "Block.h"
#include "BlockFlower.h"
#include "EntityItem.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "World.h"

EntityMooshroom::EntityMooshroom(World *world)
	: EntityCow(world)
{
	texture = "/mob/redcow.png";
	setSize(0.9f, 1.3f);
}

bool EntityMooshroom::interact(EntityPlayer *player)
{
	if (player == nullptr || player->inventory == nullptr)
		return false;

	ItemStack *stack = player->inventory->getCurrentItem();
	if (stack != nullptr && Item::bowlEmpty != nullptr && stack->itemID == Item::bowlEmpty->shiftedIndex && getGrowingAge() >= 0)
	{
		if (stack->stackSize == 1)
		{
			player->inventory->setInventorySlotContents(player->inventory->currentItem, new ItemStack(Item::bowlSoup));
			return true;
		}

		ItemStack *soup = new ItemStack(Item::bowlSoup);
		const bool stored = player->inventory->addItemStackToInventory(soup);
		delete soup;
		if (stored && !player->capabilities.isCreativeMode)
		{
			delete player->inventory->decrStackSize(player->inventory->currentItem, 1);
			return true;
		}
	}

	if (stack != nullptr && Item::shears != nullptr && stack->itemID == Item::shears->shiftedIndex && getGrowingAge() >= 0)
	{
		setEntityDead();
		worldObj->spawnParticle("largeexplode", posX, posY + static_cast<double>(height / 2.0f), posZ, 0.0, 0.0, 0.0);
		if (!worldObj->multiplayerWorld)
		{
			EntityCow *cow = new EntityCow(worldObj);
			cow->setLocationAndAngles(posX, posY, posZ, rotationYaw, rotationPitch);
			cow->setHealth(getHealth());
			cow->renderYawOffset = renderYawOffset;
			if (!worldObj->entityJoinedWorld(cow))
				delete cow;

			for (int_t i = 0; i < 5; ++i)
			{
				EntityItem *item = new EntityItem(worldObj, posX, posY + static_cast<double>(height), posZ,
					new ItemStack(Block::mushroomRed));
				if (!worldObj->entityJoinedWorld(item))
					delete item;
			}
		}
		return true;
	}

	return EntityCow::interact(player);
}

EntityAnimal *EntityMooshroom::spawnBabyAnimal(EntityAnimal *)
{
	return new EntityMooshroom(worldObj);
}
