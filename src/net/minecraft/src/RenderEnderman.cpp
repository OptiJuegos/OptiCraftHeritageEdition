#include "RenderEnderman.h"

#include "Block.h"
#include "EntityEnderman.h"
#include "ModelEnderman.h"
#include "OpenGlHelper.h"
#include "platform/RenderAPI.h"

RenderEnderman::RenderEnderman()
	: RenderLiving(new ModelEnderman(), 0.5f), endermanModel(static_cast<ModelEnderman *>(mainModel))
{
	setRenderPassModel(endermanModel);
}

void RenderEnderman::renderEnderman(EntityEnderman *enderman, double x, double y, double z, float yaw, float partialTick)
{
	endermanModel->isCarrying = enderman->getCarried() > 0;
	endermanModel->isAttacking = enderman->isAttacking;
	if (enderman->isAttacking)
	{
		const double jitter = 0.02;
		x += rnd.nextGaussian() * jitter;
		z += rnd.nextGaussian() * jitter;
	}
	RenderLiving::doRenderLiving(enderman, x, y, z, yaw, partialTick);
}

void RenderEnderman::renderCarrying(EntityEnderman *enderman, float partialTick)
{
	RenderLiving::renderEquippedItems(enderman, partialTick);
	int_t carriedId = enderman->getCarried();
	if (carriedId <= 0 || carriedId >= Block::BLOCK_REGISTRY_SIZE || Block::blocksList[carriedId] == nullptr)
		return;

	renderEnable(RenderCapability::RescaleNormal);
	renderPushMatrix();
	float scale = 0.5f;
	renderTranslate(0.0f, 11.0f / 16.0f, -(12.0f / 16.0f));
	renderRotate(20.0f, 1.0f, 0.0f, 0.0f);
	renderRotate(45.0f, 0.0f, 1.0f, 0.0f);
	renderScale(scale, -scale, scale);
	int_t brightness = enderman->getBrightnessForRender(partialTick);
	OpenGlHelper::setLightmapTextureCoords(OpenGlHelper::lightmapTexUnit,
	                                       (float)(brightness & 0xffff),
	                                       (float)((brightness >> 16) & 0xffff));
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	loadTexture("/terrain.png");
	renderBlocks->renderBlockAsItem(Block::blocksList[carriedId], enderman->getCarryingData(), 1.0f);
	renderPopMatrix();
	renderDisable(RenderCapability::RescaleNormal);
}

int RenderEnderman::renderEyes(EntityEnderman *, int_t pass, float)
{
	if (pass != 0)
		return -1;

	loadTexture("/mob/enderman_eyes.png");
	renderEnable(RenderCapability::Blend);
	renderDisable(RenderCapability::AlphaTest);
	renderBlendFunc(RenderBlendFactor::One, RenderBlendFactor::One);
	renderDisable(RenderCapability::Lighting);
	OpenGlHelper::setLightmapTextureCoords(OpenGlHelper::lightmapTexUnit, 240.0f, 240.0f);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderEnable(RenderCapability::Lighting);
	return 1;
}

int RenderEnderman::shouldRenderPass(EntityLiving *entity, int_t pass, float partialTick)
{
	return renderEyes(dynamic_cast<EntityEnderman *>(entity), pass, partialTick);
}

void RenderEnderman::renderEquippedItems(EntityLiving *entity, float partialTick)
{
	renderCarrying(dynamic_cast<EntityEnderman *>(entity), partialTick);
}

void RenderEnderman::doRenderLiving(EntityLiving *entity, double x, double y, double z, float yaw, float partialTick)
{
	renderEnderman(dynamic_cast<EntityEnderman *>(entity), x, y, z, yaw, partialTick);
}

void RenderEnderman::doRender(Entity *entity, double x, double y, double z, float yaw, float partialTick)
{
	renderEnderman(dynamic_cast<EntityEnderman *>(entity), x, y, z, yaw, partialTick);
}
