#include "EntityOtherPlayerMP.h"

#include <cmath>

#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "platform/PlatformTuning.h"
#include "World.h"

EntityOtherPlayerMP::EntityOtherPlayerMP(World *world, jstring s)
	: EntityPlayer(world)
{
	ensureEntityInit(); // see Entity::ensureEntityInit — EntityPlayer::entityInit registers DataWatcher id 16
	field_20924_a = 0.0f;
	isItemInUse = false;
	username = s;
	yOffset = 0.0f;
	stepHeight = 0.0f;
	if (!s.empty())
	{
		skinUrl = "http://s3.amazonaws.com/MinecraftSkins/" + s + ".png";
	}
	noClip = true;
	field_22062_y = 0.25f;
	renderDistanceWeight = 10.0;
}

void EntityOtherPlayerMP::resetHeight()
{
	yOffset = 0.0f;
}

bool EntityOtherPlayerMP::attackEntityFrom(Entity *entity, int_t i)
{
	return true;
}

bool EntityOtherPlayerMP::attackEntityFrom(const DamageSource &, int_t)
{
	return true;
}

void EntityOtherPlayerMP::setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i)
{
	field_784_bh = d;
	field_783_bi = d1;
	field_782_bj = d2;
	field_780_bk = f;
	field_786_bl = f1;
	field_785_bg = i;
}

void EntityOtherPlayerMP::onUpdate()
{
	field_22062_y = 0.0f;
	EntityPlayer::onUpdate();
	field_705_Q = field_704_R;
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	const float d = static_cast<float>(posX - prevPosX);
	const float d1 = static_cast<float>(posZ - prevPosZ);
	float f = MathHelper::sqrt_float(d * d + d1 * d1) * 4.0f;
#else
	double d = posX - prevPosX;
	double d1 = posZ - prevPosZ;
	float f = MathHelper::sqrt_double(d * d + d1 * d1) * 4.0f;
#endif
	if (f > 1.0f)
	{
		f = 1.0f;
	}
	field_704_R += (f - field_704_R) * 0.4f;
	field_703_S += field_704_R;
	ItemStack *heldItem = inventory->mainInventory[inventory->currentItem];
	if (!isItemInUse && isEating() && heldItem != nullptr && heldItem->getItem() != nullptr)
	{
		setItemInUse(heldItem, heldItem->getItem()->getMaxItemUseDuration(heldItem));
		isItemInUse = true;
	}
	else if (isItemInUse && !isEating())
	{
		clearItemInUse();
		isItemInUse = false;
	}
}

float EntityOtherPlayerMP::getShadowSize()
{
	return 0.0f;
}

float EntityOtherPlayerMP::getEyeHeight()
{
	return 1.82f;
}

void EntityOtherPlayerMP::onLivingUpdate()
{
	EntityPlayer::updatePlayerActionState();
	if (field_785_bg > 0)
	{
		double d = posX + (field_784_bh - posX) / (double)field_785_bg;
		double d1 = posY + (field_783_bi - posY) / (double)field_785_bg;
		double d2 = posZ + (field_782_bj - posZ) / (double)field_785_bg;
		double d3;
		for (d3 = field_780_bk - (double)rotationYaw; d3 < -180.0; d3 += 360.0) {}
		for (; d3 >= 180.0; d3 -= 360.0) {}
		rotationYaw += (float)(d3 / (double)field_785_bg);
		rotationPitch += (float)((field_786_bl - (double)rotationPitch) / (double)field_785_bg);
		field_785_bg--;
		setPosition(d, d1, d2);
		setRotation(rotationYaw, rotationPitch);
	}
	field_775_e = field_774_f;
	float f = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
	float f1 = static_cast<float>(std::atan(-motionY * static_cast<double>(0.2f))) * 15.0f;
	if (f > 0.1f)
	{
		f = 0.1f;
	}
	if (!onGround || health <= 0)
	{
		f = 0.0f;
	}
	if (onGround || health <= 0)
	{
		f1 = 0.0f;
	}
	field_774_f += (f - field_774_f) * 0.4f;
	field_9328_R += (f1 - field_9328_R) * 0.8f;
}

void EntityOtherPlayerMP::outfitWithItem(int_t i, int_t j, int_t k)
{
	ItemStack *itemstack = nullptr;
	if (j >= 0)
	{
		itemstack = new ItemStack(j, 1, k);
	}
	ItemStack **slot = nullptr;
	if (i == 0)
		slot = &inventory->mainInventory[inventory->currentItem];
	else if (i >= 1 && i <= 4)
		slot = &inventory->armorInventory[i - 1];

	if (slot != nullptr)
	{
		delete *slot;
		*slot = itemstack;
	}
	else
	{
		delete itemstack;
	}
}

void EntityOtherPlayerMP::handleItemUseFinish()
{
	EntityPlayer::handleItemUseFinish();
}
