#include "EntityIronGolem.h"
#include "DamageSource.h"

#include <string>

#include "Block.h"
#include "BlockFlower.h"
#include "DataWatcher.h"
#include "EntityAIAttackOnCollide.h"
#include "EntityAIDefendVillage.h"
#include "EntityAIHurtByTarget.h"
#include "EntityAILookAtVillager.h"
#include "EntityAILookIdle.h"
#include "EntityAIMoveThroughVillage.h"
#include "EntityAIMoveTowardsTarget.h"
#include "EntityAIMoveTwardsRestriction.h"
#include "EntityAINearestAttackableTarget.h"
#include "EntityAIWander.h"
#include "EntityAIWatchClosest.h"
#include "EntityMob.h"
#include "Entity.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "PathNavigate.h"
#include "Village.h"
#include "VillageCollection.h"
#include "World.h"

EntityIronGolem::EntityIronGolem(World *world)
	: EntityGolem(world), homeCheckTimer(0), attackTimer(0), holdRoseTick(0),
	  villageCenterX(0), villageCenterY(0), villageCenterZ(0), hasVillageCenter(false)
{
	ensureEntityInit();
	texture = "/mob/villager_golem.png";
	setSize(1.4f, 2.9f);
	health = 100;
	prevHealth = 100;
	if (getNavigator() != nullptr)
		getNavigator()->setAvoidsWater(true);
	tasks.addTask(1, new EntityAIAttackOnCollide(this, 0.25f, true));
	tasks.addTask(2, new EntityAIMoveTowardsTarget(this, 0.22f, 32.0f));
	tasks.addTask(3, new EntityAIMoveThroughVillage(this, 0.16f, true));
	tasks.addTask(4, new EntityAIMoveTwardsRestriction(this, 0.16f));
	tasks.addTask(5, new EntityAILookAtVillager(this));
	tasks.addTask(6, new EntityAIWander(this, 0.16f));
	tasks.addTask(7, new EntityAIWatchClosest(this, typeid(EntityPlayer), 6.0f));
	tasks.addTask(8, new EntityAILookIdle(this));
	targetTasks.addTask(1, new EntityAIDefendVillage(this));
	targetTasks.addTask(2, new EntityAIHurtByTarget(this, false));
	targetTasks.addTask(3, new EntityAINearestAttackableTarget(this, typeid(EntityMob), 16.0f, 0, false, true));
}

void EntityIronGolem::entityInit()
{
	EntityLiving::entityInit();
	dataWatcher->addObject(16, (byte_t)0);
}

bool EntityIronGolem::isAIEnabled()
{
	return true;
}

int_t EntityIronGolem::decreaseAirSupply(int_t airSupply)
{
	return airSupply;
}

void EntityIronGolem::updateAITick()
{
	if (--homeCheckTimer <= 0)
	{
		homeCheckTimer = 70 + rand.nextInt(50);
		Village *village = resolveVillage();
		if (village == nullptr)
		{
			hasVillageCenter = false;
			detachHome();
		}
		else
		{
			ChunkCoordinates center = village->getCenter();
			villageCenterX = center.x;
			villageCenterY = center.y;
			villageCenterZ = center.z;
			hasVillageCenter = true;
			setHomeArea(center.x, center.y, center.z, village->getVillageRadius());
		}
	}
	EntityGolem::updateAITick();
}

int_t EntityIronGolem::getMaxHealth() const
{
	return 100;
}

void EntityIronGolem::onLivingUpdate()
{
	EntityGolem::onLivingUpdate();
	if (attackTimer > 0)
		--attackTimer;
	if (holdRoseTick > 0)
		--holdRoseTick;
	if (motionX * motionX + motionZ * motionZ > static_cast<double>(2.5000003e-7f) && rand.nextInt(5) == 0)
	{
		int_t x = MathHelper::floor_double(posX);
		int_t y = MathHelper::floor_double(posY - static_cast<double>(0.2f) - (double)yOffset);
		int_t z = MathHelper::floor_double(posZ);
		int_t blockId = worldObj->getBlockId(x, y, z);
		if (blockId > 0)
		{
			const double particleX = posX + ((double)rand.nextFloat() - 0.5) * (double)width;
			const double particleZ = posZ + ((double)rand.nextFloat() - 0.5) * (double)width;
			const double motionX = 4.0 * ((double)rand.nextFloat() - 0.5);
			const double motionZ = ((double)rand.nextFloat() - 0.5) * 4.0;
			worldObj->spawnParticle("tilecrack_" + std::to_string(blockId),
				particleX, boundingBox->minY + 0.1, particleZ, motionX, 0.5, motionZ);
		}
	}
}

