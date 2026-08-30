#pragma once

#include "RenderLiving.h"
#include "java/Random.h"

class EntityEnderman;
class ModelEnderman;

// net.minecraft.src.RenderEnderman
class RenderEnderman : public RenderLiving
{
public:
	RenderEnderman();
	void doRender(Entity *entity, double x, double y, double z, float yaw, float partialTick) override;
	void doRenderLiving(EntityLiving *entity, double x, double y, double z, float yaw, float partialTick) override;

protected:
	void renderEquippedItems(EntityLiving *entity, float partialTick) override;
	int shouldRenderPass(EntityLiving *entity, int_t pass, float partialTick) override;

private:
	void renderEnderman(EntityEnderman *enderman, double x, double y, double z, float yaw, float partialTick);
	void renderCarrying(EntityEnderman *enderman, float partialTick);
	int renderEyes(EntityEnderman *enderman, int_t pass, float partialTick);

	ModelEnderman *endermanModel;
	Random rnd;
};
