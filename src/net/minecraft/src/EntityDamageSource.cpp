#include "EntityDamageSource.h"

EntityDamageSource::EntityDamageSource(const jstring &type, Entity *entity)
	: DamageSource(type, entity, entity), damageSourceEntity(entity)
{
}

Entity *EntityDamageSource::getEntity() const
{
	return damageSourceEntity;
}
