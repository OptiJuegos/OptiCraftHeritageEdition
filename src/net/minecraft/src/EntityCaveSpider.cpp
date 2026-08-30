#include "EntityCaveSpider.h"

#include "EntityLiving.h"
#include "Potion.h"
#include "PotionEffect.h"
#include "World.h"

EntityCaveSpider::EntityCaveSpider(World *world)
	: EntitySpider(world)
{
	texture = "/mob/cavespider.png";
	setSize(0.7f, 0.5f);
	setHealth(getMaxHealth());
}

int_t EntityCaveSpider::getMaxHealth() const
{
	return 12;
}

float EntityCaveSpider::spiderScaleAmount()
{
	return 0.7f;
}

bool EntityCaveSpider::attackEntityAsMob(Entity *target)
{
	if (!EntitySpider::attackEntityAsMob(target))
		return false;

	EntityLiving *living = dynamic_cast<EntityLiving *>(target);
	if (living == nullptr)
		return true;

	int_t durationSeconds = 0;
	if (worldObj->difficultySetting > 1)
	{
		if (worldObj->difficultySetting == 2)
			durationSeconds = 7;
		else if (worldObj->difficultySetting == 3)
			durationSeconds = 15;
	}

	if (durationSeconds > 0)
	{
		Potion::initPotions();
		if (Potion::poison != nullptr)
			living->addPotionEffect(new PotionEffect(Potion::poison->id, durationSeconds * 20, 0));
	}

	return true;
}
