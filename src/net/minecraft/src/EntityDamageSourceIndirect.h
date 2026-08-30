#pragma once

#include "EntityDamageSource.h"

// net.minecraft.src.EntityDamageSourceIndirect
class EntityDamageSourceIndirect : public EntityDamageSource
{
public:
	EntityDamageSourceIndirect(const jstring &type, Entity *source, Entity *indirectEntity);
	Entity *getSourceOfDamage() const override;
	Entity *getEntity() const override;

private:
	Entity *indirectEntity;
};
