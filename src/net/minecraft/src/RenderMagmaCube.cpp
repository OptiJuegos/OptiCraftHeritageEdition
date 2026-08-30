#include "RenderMagmaCube.h"

#include "EntityMagmaCube.h"
#include "ModelMagmaCube.h"
#include "platform/RenderAPI.h"

RenderMagmaCube::RenderMagmaCube() : RenderLiving(new ModelMagmaCube(), 0.25f)
{
    modelVersion = static_cast<ModelMagmaCube*>(mainModel)->func_40343_a();
}

void RenderMagmaCube::renderMagmaCube(EntityMagmaCube* magmaCube, double x, double y, double z, float yaw, float partialTick)
{
    const int currentVersion = static_cast<ModelMagmaCube*>(mainModel)->func_40343_a();
    if (currentVersion != modelVersion)
    {
        modelVersion = currentVersion;
        delete mainModel;
        mainModel = new ModelMagmaCube();
    }
    RenderLiving::doRenderLiving(magmaCube, x, y, z, yaw, partialTick);
}

void RenderMagmaCube::preRenderCallback(EntityLiving* entity, float partialTick)
{
    EntityMagmaCube* magmaCube = static_cast<EntityMagmaCube*>(entity);
    const int size = magmaCube->getSlimeSize();
    const float squish = (magmaCube->field_767_b + (magmaCube->field_768_a - magmaCube->field_767_b) * partialTick)
        / (static_cast<float>(size) * 0.5f + 1.0f);
    const float inverse = 1.0f / (squish + 1.0f);
    const float scale = static_cast<float>(size);
    renderScale(inverse * scale, scale / inverse, inverse * scale);
}

void RenderMagmaCube::doRenderLiving(EntityLiving* entity, double x, double y, double z, float yaw, float partialTick)
{
    renderMagmaCube(static_cast<EntityMagmaCube*>(entity), x, y, z, yaw, partialTick);
}

void RenderMagmaCube::doRender(Entity* entity, double x, double y, double z, float yaw, float partialTick)
{
    renderMagmaCube(static_cast<EntityMagmaCube*>(entity), x, y, z, yaw, partialTick);
}
