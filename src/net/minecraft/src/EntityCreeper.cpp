#include "EntityCreeper.h"

#include "EntityLightningBolt.h"
#include "EntityAIAttackOnCollide.h"
#include "EntityAIAvoidEntity.h"
#include "EntityAICreeperSwell.h"
#include "EntityAIHurtByTarget.h"
#include "EntityAILookIdle.h"
#include "EntityAINearestAttackableTarget.h"
#include "EntityAISwimming.h"
#include "EntityAIWander.h"
#include "EntityAIWatchClosest.h"
#include "EntityOcelot.h"
#include "EntityPlayer.h"
#include "EntitySkeleton.h"
#include "DamageSource.h"
#include "Item.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "Explosion.h"
#include "DataWatcher.h"
#include "java/String.h"

EntityCreeper::EntityCreeper(World *world)
	: EntityMob(world)
	, timeSinceIgnited(0)
	, lastActiveTime(0)
{
	ensureEntityInit();
	texture = "/mob/creeper.png";
	tasks.addTask(1, new EntityAISwimming(this));
	tasks.addTask(2, new EntityAICreeperSwell(this));
	tasks.addTask(3, new EntityAIAvoidEntity(this, typeid(EntityOcelot), 6.0f, 0.25f, 0.3f));
	tasks.addTask(4, new EntityAIAttackOnCollide(this, 0.25f, false));
	tasks.addTask(5, new EntityAIWander(this, 0.2f));
	tasks.addTask(6, new EntityAIWatchClosest(this, typeid(EntityPlayer), 8.0f));
	tasks.addTask(6, new EntityAILookIdle(this));
	targetTasks.addTask(1, new EntityAINearestAttackableTarget(this, typeid(EntityPlayer), 16.0f, 0, true));
	targetTasks.addTask(2, new EntityAIHurtByTarget(this, false));
}

void EntityCreeper::entityInit()
{
	EntityMob::entityInit();
	dataWatcher->addObject(16, (byte_t)-1);
	dataWatcher->addObject(17, (byte_t)0);
}

void EntityCreeper::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityMob::writeEntityToNBT(nbttagcompound);
	if (dataWatcher->getWatchableObjectByte(17) == 1)
	{
		nbttagcompound->setBoolean("powered", true);
	}
}

void EntityCreeper::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityMob::readEntityFromNBT(nbttagcompound);
	dataWatcher->updateObject(17, (byte_t)(nbttagcompound->getBoolean("powered") ? 1 : 0));
}

void EntityCreeper::onUpdate()
{
	if (isEntityAlive())
	{
		lastActiveTime = timeSinceIgnited;
		int_t state = getCreeperState();
		if (state > 0 && timeSinceIgnited == 0)
			worldObj->playSoundAtEntity(this, "random.fuse", 1.0f, 0.5f);
		timeSinceIgnited += state;
		if (timeSinceIgnited < 0)
			timeSinceIgnited = 0;
		if (timeSinceIgnited >= 30)
		{
			timeSinceIgnited = 30;
			if (!worldObj->multiplayerWorld)
			{
				delete worldObj->createExplosion(this, posX, posY, posZ, getPowered() ? 6.0f : 3.0f);
				setEntityDead();
			}
		}
	}
	EntityMob::onUpdate();
}

bool EntityCreeper::isAIEnabled()
{
	return true;
}

bool EntityCreeper::attackEntityAsMob(Entity *)
{
	return true;
}

jstring EntityCreeper::getHurtSound()   { return "mob.creeper"; }
jstring EntityCreeper::getDeathSound()  { return "mob.creeperdeath"; }

void EntityCreeper::onDeath(Entity *entity)
{
	EntityMob::onDeath(entity);
	// The killer is null whenever nothing killed the creeper: fall damage,
	// suffocation, fire, drowning, cactus, an explosion, or the
	// handleHealthUpdate(3) path that calls onDeath(nullptr) outright.
	// The Java original wrote `entity instanceof EntitySkeleton`, and
	// `null instanceof X` is simply false there, so the null case never had to
	// be written out; translated to a virtual call it dereferences the null
	// and takes the EE down (TLB miss at 0x0, then a jump through vtable slot
	// 0xd0 to address 0). EntityLiving::onDeath and EntityPlayer::onDeath
	// already guard the same parameter — this override was the one that missed.
	if (dynamic_cast<EntitySkeleton*>(entity) != nullptr)
	{
		dropItem(Item::record13->shiftedIndex + rand.nextInt(10), 1);
	}
}

void EntityCreeper::onDeath(const DamageSource &source)
{
	EntityMob::onDeath(source);
	if (dynamic_cast<EntitySkeleton *>(source.getEntity()) != nullptr && Item::record13 != nullptr)
		dropItem(Item::record13->shiftedIndex + rand.nextInt(10), 1);
}

bool EntityCreeper::getPowered()
{
	return dataWatcher->getWatchableObjectByte(17) == 1;
}

float EntityCreeper::setCreeperFlashTime(float f)
{
	return ((float)lastActiveTime + (float)(timeSinceIgnited - lastActiveTime) * f) / 28.0f;
}

int_t EntityCreeper::getDropItemId()
{
	return Item::gunpowder->shiftedIndex;
}

int_t EntityCreeper::getCreeperState()
{
	return dataWatcher->getWatchableObjectByte(16);
}

void EntityCreeper::setCreeperState(int_t i)
{
	dataWatcher->updateObject(16, (byte_t)i);
}

void EntityCreeper::onStruckByLightning(EntityLightningBolt *entitylightningbolt)
{
	EntityMob::onStruckByLightning(entitylightningbolt);
	dataWatcher->updateObject(17, (byte_t)1);
}
