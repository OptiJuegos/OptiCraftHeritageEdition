#pragma once

#include "EntityCow.h"

// net.minecraft.src.EntityMooshroom
class EntityMooshroom : public EntityCow
{
public:
	explicit EntityMooshroom(World *world);

	bool interact(EntityPlayer *player) override;
	EntityAnimal *spawnBabyAnimal(EntityAnimal *mate) override;
};
