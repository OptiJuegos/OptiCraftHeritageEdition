#include "EntityVillager.h"

#include "DataWatcher.h"
#include "EntityAIAvoidEntity.h"
#include "EntityAIFollowGolem.h"
#include "EntityAIMoveIndoors.h"
#include "EntityAIMoveTwardsRestriction.h"
#include "EntityAIOpenDoor.h"
#include "EntityAIPlay.h"
#include "EntityAIRestrictOpenDoor.h"
#include "EntityAISwimming.h"
#include "EntityAIVillagerMate.h"
#include "EntityAIWander.h"
#include "EntityAIWatchClosest.h"
#include "EntityAIWatchClosest2.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "EntityZombie.h"
#include "NBTTagCompound.h"
#include "MathHelper.h"
#include "PathNavigate.h"
#include "Village.h"
#include "VillageCollection.h"
#include "World.h"

EntityVillager::EntityVillager(World *world)
	: EntityVillager(world, 0)
{
}

EntityVillager::EntityVillager(World *world, int_t profession)
	: EntityAgeable(world), randomTickDivider(0), isMatingFlag(false), isPlayingFlag(false)
{
	ensureEntityInit();
	setProfession(profession);
	texture = "/mob/villager/villager.png";
	moveSpeed = 0.5f;
	health = 20;
	prevHealth = 20;
	if (getNavigator() != nullptr)
	{
		getNavigator()->setBreakDoors(true);
		getNavigator()->setAvoidsWater(true);
	}
	tasks.addTask(0, new EntityAISwimming(this));
	tasks.addTask(1, new EntityAIAvoidEntity(this, typeid(EntityZombie), 8.0f, 0.3f, 0.35f));
	tasks.addTask(2, new EntityAIMoveIndoors(this));
	tasks.addTask(3, new EntityAIRestrictOpenDoor(this));
	tasks.addTask(4, new EntityAIOpenDoor(this, true));
	tasks.addTask(5, new EntityAIMoveTwardsRestriction(this, 0.3f));
	tasks.addTask(6, new EntityAIVillagerMate(this));
	tasks.addTask(7, new EntityAIFollowGolem(this));
	tasks.addTask(8, new EntityAIPlay(this, 0.32f));
	tasks.addTask(9, new EntityAIWatchClosest2(this, typeid(EntityPlayer), 3.0f, 1.0f));
	tasks.addTask(9, new EntityAIWatchClosest2(this, typeid(EntityVillager), 5.0f, 0.02f));
	tasks.addTask(9, new EntityAIWander(this, 0.3f));
	tasks.addTask(10, new EntityAIWatchClosest(this, typeid(EntityLiving), 8.0f));
}

void EntityVillager::entityInit()
{
	EntityAgeable::entityInit();
	dataWatcher->addObject(16, int_t(0));
}

const char *EntityVillager::getEntityTexture()
{
	switch (getProfession())
	{
	case 0: return "/mob/villager/farmer.png";
	case 1: return "/mob/villager/librarian.png";
	case 2: return "/mob/villager/priest.png";
	case 3: return "/mob/villager/smith.png";
	case 4: return "/mob/villager/butcher.png";
	default: return EntityAgeable::getEntityTexture();
	}
}

void EntityVillager::writeEntityToNBT(NBTTagCompound *tag)
{
	EntityAgeable::writeEntityToNBT(tag);
	tag->setInteger("Profession", getProfession());
}

void EntityVillager::readEntityFromNBT(NBTTagCompound *tag)
{
	EntityAgeable::readEntityFromNBT(tag);
	setProfession(tag->getInteger("Profession"));
}

void EntityVillager::onLivingUpdate()
{
	EntityAgeable::onLivingUpdate();
}

bool EntityVillager::attackEntityFrom(Entity *entity, int_t damage)
{
	return EntityAgeable::attackEntityFrom(entity, damage);
}

void EntityVillager::setRevengeTarget(EntityLiving *target)
{
	EntityAgeable::setRevengeTarget(target);
	if (target == nullptr || worldObj == nullptr || worldObj->villageCollectionObj == nullptr)
		return;
	Village *village = worldObj->villageCollectionObj->findNearestVillage(
		MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ), 32);
	if (village != nullptr)
		village->addOrRenewAgressor(target);
}

bool EntityVillager::isAIEnabled()
{
	return true;
}

int_t EntityVillager::getMaxHealth() const
{
	return 20;
}

void EntityVillager::updateAITick()
{
	if (--randomTickDivider <= 0)
	{
		randomTickDivider = 70 + rand.nextInt(50);
		if (worldObj != nullptr && worldObj->villageCollectionObj != nullptr)
		{
			int_t x = MathHelper::floor_double(posX);
			int_t y = MathHelper::floor_double(posY);
			int_t z = MathHelper::floor_double(posZ);
			worldObj->villageCollectionObj->addVillagerPosition(x, y, z);
			Village *village = worldObj->villageCollectionObj->findNearestVillage(x, y, z, 32);
			if (village == nullptr)
				detachHome();
			else
			{
				ChunkCoordinates center = village->getCenter();
				setHomeArea(center.x, center.y, center.z, village->getVillageRadius());
			}
		}
	}
	EntityAgeable::updateAITick();
}

bool EntityVillager::canDespawn()
{
	return false;
}

jstring EntityVillager::getLivingSound()
{
	return "mob.villager.default";
}

jstring EntityVillager::getHurtSound()
{
	return "mob.villager.defaulthurt";
}

jstring EntityVillager::getDeathSound()
{
	return "mob.villager.defaultdeath";
}

void EntityVillager::setProfession(int_t profession)
{
	dataWatcher->updateObject(16, profession);
}

int_t EntityVillager::getProfession()
{
	return dataWatcher->getWatchableObjectInt(16);
}

bool EntityVillager::getIsMatingFlag() const
{
	return isMatingFlag;
}

void EntityVillager::setIsMatingFlag(bool value)
{
	isMatingFlag = value;
}

void EntityVillager::setIsPlayingFlag(bool value)
{
	isPlayingFlag = value;
}

bool EntityVillager::getIsPlayingFlag() const
{
	return isPlayingFlag;
}
