#include "EntityClientPlayerMP.h"

#include "AxisAlignedBB.h"
#include "client/Minecraft.h"
#include "Container.h"
#include "DamageSource.h"
#include "EntityItem.h"
#include "GuiIngame.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "NetClientHandler.h"
#include "Packet10Flying.h"
#include "Packet101CloseWindow.h"
#include "Packet11PlayerPosition.h"
#include "Packet12PlayerLook.h"
#include "Packet13PlayerLookMove.h"
#include "Packet14BlockDig.h"
#include "Packet18Animation.h"
#include "Packet19EntityAction.h"
#include "Packet202PlayerAbilities.h"
#include "Packet3Chat.h"
#include "Packet9Respawn.h"
#include "StatBase.h"
#include "World.h"
#include "WorldType.h"
#include "WorldInfo.h"
#include "java/String.h"

EntityClientPlayerMP::EntityClientPlayerMP(Minecraft *minecraft, World *world, Session *session, NetClientHandler *netclienthandler)
	: EntityPlayerSP(minecraft, world, session, 0)
	, sendQueue(netclienthandler)
	, field_9380_bx(0)
	, field_21093_bH(false)
	, oldPosX(0.0)
	, field_9378_bz(0.0)
	, oldPosY(0.0)
	, oldPosZ(0.0)
	, oldRotationYaw(0.0f)
	, oldRotationPitch(0.0f)
	, field_9382_bF(false)
	, wasSprinting(false)
	, wasSneaking(false)
	, field_12242_bI(0)
{
}

bool EntityClientPlayerMP::attackEntityFrom(Entity *, int_t)
{
	return false;
}

bool EntityClientPlayerMP::attackEntityFrom(const DamageSource &, int_t)
{
	return false;
}

void EntityClientPlayerMP::heal(int_t)
{
}

void EntityClientPlayerMP::onUpdate()
{
	if (!worldObj->blockExists(MathHelper::floor_double(posX), 0, MathHelper::floor_double(posZ)))
		return;
	EntityPlayerSP::onUpdate();
	sendMotionUpdates();
}

void EntityClientPlayerMP::sendMotionUpdates()
{
	if (field_9380_bx++ == 20)
	{
		sendInventoryChanged();
		field_9380_bx = 0;
	}
	const bool sprinting = isSprinting();
	if (sprinting != wasSprinting)
	{
		sendQueue->addToSendQueue(new Packet19EntityAction(this, sprinting ? 4 : 5));
		wasSprinting = sprinting;
	}

	const bool sneaking = isSneaking();
	if (sneaking != wasSneaking)
	{
		sendQueue->addToSendQueue(new Packet19EntityAction(this, sneaking ? 1 : 2));
		wasSneaking = sneaking;
	}
	double d = posX - oldPosX;
	double d1 = boundingBox->minY - field_9378_bz;
	double d2 = posY - oldPosY;
	double d3 = posZ - oldPosZ;
	double d4 = rotationYaw - oldRotationYaw;
	double d5 = rotationPitch - oldRotationPitch;
	bool flag1 = d1 != 0.0 || d2 != 0.0 || d != 0.0 || d3 != 0.0;
	bool flag2 = d4 != 0.0 || d5 != 0.0;
	if (ridingEntity != nullptr)
	{
		if (flag2)
			sendQueue->addToSendQueue(new Packet11PlayerPosition(motionX, -999.0, -999.0, motionZ, onGround));
		else
			sendQueue->addToSendQueue(new Packet13PlayerLookMove(motionX, -999.0, -999.0, motionZ, rotationYaw, rotationPitch, onGround));
		flag1 = false;
	}
	else if (flag1 && flag2)
	{
		sendQueue->addToSendQueue(new Packet13PlayerLookMove(posX, boundingBox->minY, posY, posZ, rotationYaw, rotationPitch, onGround));
		field_12242_bI = 0;
	}
	else if (flag1)
	{
		sendQueue->addToSendQueue(new Packet11PlayerPosition(posX, boundingBox->minY, posY, posZ, onGround));
		field_12242_bI = 0;
	}
	else if (flag2)
	{
		sendQueue->addToSendQueue(new Packet12PlayerLook(rotationYaw, rotationPitch, onGround));
		field_12242_bI = 0;
	}
	else
	{
		sendQueue->addToSendQueue(new Packet10Flying(onGround));
		if (field_9382_bF != onGround || field_12242_bI > 200)
			field_12242_bI = 0;
		else
			field_12242_bI++;
	}
	field_9382_bF = onGround;
	if (flag1)
	{
		oldPosX = posX;
		field_9378_bz = boundingBox->minY;
		oldPosY = posY;
		oldPosZ = posZ;
	}
	if (flag2)
	{
		oldRotationYaw = rotationYaw;
		oldRotationPitch = rotationPitch;
	}
}

