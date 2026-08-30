#include "PlayerController.h"

#include "PlayerControllerCreative.h"

#include "Block.h"
#include "Container.h"
#include "Entity.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "Minecraft.h"
#include "Session.h"
#include "World.h"
#include "WorldProvider.h"

PlayerController::PlayerController(Minecraft *minecraft)
	: mc(minecraft)
	, field_1064_b(false)
{
}

void PlayerController::onWorldChanged(World *)
{
}

void PlayerController::clickBlock(int_t i, int_t j, int_t k, int_t l)
{
	mc->theWorld->onBlockHit(mc->thePlayer, i, j, k, l);
	sendBlockRemoved(i, j, k, l);
}

bool PlayerController::sendBlockRemoved(int_t i, int_t j, int_t k, int_t)
{
	World *world = mc->theWorld;
	Block *block = Block::blocksList[world->getBlockId(i, j, k)];
	if (block == nullptr)
		return false;
	world->playAuxSFX(2001, i, j, k, block->blockID + (world->getBlockMetadata(i, j, k) << 12));
	int_t i1 = world->getBlockMetadata(i, j, k);
	bool flag = world->setBlockWithNotify(i, j, k, 0);
	if (flag)
		block->onBlockDestroyedByPlayer(world, i, j, k, i1);
	return flag;
}

void PlayerController::sendBlockRemoving(int_t, int_t, int_t, int_t)
{
}

void PlayerController::resetBlockRemoving()
{
}

void PlayerController::setPartialTime(float)
{
}

float PlayerController::getBlockReachDistance()
{
	return 5.0f;
}

bool PlayerController::sendUseItem(EntityPlayer *entityplayer, World *world, ItemStack *itemstack)
{
	int_t i = itemstack->stackSize;
	ItemStack *itemstack1 = itemstack->useItemRightClick(world, entityplayer);
	if (itemstack1 != itemstack || (itemstack1 != nullptr && itemstack1->stackSize != i))
	{
		ItemStack *replacedStack = itemstack1 != itemstack ? itemstack : nullptr;
		entityplayer->inventory->mainInventory[entityplayer->inventory->currentItem] = itemstack1;
		delete replacedStack;
		if (itemstack1 != nullptr && itemstack1->stackSize == 0)
		{
			delete itemstack1;
			entityplayer->inventory->mainInventory[entityplayer->inventory->currentItem] = nullptr;
		}
		return true;
	}
	return false;
}

void PlayerController::onStoppedUsingItem(EntityPlayer *entityplayer)
{
	if (entityplayer != nullptr)
		entityplayer->stopUsingItem();
}

void PlayerController::flipPlayer(EntityPlayer *)
{
}

void PlayerController::updateController()
{
}

bool PlayerController::shouldDrawHUD()
{
	return true;
}

bool PlayerController::func_35643_e()
{
	return false;
}

bool PlayerController::func_35642_f()
{
	return false;
}

void PlayerController::initializePlayer(EntityPlayer *entityplayer)
{
	PlayerControllerCreative::disableAbilities(entityplayer);
}

bool PlayerController::sendPlaceBlock(EntityPlayer *entityplayer, World *world, ItemStack *itemstack, int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = world->getBlockId(i, j, k);
	if (i1 > 0 && Block::blocksList[i1]->blockActivated(world, i, j, k, entityplayer))
		return true;
	if (itemstack == nullptr)
		return false;
	return itemstack->useItem(entityplayer, world, i, j, k, l);
}

EntityPlayer *PlayerController::createPlayer(World *world)
{
	return new EntityPlayerSP(mc, world, mc->session, world->worldProvider->worldType);
}

void PlayerController::interactWithEntity(EntityPlayer *entityplayer, Entity *entity)
{
	entityplayer->useCurrentItemOnEntity(entity);
}

void PlayerController::attackEntity(EntityPlayer *entityplayer, Entity *entity)
{
	entityplayer->attackTargetEntityWithCurrentItem(entity);
}

ItemStack *PlayerController::handleWindowClick(int_t, int_t j, int_t k, bool flag, EntityPlayer *entityplayer)
{
	return entityplayer->craftingInventory->slotClick(j, k, flag, entityplayer);
}

void PlayerController::closeWindow(int_t, EntityPlayer *entityplayer)
{
	entityplayer->craftingInventory->onCraftGuiClosed(entityplayer);
	entityplayer->craftingInventory = entityplayer->inventorySlots;
}

ItemStack *PlayerController::windowClick(int_t windowId, int_t slotId, int_t button, bool shift, EntityPlayer *player)
{
	// Keep the descriptive C++ wrapper on the original virtual path. In
	// multiplayer PlayerControllerMP overrides handleWindowClick() to attach a
	// transaction id and send Packet102WindowClick; calling Container directly
	// here made every inventory/crafting click client-side only.
	return handleWindowClick(windowId, slotId, button, shift, player);
}

void PlayerController::sendEnchantPacket(int_t windowId, int_t enchantment)
{
	(void)windowId;
	(void)enchantment;
}

void PlayerController::onContainerClosed(int_t, EntityPlayer *player)
{
	player->craftingInventory->onCraftGuiClosed(player);
	// Java's func_20086_a resets this too. Dropping it left craftingInventory
	// pointing at a container the closing GuiContainer is about to delete.
	player->craftingInventory = player->inventorySlots;
}


bool PlayerController::isNotCreative()
{
	return true;
}

bool PlayerController::isInCreativeMode()
{
	return false;
}

bool PlayerController::extendedReach()
{
	return false;
}

void PlayerController::sendSlotPacket(ItemStack *, int_t)
{
}

void PlayerController::sendPacketDropItem(ItemStack *)
{
}
