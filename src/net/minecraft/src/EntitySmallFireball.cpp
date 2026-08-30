#include "EntitySmallFireball.h"

#include "Block.h"
#include "BlockFire.h"
#include "DamageSource.h"
#include "EntityLiving.h"
#include "MovingObjectPosition.h"
#include "World.h"

EntitySmallFireball::EntitySmallFireball(World *world)
	: EntityFireball(world)
{
	setSize(5.0f / 16.0f, 5.0f / 16.0f);
}

EntitySmallFireball::EntitySmallFireball(World *world, EntityLiving *shooter,
	double xAcceleration, double yAcceleration, double zAcceleration)
	: EntityFireball(world, shooter, xAcceleration, yAcceleration, zAcceleration)
{
	setSize(5.0f / 16.0f, 5.0f / 16.0f);
}

EntitySmallFireball::EntitySmallFireball(World *world, double x, double y, double z,
	double xAcceleration, double yAcceleration, double zAcceleration)
	: EntityFireball(world, x, y, z, xAcceleration, yAcceleration, zAcceleration)
{
	setSize(5.0f / 16.0f, 5.0f / 16.0f);
}

void EntitySmallFireball::onImpact(MovingObjectPosition *hit)
{
	if (worldObj->multiplayerWorld || hit == nullptr)
		return;
	if (hit->entityHit != nullptr)
	{
		if (!hit->entityHit->isImmuneToFire())
		{
			DamageSource source = DamageSource::causeFireballDamage(this, getShootingEntity());
			if (hit->entityHit->attackEntityFrom(source, 5))
				hit->entityHit->setFire(5);
		}
	}
	else
	{
		int_t x = hit->blockX;
		int_t y = hit->blockY;
		int_t z = hit->blockZ;
		switch (hit->sideHit)
		{
		case 0: --y; break;
		case 1: ++y; break;
		case 2: --z; break;
		case 3: ++z; break;
		case 4: --x; break;
		case 5: ++x; break;
		default: break;
		}
		if (worldObj->isAirBlock(x, y, z))
			worldObj->setBlockWithNotify(x, y, z, Block::fire->blockID);
	}
	setEntityDead();
}

bool EntitySmallFireball::canBeCollidedWith()
{
	return false;
}

bool EntitySmallFireball::attackEntityFrom(Entity *, int_t)
{
	return false;
}

bool EntitySmallFireball::attackEntityFrom(const DamageSource &, int_t)
{
	return false;
}
