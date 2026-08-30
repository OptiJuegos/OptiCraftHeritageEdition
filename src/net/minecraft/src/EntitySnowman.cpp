#include "EntitySnowman.h"

#include <typeinfo>

#include "BiomeGenBase.h"
#include "Block.h"
#include "DamageSource.h"
#include "EntityAIArrowAttack.h"
#include "EntityAILookIdle.h"
#include "EntityAINearestAttackableTarget.h"
#include "EntityAIWander.h"
#include "EntityAIWatchClosest.h"
#include "EntityMob.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "PathNavigate.h"
#include "World.h"

EntitySnowman::EntitySnowman(World *world)
	: EntityGolem(world)
{
	texture = "/mob/snowman.png";
	setSize(0.4f, 1.8f);
	if (getNavigator() != nullptr)
		getNavigator()->setAvoidsWater(true);
	tasks.addTask(1, new EntityAIArrowAttack(this, 0.25f, 2, 20));
	tasks.addTask(2, new EntityAIWander(this, 0.2f));
	tasks.addTask(3, new EntityAIWatchClosest(this, typeid(EntityPlayer), 6.0f));
	tasks.addTask(4, new EntityAILookIdle(this));
	targetTasks.addTask(1, new EntityAINearestAttackableTarget(this, typeid(EntityMob), 16.0f, 0, true));
	setHealth(getMaxHealth());
}

bool EntitySnowman::isAIEnabled()
{
	return true;
}

int_t EntitySnowman::getMaxHealth() const
{
	return 4;
}

void EntitySnowman::onLivingUpdate()
{
	EntityGolem::onLivingUpdate();
	if (isWet())
		attackEntityFrom(DamageSource::drown, 1);

	int_t x = MathHelper::floor_double(posX);
	int_t z = MathHelper::floor_double(posZ);
	BiomeGenBase *biome = worldObj->getBiomeGenForCoords(x, z);
	if (biome != nullptr && biome->getFloatTemperature() > 1.0f)
		attackEntityFrom(DamageSource::onFire, 1);

	for (int_t i = 0; i < 4; ++i)
	{
		x = MathHelper::floor_double(posX + static_cast<double>(static_cast<float>(i % 2 * 2 - 1) * 0.25f));
		const int_t y = MathHelper::floor_double(posY);
		z = MathHelper::floor_double(posZ + static_cast<double>(static_cast<float>(i / 2 % 2 * 2 - 1) * 0.25f));
		biome = worldObj->getBiomeGenForCoords(x, z);
		if (worldObj->getBlockId(x, y, z) == 0 && biome != nullptr && biome->getFloatTemperature() < 0.8f &&
			Block::snow != nullptr && Block::snow->canPlaceBlockAt(worldObj, x, y, z))
		{
			worldObj->setBlockWithNotify(x, y, z, Block::snow->blockID);
		}
	}
}

void EntitySnowman::writeEntityToNBT(NBTTagCompound *tag)
{
	EntityGolem::writeEntityToNBT(tag);
}

void EntitySnowman::readEntityFromNBT(NBTTagCompound *tag)
{
	EntityGolem::readEntityFromNBT(tag);
}

int_t EntitySnowman::getDropItemId()
{
	return Item::snowball != nullptr ? Item::snowball->shiftedIndex : 0;
}

void EntitySnowman::dropFewItems(bool, int_t lootingLevel)
{
	if (Item::snowball == nullptr)
		return;
	const int_t count = rand.nextInt(16);
	for (int_t i = 0; i < count; ++i)
		dropItem(Item::snowball->shiftedIndex, 1);
}
