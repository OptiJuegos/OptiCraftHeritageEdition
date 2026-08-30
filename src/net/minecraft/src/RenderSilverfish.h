#pragma once

#include "RenderLiving.h"

class EntitySilverfish;

// net.minecraft.src.RenderSilverfish
class RenderSilverfish : public RenderLiving
{
public:
    RenderSilverfish();
    void doRenderLiving(EntityLiving* entity, double x, double y, double z, float yaw, float partialTick) override;
    void doRender(Entity* entity, double x, double y, double z, float yaw, float partialTick) override;

protected:
    float getDeathMaxRotation(EntityLiving* entity) override;
    int shouldRenderPass(EntityLiving* entity, int pass, float partialTick) override;
};
