#include "RenderBlaze.h"

#include "EntityBlaze.h"
#include "ModelBlaze.h"

RenderBlaze::RenderBlaze() : RenderLiving(new ModelBlaze(), 0.5f)
{
    modelVersion = static_cast<ModelBlaze*>(mainModel)->func_40321_a();
}

void RenderBlaze::renderBlaze(EntityBlaze* blaze, double x, double y, double z, float yaw, float partialTick)
{
    const int currentVersion = static_cast<ModelBlaze*>(mainModel)->func_40321_a();
    if (currentVersion != modelVersion)
    {
        modelVersion = currentVersion;
        delete mainModel;
        mainModel = new ModelBlaze();
    }
    RenderLiving::doRenderLiving(blaze, x, y, z, yaw, partialTick);
}

void RenderBlaze::doRenderLiving(EntityLiving* entity, double x, double y, double z, float yaw, float partialTick)
{
    renderBlaze(static_cast<EntityBlaze*>(entity), x, y, z, yaw, partialTick);
}

void RenderBlaze::doRender(Entity* entity, double x, double y, double z, float yaw, float partialTick)
{
    renderBlaze(static_cast<EntityBlaze*>(entity), x, y, z, yaw, partialTick);
}
