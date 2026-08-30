#include "RenderVillager.h"

#include "EntityVillager.h"
#include "ModelVillager.h"
#include "platform/RenderAPI.h"

RenderVillager::RenderVillager() : RenderLiving(new ModelVillager(0.0f), 0.5f)
{
    villagerModel = static_cast<ModelVillager*>(mainModel);
}

void RenderVillager::renderVillager(EntityVillager* villager, double x, double y, double z, float yaw, float partialTick)
{
    RenderLiving::doRenderLiving(villager, x, y, z, yaw, partialTick);
}

void RenderVillager::preRenderCallback(EntityLiving* entity, float partialTick)
{
    EntityVillager* villager = static_cast<EntityVillager*>(entity);
    float scale = 15.0f / 16.0f;
    if (villager->getGrowingAge() < 0)
    {
        scale *= 0.5f;
        shadowSize = 0.25f;
    }
    else
    {
        shadowSize = 0.5f;
    }
    renderScale(scale, scale, scale);
}

int RenderVillager::shouldRenderPass(EntityLiving* entity, int pass, float partialTick)
{
    return -1;
}

void RenderVillager::renderEquippedItems(EntityLiving* entity, float partialTick)
{
    RenderLiving::renderEquippedItems(entity, partialTick);
}

void RenderVillager::passSpecialRender(EntityLiving* entity, double x, double y, double z)
{
}

void RenderVillager::doRenderLiving(EntityLiving* entity, double x, double y, double z, float yaw, float partialTick)
{
    renderVillager(static_cast<EntityVillager*>(entity), x, y, z, yaw, partialTick);
}

void RenderVillager::doRender(Entity* entity, double x, double y, double z, float yaw, float partialTick)
{
    renderVillager(static_cast<EntityVillager*>(entity), x, y, z, yaw, partialTick);
}
