#pragma once

class Entity;

// net.minecraft.src.EntityAINearestAttackableTargetSorter
class EntityAINearestAttackableTargetSorter
{
public:
	explicit EntityAINearestAttackableTargetSorter(Entity *owner);
	int compare(Entity *left, Entity *right) const;
	int func_48469_a(Entity *left, Entity *right) const;
	bool operator()(Entity *left, Entity *right) const;

private:
	Entity *theEntity;
};
