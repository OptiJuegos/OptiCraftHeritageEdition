#pragma once

#include "EntitySpider.h"

// net.minecraft.src.EntityCaveSpider
class EntityCaveSpider : public EntitySpider
{
public:
	explicit EntityCaveSpider(World *world);

	int_t getMaxHealth() const override;
	float spiderScaleAmount() override;
	bool attackEntityAsMob(Entity *target) override;
};
