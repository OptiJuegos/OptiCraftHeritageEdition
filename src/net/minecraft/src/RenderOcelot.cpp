#include "RenderOcelot.h"

#include "EntityOcelot.h"
#include "platform/RenderAPI.h"

RenderOcelot::RenderOcelot(ModelBase *model, float size)
	: RenderLiving(model, size)
{
}

void RenderOcelot::doRenderLiving(EntityLiving *entity, double x, double y, double z, float yaw, float partialTicks)
{
	RenderLiving::doRenderLiving(entity, x, y, z, yaw, partialTicks);
}

void RenderOcelot::doRender(Entity *entity, double x, double y, double z, float yaw, float partialTicks)
{
	doRenderLiving(static_cast<EntityOcelot *>(entity), x, y, z, yaw, partialTicks);
}

void RenderOcelot::preRenderCallback(EntityLiving *entity, float)
{
	EntityOcelot *ocelot = dynamic_cast<EntityOcelot *>(entity);
	if (ocelot != nullptr && ocelot->isTamed())
		renderScale(0.8f, 0.8f, 0.8f);
}
