#include "EntityDamageSourceIndirect.h"

EntityDamageSourceIndirect::EntityDamageSourceIndirect(const jstring &type, Entity *source, Entity *indirect)
	: EntityDamageSource(type, source), indirectEntity(indirect)
{
}

Entity *EntityDamageSourceIndirect::getSourceOfDamage() const
{
	return damageSourceEntity;
}

Entity *EntityDamageSourceIndirect::getEntity() const
{
	return indirectEntity;
}
