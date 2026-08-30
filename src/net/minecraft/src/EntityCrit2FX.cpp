#include "EntityCrit2FX.h"

#include "AxisAlignedBB.h"
#include "Entity.h"
#include "World.h"

EntityCrit2FX::EntityCrit2FX(World *world, Entity *entity)
	: EntityCrit2FX(world, entity, "crit")
{
}

EntityCrit2FX::EntityCrit2FX(World *world, Entity *entity, const jstring &name)
	: EntityFX(world,
	           entity != nullptr ? entity->posX : 0.0,
	           entity != nullptr && entity->boundingBox != nullptr
	               ? entity->boundingBox->minY + static_cast<double>(entity->height / 2.0f)
	               : 0.0,
	           entity != nullptr ? entity->posZ : 0.0,
	           entity != nullptr ? entity->motionX : 0.0,
	           entity != nullptr ? entity->motionY : 0.0,
	           entity != nullptr ? entity->motionZ : 0.0),
	  targetEntityId(entity != nullptr ? entity->entityId : -1),
	  currentLife(0), maximumLife(3), particleName(name)
{
	onUpdate();
}

void EntityCrit2FX::renderParticle(Tessellator *, float, float, float, float, float, float)
{
}

void EntityCrit2FX::onUpdate()
{
	Entity *target = targetEntityId >= 0 ? worldObj->getEntityByID(targetEntityId) : nullptr;
	if (target == nullptr || target->boundingBox == nullptr)
	{
		setEntityDead();
		return;
	}

	for (int_t i = 0; i < 16; ++i)
	{
		const double x = static_cast<double>(rand.nextFloat() * 2.0f - 1.0f);
		const double y = static_cast<double>(rand.nextFloat() * 2.0f - 1.0f);
		const double z = static_cast<double>(rand.nextFloat() * 2.0f - 1.0f);
		if (x * x + y * y + z * z <= 1.0)
		{
			const double px = target->posX + x * static_cast<double>(target->width) / 4.0;
			const double py = target->boundingBox->minY + static_cast<double>(target->height / 2.0f) +
			                  y * static_cast<double>(target->height) / 4.0;
			const double pz = target->posZ + z * static_cast<double>(target->width) / 4.0;
			worldObj->spawnParticle(particleName, px, py, pz, x, y + 0.2, z);
		}
	}

	++currentLife;
	if (currentLife >= maximumLife)
		setEntityDead();
}

int_t EntityCrit2FX::getFXLayer()
{
	return 3;
}