void EntityIronGolem::writeEntityToNBT(NBTTagCompound *tag)
{
	EntityGolem::writeEntityToNBT(tag);
	tag->setBoolean("PlayerCreated", isPlayerCreated());
}

void EntityIronGolem::readEntityFromNBT(NBTTagCompound *tag)
{
	EntityGolem::readEntityFromNBT(tag);
	setPlayerCreated(tag->getBoolean("PlayerCreated"));
}

bool EntityIronGolem::canAttackEntity(EntityLiving *target)
{
	if (target != nullptr && target->isPlayer() && isPlayerCreated())
		return false;
	return EntityGolem::canAttackEntity(target);
}

bool EntityIronGolem::func_48100_a(const std::type_info &type) const
{
	if (isPlayerCreated() && type == typeid(EntityPlayer))
		return false;
	return EntityGolem::func_48100_a(type);
}

bool EntityIronGolem::attackEntityAsMob(Entity *target)
{
	if (target == nullptr || worldObj == nullptr)
		return false;
	attackTimer = 10;
	worldObj->setEntityState(this, (unsigned char)4);
	bool attacked = target->attackEntityFrom(DamageSource::causeMobDamage(this), 7 + rand.nextInt(15));
	if (attacked)
		target->motionY += static_cast<double>(0.4f);
	worldObj->playSoundAtEntity(this, "mob.irongolem.throw", 1.0f, 1.0f);
	return attacked;
}

void EntityIronGolem::handleHealthUpdate(byte_t state)
{
	if (state == 4)
	{
		attackTimer = 10;
		worldObj->playSoundAtEntity(this, "mob.irongolem.throw", 1.0f, 1.0f);
	}
	else if (state == 11)
	{
		holdRoseTick = 400;
	}
	else
	{
		EntityGolem::handleHealthUpdate(state);
	}
}

Village *EntityIronGolem::resolveVillage() const
{
	if (worldObj == nullptr || worldObj->villageCollectionObj == nullptr)
		return nullptr;
	if (hasVillageCenter)
	{
		Village *village = worldObj->villageCollectionObj->findNearestVillage(villageCenterX, villageCenterY, villageCenterZ, 1);
		if (village != nullptr)
			return village;
	}
	return worldObj->villageCollectionObj->findNearestVillage(MathHelper::floor_double(posX),
		MathHelper::floor_double(posY), MathHelper::floor_double(posZ), 32);
}

Village *EntityIronGolem::getVillage() const
{
	return resolveVillage();
}

int_t EntityIronGolem::getAttackTimer() const
{
	return attackTimer;
}

int_t EntityIronGolem::func_48114_ab() const
{
	return getAttackTimer();
}

void EntityIronGolem::setHoldingRose(bool value)
{
	holdRoseTick = value ? 400 : 0;
	if (worldObj != nullptr)
		worldObj->setEntityState(this, (unsigned char)11);
}

int_t EntityIronGolem::getHoldRoseTick() const
{
	return holdRoseTick;
}

int_t EntityIronGolem::func_48117_D_() const
{
	return getHoldRoseTick();
}

bool EntityIronGolem::isPlayerCreated() const
{
	return (dataWatcher->getWatchableObjectByte(16) & 1) != 0;
}

bool EntityIronGolem::func_48112_E_() const
{
	return isPlayerCreated();
}

void EntityIronGolem::func_48115_b(bool value)
{
	setPlayerCreated(value);
}

void EntityIronGolem::setPlayerCreated(bool value)
{
	byte_t flags = dataWatcher->getWatchableObjectByte(16);
	dataWatcher->updateObject(16, (byte_t)(value ? (flags | 1) : (flags & ~1)));
}

jstring EntityIronGolem::getLivingSound()
{
	return "none";
}

jstring EntityIronGolem::getHurtSound()
{
	return "mob.irongolem.hit";
}

jstring EntityIronGolem::getDeathSound()
{
	return "mob.irongolem.death";
}

void EntityIronGolem::playStepSound(int_t x, int_t y, int_t z, int_t blockId)
{
	(void)x;
	(void)y;
	(void)z;
	(void)blockId;
	if (worldObj != nullptr)
		worldObj->playSoundAtEntity(this, "mob.irongolem.walk", 1.0f, 1.0f);
}

void EntityIronGolem::dropFewItems(bool, int_t lootingLevel)
{
	int_t flowers = rand.nextInt(3);
	for (int_t i = 0; i < flowers; ++i)
		if (Block::plantRed != nullptr)
			dropItem(Block::plantRed->blockID, 1);
	int_t iron = 3 + rand.nextInt(3);
	for (int_t i = 0; i < iron; ++i)
		if (Item::ingotIron != nullptr)
			dropItem(Item::ingotIron->shiftedIndex, 1);
}
