#include "RenderIronGolem.h"

#include <cmath>

#include "Block.h"
#include "BlockFlower.h"
#include "EntityIronGolem.h"
#include "ModelIronGolem.h"
#include "ModelRenderer.h"
#include "OpenGlHelper.h"
#include "platform/RenderAPI.h"

RenderIronGolem::RenderIronGolem()
	: RenderLiving(new ModelIronGolem(), 0.5f), ironGolemModel(static_cast<ModelIronGolem *>(mainModel))
{
}

void RenderIronGolem::rotateCorpse(EntityLiving *entity, float age, float yaw, float partialTick)
{
	RenderLiving::rotateCorpse(entity, age, yaw, partialTick);
	EntityIronGolem *golem = dynamic_cast<EntityIronGolem *>(entity);
	if (golem == nullptr || golem->field_704_R < 0.01f)
		return;

	const float period = 13.0f;
	float phase = golem->field_703_S - golem->field_704_R * (1.0f - partialTick) + 6.0f;
	float wave = (std::fabs(std::fmod(phase, period) - period * 0.5f) - period * 0.25f) / (period * 0.25f);
	renderRotate(6.5f * wave, 0.0f, 0.0f, 1.0f);
}

void RenderIronGolem::renderEquippedItems(EntityLiving *entity, float partialTick)
{
	RenderLiving::renderEquippedItems(entity, partialTick);
	EntityIronGolem *golem = dynamic_cast<EntityIronGolem *>(entity);
	if (golem == nullptr || golem->getHoldRoseTick() == 0)
		return;

	renderEnable(RenderCapability::RescaleNormal);
	renderPushMatrix();
	renderRotate(5.0f + 180.0f * ironGolemModel->field_48233_c->rotateAngleX / 3.14159265358979323846f,
	             1.0f, 0.0f, 0.0f);
	renderTranslate(-(11.0f / 16.0f), 1.25f, -(15.0f / 16.0f));
	renderRotate(90.0f, 1.0f, 0.0f, 0.0f);
	const float scale = 0.8f;
	renderScale(scale, -scale, scale);
	int_t brightness = golem->getBrightnessForRender(partialTick);
	OpenGlHelper::setLightmapTextureCoords(OpenGlHelper::lightmapTexUnit,
	                                       (float)(brightness & 0xffff),
	                                       (float)((brightness >> 16) & 0xffff));
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	loadTexture("/terrain.png");
	renderBlocks->renderBlockAsItem(Block::plantRed, 0, 1.0f);
	renderPopMatrix();
	renderDisable(RenderCapability::RescaleNormal);
}

void RenderIronGolem::doRenderLiving(EntityLiving *entity, double x, double y, double z, float yaw, float partialTick)
{
	RenderLiving::doRenderLiving(entity, x, y, z, yaw, partialTick);
}

void RenderIronGolem::doRender(Entity *entity, double x, double y, double z, float yaw, float partialTick)
{
	doRenderLiving(dynamic_cast<EntityLiving *>(entity), x, y, z, yaw, partialTick);
}
