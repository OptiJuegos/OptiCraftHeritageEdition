#include "RenderXPOrb.h"

#include "EntityXPOrb.h"
#include "MathHelper.h"
#include "OpenGlHelper.h"
#include "Tessellator.h"
#include "platform/RenderAPI.h"

RenderXPOrb::RenderXPOrb()
{
	shadowSize = 0.15f;
	shadowOpaque = 12.0f / 16.0f;
}

void RenderXPOrb::renderXPOrb(EntityXPOrb *orb, double x, double y, double z, float, float partialTick)
{
	if (orb == nullptr || renderManager == nullptr)
		return;

	renderPushMatrix();
	renderTranslate((float)x, (float)y, (float)z);
	int_t texture = orb->getTextureByXP();
	loadTexture("/item/xporb.png");
	Tessellator &tessellator = Tessellator::instance;
	float u0 = (float)(texture % 4 * 16) / 64.0f;
	float u1 = (float)(texture % 4 * 16 + 16) / 64.0f;
	float v0 = (float)(texture / 4 * 16) / 64.0f;
	float v1 = (float)(texture / 4 * 16 + 16) / 64.0f;

	int_t brightness = orb->getBrightnessForRender(partialTick);
	OpenGlHelper::setLightmapTextureCoords(OpenGlHelper::lightmapTexUnit,
	                                       (float)(brightness & 0xffff),
	                                       (float)((brightness >> 16) & 0xffff));
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	float phase = ((float)orb->xpColor + partialTick) / 2.0f;
	int_t red = (int_t)((MathHelper::sin(phase) + 1.0f) * 0.5f * 255.0f);
	int_t green = 255;
	int_t blue = (int_t)((MathHelper::sin(phase + 3.14159265358979323846f * 4.0f / 3.0f) + 1.0f) * 0.1f * 255.0f);
	int_t color = red << 16 | green << 8 | blue;

	renderRotate(180.0f - renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
	renderRotate(-renderManager->playerViewX, 1.0f, 0.0f, 0.0f);
	renderScale(0.3f, 0.3f, 0.3f);
	tessellator.startDrawingQuads();
	tessellator.setColorRGBA_I(color, 128);
	tessellator.setNormal(0.0f, 1.0f, 0.0f);
	tessellator.addVertexWithUV(-0.5f, -0.25f, 0.0f, u0, v1);
	tessellator.addVertexWithUV(0.5f, -0.25f, 0.0f, u1, v1);
	tessellator.addVertexWithUV(0.5f, 0.75f, 0.0f, u1, v0);
	tessellator.addVertexWithUV(-0.5f, 0.75f, 0.0f, u0, v0);
	tessellator.draw();
	renderDisable(RenderCapability::Blend);
	renderDisable(RenderCapability::RescaleNormal);
	renderPopMatrix();
}

void RenderXPOrb::doRender(Entity *entity, double x, double y, double z, float yaw, float partialTick)
{
	renderXPOrb(dynamic_cast<EntityXPOrb *>(entity), x, y, z, yaw, partialTick);
}
