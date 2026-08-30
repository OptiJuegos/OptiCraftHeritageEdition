#pragma once

#include <vector>

class Entity;
class EntityLiving;

// net.minecraft.src.EntitySenses
class EntitySenses
{
public:
	explicit EntitySenses(EntityLiving *entity);
	void clearSensingCache();
	bool canSee(Entity *target);

private:
	EntityLiving *entity;
	std::vector<Entity *> canSeeCachePositive;
	std::vector<Entity *> canSeeCacheNegative;
};
