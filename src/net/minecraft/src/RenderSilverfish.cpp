#include "RenderSilverfish.h"

#include "EntitySilverfish.h"
#include "ModelSilverfish.h"

RenderSilverfish::RenderSilverfish() : RenderLiving(new ModelSilverfish(), 0.3f)
{
}

float RenderSilverfish::getDeathMaxRotation(EntityLiving* entity)
{
    return 180.0f;
}

int RenderSilverfish::shouldRenderPass(EntityLiving* entity, int pass, float partialTick)
{
    return -1;
}

void RenderSilverfish::doRenderLiving(EntityLiving* entity, double x, double y, double z, float yaw, float partialTick)
{
    RenderLiving::doRenderLiving(static_cast<EntitySilverfish*>(entity), x, y, z, yaw, partialTick);
}

void RenderSilverfish::doRender(Entity* entity, double x, double y, double z, float yaw, float partialTick)
{
    RenderLiving::doRenderLiving(static_cast<EntitySilverfish*>(entity), x, y, z, yaw, partialTick);
}
