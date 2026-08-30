#pragma once

#include "DamageSource.h"

// net.minecraft.src.EntityDamageSource
class EntityDamageSource : public DamageSource
{
public:
	EntityDamageSource(const jstring &type, Entity *entity);
	Entity *getEntity() const override;

protected:
	Entity *damageSourceEntity;
};
