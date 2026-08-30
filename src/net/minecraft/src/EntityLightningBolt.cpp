#include "EntityLightningBolt.h"

#include <vector>

#include "AxisAlignedBB.h"
#include "Block.h"
#include "BlockFire.h"
#include "Entity.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "Vec3D.h"
#include "World.h"

EntityLightningBolt::EntityLightningBolt(World *world, double d, double d1, double d2)
	: EntityWeatherEffect(world)
{
	ensureEntityInit();
	field_27029_a = 0LL;
	setLocationAndAngles(d, d1, d2, 0.0f, 0.0f);
	field_27028_b = 2;
	field_27029_a = rand.nextLong();
	field_27030_c = rand.nextInt(3) + 1;
	if (world->difficultySetting >= 2 && world->doChunksNearChunkExist(MathHelper::floor_double(d), MathHelper::floor_double(d1), MathHelper::floor_double(d2), 10))
	{
		int_t i  = MathHelper::floor_double(d);
		int_t k  = MathHelper::floor_double(d1);
		int_t i1 = MathHelper::floor_double(d2);
		if (world->getBlockId(i, k, i1) == 0 && Block::fire->canPlaceBlockAt(world, i, k, i1))
		{
			world->setBlockWithNotify(i, k, i1, Block::fire->blockID);
		}
		for (int_t j = 0; j < 4; j++)
		{
			int_t l  = (MathHelper::floor_double(d)  + rand.nextInt(3)) - 1;
			int_t j1 = (MathHelper::floor_double(d1) + rand.nextInt(3)) - 1;
			int_t k1 = (MathHelper::floor_double(d2) + rand.nextInt(3)) - 1;
			if (world->getBlockId(l, j1, k1) == 0 && Block::fire->canPlaceBlockAt(world, l, j1, k1))
			{
				world->setBlockWithNotify(l, j1, k1, Block::fire->blockID);
			}
		}
	}
}

void EntityLightningBolt::onUpdate()
{
	Entity::onUpdate();
	if (field_27028_b == 2)
	{
		worldObj->playSoundEffect(posX, posY, posZ, "ambient.weather.thunder", 10000.0f, 0.8f + rand.nextFloat() * 0.2f);
		worldObj->playSoundEffect(posX, posY, posZ, "random.explode", 2.0f, 0.5f + rand.nextFloat() * 0.2f);
	}
	field_27028_b--;
	if (field_27028_b < 0)
	{
		if (field_27030_c == 0)
		{
			setEntityDead();
		}
		else if (field_27028_b < -rand.nextInt(10))
		{
			field_27030_c--;
			field_27028_b = 1;
			field_27029_a = rand.nextLong();
			if (worldObj->doChunksNearChunkExist(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ), 10))
			{
				int_t i = MathHelper::floor_double(posX);
				int_t j = MathHelper::floor_double(posY);
				int_t k = MathHelper::floor_double(posZ);
				if (worldObj->getBlockId(i, j, k) == 0 && Block::fire->canPlaceBlockAt(worldObj, i, j, k))
				{
					worldObj->setBlockWithNotify(i, j, k, Block::fire->blockID);
				}
			}
		}
	}
	if (field_27028_b >= 0)
	{
		double d = 3.0;
		const std::vector<Entity *> list = worldObj->getEntitiesWithinAABBExcludingEntity(this, AxisAlignedBB::getBoundingBoxFromPool(posX - d, posY - d, posZ - d, posX + d, posY + 6.0 + d, posZ + d));
		for (size_t l = 0; l < list.size(); l++)
		{
			Entity *entity = list[l];
			entity->onStruckByLightning(this);
		}
		worldObj->field_27172_i = 2;
	}
}

void EntityLightningBolt::entityInit()
{
}

void EntityLightningBolt::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
}

void EntityLightningBolt::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
}

bool EntityLightningBolt::isInRangeToRenderVec3D(Vec3D *vec3d)
{
	return field_27028_b >= 0;
}
