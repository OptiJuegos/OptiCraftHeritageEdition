#pragma once

#include "RenderLiving.h"

class EntityIronGolem;
class ModelIronGolem;

// net.minecraft.src.RenderIronGolem
class RenderIronGolem : public RenderLiving
{
public:
	RenderIronGolem();
	void doRender(Entity *entity, double x, double y, double z, float yaw, float partialTick) override;
	void doRenderLiving(EntityLiving *entity, double x, double y, double z, float yaw, float partialTick) override;

protected:
	void rotateCorpse(EntityLiving *entity, float age, float yaw, float partialTick) override;
	void renderEquippedItems(EntityLiving *entity, float partialTick) override;

private:
	ModelIronGolem *ironGolemModel;
};
