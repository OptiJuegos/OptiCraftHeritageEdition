#include "EntityZombie.h"

#include "Item.h"
#include "EntityAIAttackOnCollide.h"
#include "EntityAIBreakDoor.h"
#include "EntityAIHurtByTarget.h"
#include "EntityAILookIdle.h"
#include "EntityAIMoveThroughVillage.h"
#include "EntityAIMoveTwardsRestriction.h"
#include "EntityAINearestAttackableTarget.h"
#include "EntityAISwimming.h"
#include "EntityAIWander.h"
#include "EntityAIWatchClosest.h"
#include "EntityPlayer.h"
#include "EntityVillager.h"
#include "PathNavigate.h"
#include "MathHelper.h"
#include "World.h"

EntityZombie::EntityZombie(World *world)
	: EntityMob(world)
{
	texture = "/mob/zombie.png";
	moveSpeed = 0.23f;
	attackStrength = 4;
	getNavigator()->setBreakDoors(true);
	tasks.addTask(0, new EntityAISwimming(this));
	tasks.addTask(1, new EntityAIBreakDoor(this));
	tasks.addTask(2, new EntityAIAttackOnCollide(this, typeid(EntityPlayer), moveSpeed, false));
	tasks.addTask(3, new EntityAIAttackOnCollide(this, typeid(EntityVillager), moveSpeed, true));
	tasks.addTask(4, new EntityAIMoveTwardsRestriction(this, moveSpeed));
	tasks.addTask(5, new EntityAIMoveThroughVillage(this, moveSpeed, false));
	tasks.addTask(6, new EntityAIWander(this, moveSpeed));
	tasks.addTask(7, new EntityAIWatchClosest(this, typeid(EntityPlayer), 8.0f));
	tasks.addTask(7, new EntityAILookIdle(this));
	targetTasks.addTask(1, new EntityAIHurtByTarget(this, false));
	targetTasks.addTask(2, new EntityAINearestAttackableTarget(this, typeid(EntityPlayer), 16.0f, 0, true));
	targetTasks.addTask(2, new EntityAINearestAttackableTarget(this, typeid(EntityVillager), 16.0f, 0, false));
}

void EntityZombie::onLivingUpdate()
{
	if (worldObj->isDaytime() && !worldObj->multiplayerWorld)
	{
		float f = getEntityBrightness(1.0f);
		if (f > 0.5f && worldObj->canBlockSeeTheSky(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ)) && rand.nextFloat() * 30.0f < (f - 0.4f) * 2.0f)
		{
			setFire(8);
		}
	}
	EntityMob::onLivingUpdate();
}

bool EntityZombie::isAIEnabled()
{
	return true;
}

int_t EntityZombie::getMaxHealth() const
{
	return 20;
}

int_t EntityZombie::getTotalArmorValue() const
{
	return 2;
}

jstring EntityZombie::getLivingSound() { return "mob.zombie"; }
jstring EntityZombie::getHurtSound()   { return "mob.zombiehurt"; }
jstring EntityZombie::getDeathSound()  { return "mob.zombiedeath"; }

int_t EntityZombie::getDropItemId()
{
	return Item::rottenFlesh->shiftedIndex;
}

EnumCreatureAttribute EntityZombie::getCreatureAttribute() const
{
	return EnumCreatureAttribute::UNDEAD;
}

void EntityZombie::dropRareDrop(int_t)
{
	switch (rand.nextInt(4))
	{
	case 0: if (Item::swordSteel != nullptr) dropItem(Item::swordSteel->shiftedIndex, 1); break;
	case 1: if (Item::helmetSteel != nullptr) dropItem(Item::helmetSteel->shiftedIndex, 1); break;
	case 2: if (Item::ingotIron != nullptr) dropItem(Item::ingotIron->shiftedIndex, 1); break;
	case 3: if (Item::shovelSteel != nullptr) dropItem(Item::shovelSteel->shiftedIndex, 1); break;
	}
}
