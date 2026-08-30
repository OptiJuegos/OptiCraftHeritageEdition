#pragma once

#include "EntityLiving.h"

class World;

// net.minecraft.src.EntityFlying
class EntityFlying : public EntityLiving
{
public:
	EntityFlying(World *world) : EntityLiving(world) {}

protected:
	void fall(float f) override {}

public:
	void moveEntityWithHeading(float f, float f1) override;
	bool isOnLadder() override { return false; }
};
