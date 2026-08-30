#pragma once

#include "RenderLiving.h"

class EntityMagmaCube;

// net.minecraft.src.RenderMagmaCube
class RenderMagmaCube : public RenderLiving
{
public:
    RenderMagmaCube();
    void doRenderLiving(EntityLiving* entity, double x, double y, double z, float yaw, float partialTick) override;
    void doRender(Entity* entity, double x, double y, double z, float yaw, float partialTick) override;

protected:
    void preRenderCallback(EntityLiving* entity, float partialTick) override;

private:
    void renderMagmaCube(EntityMagmaCube* magmaCube, double x, double y, double z, float yaw, float partialTick);
    int modelVersion;
};
