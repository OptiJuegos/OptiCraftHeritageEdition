#include "EntityAINearestAttackableTargetSorter.h"

#include "Entity.h"

EntityAINearestAttackableTargetSorter::EntityAINearestAttackableTargetSorter(Entity *owner) : theEntity(owner)
{
}

int EntityAINearestAttackableTargetSorter::compare(Entity *left, Entity *right) const
{
	if (theEntity == nullptr || left == right)
		return 0;
	double leftDistance = left != nullptr ? theEntity->getDistanceSqToEntity(left) : 0.0;
	double rightDistance = right != nullptr ? theEntity->getDistanceSqToEntity(right) : 0.0;
	return leftDistance < rightDistance ? -1 : (leftDistance > rightDistance ? 1 : 0);
}

int EntityAINearestAttackableTargetSorter::func_48469_a(Entity *left, Entity *right) const
{
	return compare(left, right);
}

bool EntityAINearestAttackableTargetSorter::operator()(Entity *left, Entity *right) const
{
	return compare(left, right) < 0;
}
