#pragma once

#include "RenderLiving.h"

class EntityOcelot;
class ModelBase;

// net.minecraft.src.RenderOcelot
class RenderOcelot : public RenderLiving
{
public:
	RenderOcelot(ModelBase *model, float shadowSize);

	void doRenderLiving(EntityLiving *entity, double x, double y, double z, float yaw, float partialTicks) override;
	void doRender(Entity *entity, double x, double y, double z, float yaw, float partialTicks) override;

protected:
	void preRenderCallback(EntityLiving *entity, float partialTicks) override;
};
