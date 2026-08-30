#include "EntityFootStepFX.h"

#include "platform/RenderAPI.h"

#include "MathHelper.h"
#include "RenderEngine.h"
#include "Tessellator.h"
#include "World.h"

EntityFootStepFX::EntityFootStepFX(RenderEngine *renderengine, World *world, double d, double d1, double d2)
	: EntityFX(world, d, d1, d2, 0.0, 0.0, 0.0)
{
	field_27018_a = 0;
	field_27020_o = 0;
	field_27019_p = renderengine;
	motionX = motionY = motionZ = 0.0;
	field_27020_o = 200;
}

void EntityFootStepFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	float f6 = ((float)field_27018_a + f) / (float)field_27020_o;
	f6 *= f6;
	float f7 = 2.0f - f6 * 2.0f;
	if (f7 > 1.0f)
	{
		f7 = 1.0f;
	}
	f7 *= 0.2f;
	renderDisable(RenderCapability::Lighting);
	float f8 = 0.125f;
	float f9  = (float)(posX - interpPosX);
	float f10 = (float)(posY - interpPosY);
	float f11 = (float)(posZ - interpPosZ);
	float f12 = worldObj->getLightBrightness(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ));
	field_27019_p->bindTexture(field_27019_p->getTexture("/misc/footprint.png"));
	renderEnable(RenderCapability::Blend);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
	tessellator->startDrawingQuads();
	tessellator->setColorRGBA_F(f12, f12, f12, f7);
	tessellator->addVertexWithUV(f9 - f8, f10, f11 + f8, 0.0f, 1.0f);
	tessellator->addVertexWithUV(f9 + f8, f10, f11 + f8, 1.0f, 1.0f);
	tessellator->addVertexWithUV(f9 + f8, f10, f11 - f8, 1.0f, 0.0f);
	tessellator->addVertexWithUV(f9 - f8, f10, f11 - f8, 0.0f, 0.0f);
	tessellator->draw();
	renderDisable(RenderCapability::Blend);
	renderEnable(RenderCapability::Lighting);
}

void EntityFootStepFX::onUpdate()
{
	field_27018_a++;
	if (field_27018_a == field_27020_o)
	{
		setEntityDead();
	}
}

int_t EntityFootStepFX::getFXLayer()
{
	return 3;
}
