#pragma once

#include "Render.h"

class EntityXPOrb;

// net.minecraft.src.RenderXPOrb
class RenderXPOrb : public Render
{
public:
	RenderXPOrb();
	void doRender(Entity *entity, double x, double y, double z, float yaw, float partialTick) override;

private:
	void renderXPOrb(EntityXPOrb *orb, double x, double y, double z, float yaw, float partialTick);
};
