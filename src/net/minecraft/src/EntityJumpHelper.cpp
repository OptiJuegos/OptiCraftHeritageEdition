#include "EntityJumpHelper.h"

#include "EntityLiving.h"

EntityJumpHelper::EntityJumpHelper(EntityLiving *entity) : entity(entity), jumping(false)
{
}

void EntityJumpHelper::setJumping()
{
	jumping = true;
}

void EntityJumpHelper::doJump()
{
	if (entity != nullptr)
		entity->setJumping(jumping);
	jumping = false;
}
