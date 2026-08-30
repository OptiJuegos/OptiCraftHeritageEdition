#include "RenderWolf.h"
#include "EntityWolf.h"

RenderWolf::RenderWolf(ModelBase *modelBase, float shadowSize) : RenderLiving(modelBase, shadowSize) {}

void RenderWolf::renderWolf(EntityWolf *wolf, double x, double y, double z, float yaw, float partialTicks)
{
	RenderLiving::doRenderLiving(wolf, x, y, z, yaw, partialTicks);
}

void RenderWolf::doRenderLiving(EntityLiving *entity, double x, double y, double z, float yaw, float partialTicks)
{
	renderWolf(static_cast<EntityWolf *>(entity), x, y, z, yaw, partialTicks);
}

void RenderWolf::doRender(Entity *entity, double x, double y, double z, float yaw, float partialTicks)
{
	renderWolf(static_cast<EntityWolf *>(entity), x, y, z, yaw, partialTicks);
}

float RenderWolf::getTicksExisted(EntityLiving *entity, float)
{
	return static_cast<EntityWolf *>(entity)->setTailRotation();
}

void RenderWolf::preRenderCallback(EntityLiving *, float) {}
