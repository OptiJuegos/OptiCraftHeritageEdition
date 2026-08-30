#include "EntityChicken.h"

#include "EntityAIFollowParent.h"
#include "EntityAILookIdle.h"
#include "EntityAIMate.h"
#include "EntityAIPanic.h"
#include "EntityAISwimming.h"
#include "EntityAITempt.h"
#include "EntityAIWander.h"
#include "EntityAIWatchClosest.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "World.h"

EntityChicken::EntityChicken(World *world)
	: EntityAnimal(world), field_753_a(false), field_752_b(0.0f), destPos(0.0f), field_757_d(0.0f),
	  field_756_e(0.0f), field_755_h(1.0f), timeUntilNextEgg(0)
{
	ensureEntityInit();
	texture = "/mob/chicken.png";
	setSize(0.3f, 0.7f);
	health = 4;
	timeUntilNextEgg = rand.nextInt(6000) + 6000;
	const float speed = 0.25f;
	tasks.addTask(0, new EntityAISwimming(this));
	tasks.addTask(1, new EntityAIPanic(this, 0.38f));
	tasks.addTask(2, new EntityAIMate(this, speed));
	tasks.addTask(3, new EntityAITempt(this, 0.25f, Item::wheat->shiftedIndex, false));
	tasks.addTask(4, new EntityAIFollowParent(this, 0.28f));
	tasks.addTask(5, new EntityAIWander(this, speed));
	tasks.addTask(6, new EntityAIWatchClosest(this, typeid(EntityPlayer), 6.0f));
	tasks.addTask(7, new EntityAILookIdle(this));
}

bool EntityChicken::isAIEnabled()
{
	return true;
}

int_t EntityChicken::getMaxHealth() const
{
	return 4;
}

void EntityChicken::onLivingUpdate()
{
	EntityAnimal::onLivingUpdate();
	field_756_e = field_752_b;
	field_757_d = destPos;
	destPos = (float)((double)destPos + (double)(onGround ? -1 : 4) * 0.3);
	if (destPos < 0.0f)
		destPos = 0.0f;
	if (destPos > 1.0f)
		destPos = 1.0f;
	if (!onGround && field_755_h < 1.0f)
		field_755_h = 1.0f;
	field_755_h = (float)((double)field_755_h * 0.9);
	if (!onGround && motionY < 0.0)
		motionY *= 0.6;
	field_752_b += field_755_h * 2.0f;
	if (!isChild() && !worldObj->multiplayerWorld && --timeUntilNextEgg <= 0)
	{
		worldObj->playSoundAtEntity(this, "mob.chickenplop", 1.0f, rand.nextFloatDifference() * 0.2f + 1.0f);
		dropItem(Item::egg->shiftedIndex, 1);
		timeUntilNextEgg = rand.nextInt(6000) + 6000;
	}
}

jstring EntityChicken::getLivingSound() { return "mob.chicken"; }
jstring EntityChicken::getHurtSound() { return "mob.chickenhurt"; }
jstring EntityChicken::getDeathSound() { return "mob.chickenhurt"; }
int_t EntityChicken::getDropItemId() { return Item::feather->shiftedIndex; }

void EntityChicken::dropFewItems(bool, int_t lootingLevel)
{
	const int_t featherCountBase = rand.nextInt(3);
	const int_t featherCountLooting = rand.nextInt(1 + lootingLevel);
	const int_t featherCount = featherCountBase + featherCountLooting;
	for (int_t i = 0; i < featherCount; ++i)
		dropItem(Item::feather->shiftedIndex, 1);

	Item *meat = isBurning() ? Item::chickenCooked : Item::chickenRaw;
	if (meat != nullptr)
		dropItem(meat->shiftedIndex, 1);
}

EntityAnimal *EntityChicken::spawnBabyAnimal(EntityAnimal *mate)
{
	(void)mate;
	return new EntityChicken(worldObj);
}
