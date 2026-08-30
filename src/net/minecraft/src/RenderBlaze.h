#pragma once

#include "RenderLiving.h"

class EntityBlaze;

// net.minecraft.src.RenderBlaze
class RenderBlaze : public RenderLiving
{
public:
    RenderBlaze();
    void doRenderLiving(EntityLiving* entity, double x, double y, double z, float yaw, float partialTick) override;
    void doRender(Entity* entity, double x, double y, double z, float yaw, float partialTick) override;

private:
    void renderBlaze(EntityBlaze* blaze, double x, double y, double z, float yaw, float partialTick);
    int modelVersion;
};