void EntityClientPlayerMP::dropCurrentItem()
{
	(void)dropOneItem();
}

EntityItem *EntityClientPlayerMP::dropOneItem()
{
	if (sendQueue != nullptr)
		sendQueue->addToSendQueue(new Packet14BlockDig(4, 0, 0, 0, 0));
	return nullptr;
}

void EntityClientPlayerMP::joinEntityItemWithWorld(EntityItem *entityitem)
{
	// The multiplayer client never inserts authoritative item drops locally.
	// Java lets the temporary object fall out of scope; C++ must release it.
	delete entityitem;
}

void EntityClientPlayerMP::sendInventoryChanged()
{
}

void EntityClientPlayerMP::sendChatMessage(const std::string &s)
{
	if (mc != nullptr && mc->ingameGUI != nullptr)
	{
		std::vector<std::string> &history = mc->ingameGUI->getSentMessages();
		if (history.empty() || history.back() != s)
			history.push_back(s);
	}
	sendQueue->addToSendQueue(new Packet3Chat(s));
}

void EntityClientPlayerMP::swingItem()
{
	EntityPlayerSP::swingItem();
	sendQueue->addToSendQueue(new Packet18Animation(this, 1));
}

void EntityClientPlayerMP::respawnPlayer()
{
	WorldType *terrainType = worldObj != nullptr && worldObj->getWorldInfo() != nullptr
		? worldObj->getWorldInfo()->getTerrainType()
		: WorldType::DEFAULT;
	const int_t difficulty = worldObj != nullptr ? worldObj->difficultySetting : 0;
	const int_t worldHeight = worldObj != nullptr ? worldObj->getHeight() : 256;
	sendQueue->addToSendQueue(new Packet9Respawn(dimension, static_cast<byte_t>(difficulty), terrainType, worldHeight, 0));
}

void EntityClientPlayerMP::damageEntity(int_t i)
{
	damageEntity(DamageSource::generic, i);
}

void EntityClientPlayerMP::damageEntity(const DamageSource &, int_t damage)
{
	health -= damage;
}

void EntityClientPlayerMP::closeScreen()
{
	sendQueue->addToSendQueue(new Packet101CloseWindow(craftingInventory->windowId));
	ItemStack *cursorStack = inventory->getItemStack();
	inventory->setItemStack(nullptr);
	delete cursorStack;
	EntityPlayerSP::closeScreen();
}

void EntityClientPlayerMP::setHealth(int_t i)
{
	if (field_21093_bH)
		EntityPlayerSP::setHealth(i);
	else
	{
		health = i;
		field_21093_bH = true;
	}
}

void EntityClientPlayerMP::addStat(StatBase *statbase, int_t i)
{
	// Release 1.2.5 keeps independent stats client-authoritative. Server-owned
	// stats arrive through Packet200 and use incrementStat().
	if (statbase != nullptr && statbase->independent)
		EntityPlayerSP::addStat(statbase, i);
}

void EntityClientPlayerMP::incrementStat(StatBase *statbase, int_t i)
{
	if (statbase != nullptr && !statbase->independent)
		EntityPlayerSP::addStat(statbase, i);
}

void EntityClientPlayerMP::addServerStatistic(StatBase *statbase, int_t i)
{
	incrementStat(statbase, i);
}

void EntityClientPlayerMP::func_50009_aI()
{
	if (sendQueue != nullptr)
		sendQueue->addToSendQueue(new Packet202PlayerAbilities(&capabilities));
}
