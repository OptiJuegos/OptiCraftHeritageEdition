#include "EntityPig.h"

#include "AchievementList.h"
#include "DataWatcher.h"
#include "EntityAIFollowParent.h"
#include "EntityAILookIdle.h"
#include "EntityAIMate.h"
#include "EntityAIPanic.h"
#include "EntityAISwimming.h"
#include "EntityAITempt.h"
#include "EntityAIWander.h"
#include "EntityAIWatchClosest.h"
#include "EntityPigZombie.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "NBTTagCompound.h"
#include "PathNavigate.h"
#include "World.h"

EntityPig::EntityPig(World *world)
	: EntityAnimal(world)
{
	ensureEntityInit();
	texture = "/mob/pig.png";
	setSize(0.9f, 0.9f);
	if (getNavigator() != nullptr)
		getNavigator()->setAvoidsWater(true);
	const float speed = 0.25f;
	tasks.addTask(0, new EntityAISwimming(this));
	tasks.addTask(1, new EntityAIPanic(this, 0.38f));
	tasks.addTask(2, new EntityAIMate(this, speed));
	tasks.addTask(3, new EntityAITempt(this, 0.25f, Item::wheat->shiftedIndex, false));
	tasks.addTask(4, new EntityAIFollowParent(this, 0.28f));
	tasks.addTask(5, new EntityAIWander(this, speed));
	tasks.addTask(6, new EntityAIWatchClosest(this, typeid(EntityPlayer), 6.0f));
	tasks.addTask(7, new EntityAILookIdle(this));
	setHealth(getMaxHealth());
}

bool EntityPig::isAIEnabled()
{
	return true;
}

int_t EntityPig::getMaxHealth() const
{
	return 10;
}

void EntityPig::entityInit()
{
	EntityAnimal::entityInit();
	dataWatcher->addObject(16, (byte_t)0);
}

void EntityPig::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityAnimal::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setBoolean("Saddle", getSaddled());
}

void EntityPig::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityAnimal::readEntityFromNBT(nbttagcompound);
	setSaddled(nbttagcompound->getBoolean("Saddle"));
}

jstring EntityPig::getLivingSound() { return "mob.pig"; }
jstring EntityPig::getHurtSound() { return "mob.pig"; }
jstring EntityPig::getDeathSound() { return "mob.pigdeath"; }

bool EntityPig::interact(EntityPlayer *entityplayer)
{
	if (EntityAnimal::interact(entityplayer))
		return true;
	if (!getSaddled() || worldObj->multiplayerWorld || (riddenByEntity != nullptr && riddenByEntity != entityplayer))
		return false;
	entityplayer->mountEntity(this);
	return true;
}

int_t EntityPig::getDropItemId()
{
	return fire > 0 ? Item::porkCooked->shiftedIndex : Item::porkRaw->shiftedIndex;
}

bool EntityPig::getSaddled()
{
	return (dataWatcher->getWatchableObjectByte(16) & 1) != 0;
}

void EntityPig::setSaddled(bool flag)
{
	dataWatcher->updateObject(16, (byte_t)(flag ? 1 : 0));
}

void EntityPig::onStruckByLightning(EntityLightningBolt *entitylightningbolt)
{
	(void)entitylightningbolt;
	if (worldObj->multiplayerWorld)
		return;
	EntityPigZombie *entitypigzombie = new EntityPigZombie(worldObj);
	entitypigzombie->setLocationAndAngles(posX, posY, posZ, rotationYaw, rotationPitch);
	if (worldObj->entityJoinedWorld(entitypigzombie))
		setEntityDead();
	else
		delete entitypigzombie;
}

void EntityPig::fall(float f)
{
	EntityAnimal::fall(f);
	if (f > 5.0f && riddenByEntity != nullptr && riddenByEntity->isPlayer())
		((EntityPlayer *)riddenByEntity)->triggerAchievement(AchievementList::flyPig);
}

EntityAnimal *EntityPig::spawnBabyAnimal(EntityAnimal *mate)
{
	(void)mate;
	return new EntityPig(worldObj);
}
