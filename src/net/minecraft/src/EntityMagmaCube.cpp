#include "EntityMagmaCube.h"

#include "AxisAlignedBB.h"
#include "Item.h"
#include "World.h"

EntityMagmaCube::EntityMagmaCube(World *world)
	: EntitySlime(world)
{
	texture = "/mob/lava.png";
	immuneToFire = true;
	landMovementFactor = 0.2f;
}

bool EntityMagmaCube::getCanSpawnHere()
{
	return worldObj->difficultySetting > 0 && worldObj->checkIfAABBIsClear(boundingBox) &&
		worldObj->getCollidingBoundingBoxes(this, boundingBox).empty() && !worldObj->isAnyLiquid(boundingBox);
}

int_t EntityMagmaCube::getTotalArmorValue() const
{
	return const_cast<EntityMagmaCube *>(this)->getSlimeSize() * 3;
}

int_t EntityMagmaCube::getBrightnessForRender(float)
{
	return 15728880;
}

float EntityMagmaCube::getEntityBrightness(float)
{
	return 1.0f;
}

jstring EntityMagmaCube::getSlimeParticle()
{
	return "flame";
}

EntitySlime *EntityMagmaCube::createInstance()
{
	return new EntityMagmaCube(worldObj);
}

int_t EntityMagmaCube::getDropItemId()
{
	return Item::magmaCream != nullptr ? Item::magmaCream->shiftedIndex : 0;
}

void EntityMagmaCube::dropFewItems(bool, int_t lootingLevel)
{
	const int_t itemId = getDropItemId();
	if (itemId <= 0 || getSlimeSize() <= 1)
		return;

	int_t count = rand.nextInt(4) - 2;
	if (lootingLevel > 0)
		count += rand.nextInt(lootingLevel + 1);
	for (int_t i = 0; i < count; ++i)
		dropItem(itemId, 1);
}

bool EntityMagmaCube::isBurning()
{
	return false;
}

int_t EntityMagmaCube::getJumpDelay()
{
	return EntitySlime::getJumpDelay() * 4;
}

void EntityMagmaCube::updateSquish()
{
	field_40139_a *= 0.9f;
}

void EntityMagmaCube::jump()
{
	motionY = static_cast<double>(0.42f + static_cast<float>(getSlimeSize()) * 0.1f);
	isAirBorne = true;
}

void EntityMagmaCube::fall(float)
{
}

bool EntityMagmaCube::canDamagePlayer()
{
	return true;
}

int_t EntityMagmaCube::getAttackStrength()
{
	return EntitySlime::getAttackStrength() + 2;
}

jstring EntityMagmaCube::getHurtSound()
{
	return "mob.slime";
}

jstring EntityMagmaCube::getDeathSound()
{
	return "mob.slime";
}

jstring EntityMagmaCube::getSlimeSound()
{
	return getSlimeSize() > 1 ? "mob.magmacube.big" : "mob.magmacube.small";
}

bool EntityMagmaCube::handleLavaMovement()
{
	return false;
}

bool EntityMagmaCube::makesSoundOnLand()
{
	return true;
}
