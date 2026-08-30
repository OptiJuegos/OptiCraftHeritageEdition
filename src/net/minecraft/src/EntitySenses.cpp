#include "EntitySenses.h"

#include <algorithm>

#include "Entity.h"
#include "EntityLiving.h"

EntitySenses::EntitySenses(EntityLiving *entity) : entity(entity)
{
}

void EntitySenses::clearSensingCache()
{
	canSeeCachePositive.clear();
	canSeeCacheNegative.clear();
}

bool EntitySenses::canSee(Entity *target)
{
	if (target == nullptr || entity == nullptr)
		return false;
	if (std::find(canSeeCachePositive.begin(), canSeeCachePositive.end(), target) != canSeeCachePositive.end())
		return true;
	if (std::find(canSeeCacheNegative.begin(), canSeeCacheNegative.end(), target) != canSeeCacheNegative.end())
		return false;
	bool visible = entity->canEntityBeSeen(target);
	(visible ? canSeeCachePositive : canSeeCacheNegative).push_back(target);
	return visible;
}
