#include "RenderSnowball.h"

#include "EntityPotion.h"
#include "PotionHelper.h"
#include "Tessellator.h"
#include "platform/RenderAPI.h"

RenderSnowball::RenderSnowball(int_t iconIndex) : itemIconIndex(iconIndex) {}

void RenderSnowball::doRender(Entity *entity, double x, double y, double z, float, float)
{
	renderPushMatrix();
	renderTranslate((float)x, (float)y, (float)z);
	renderEnable(RenderCapability::RescaleNormal);
	renderScale(0.5f, 0.5f, 0.5f);
	loadTexture("/gui/items.png");

	if (itemIconIndex == 154)
	{
		EntityPotion *potion = dynamic_cast<EntityPotion *>(entity);
		if (potion != nullptr)
		{
			int_t color = PotionHelper::func_40358_a(potion->getPotionDamage(), false);
			float red = (float)(color >> 16 & 255) / 255.0f;
			float green = (float)(color >> 8 & 255) / 255.0f;
			float blue = (float)(color & 255) / 255.0f;
			renderColor3f(red, green, blue);
			renderPushMatrix();
			renderIcon(141);
			renderPopMatrix();
			renderColor3f(1.0f, 1.0f, 1.0f);
		}
	}

	renderIcon(itemIconIndex);
	renderDisable(RenderCapability::RescaleNormal);
	renderPopMatrix();
}

void RenderSnowball::renderIcon(int_t iconIndex)
{
	float u0 = (float)((iconIndex % 16) * 16) / 256.0f;
	float u1 = (float)((iconIndex % 16) * 16 + 16) / 256.0f;
	float v0 = (float)((iconIndex / 16) * 16) / 256.0f;
	float v1 = (float)((iconIndex / 16) * 16 + 16) / 256.0f;
	float size = 1.0f;
	float half = 0.5f;
	float yOffset = 0.25f;
	renderRotate(180.0f - renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
	renderRotate(-renderManager->playerViewX, 1.0f, 0.0f, 0.0f);
	Tessellator *tessellator = &Tessellator::instance;
	tessellator->startDrawingQuads();
	tessellator->setNormal(0.0f, 1.0f, 0.0f);
	tessellator->addVertexWithUV(0.0f - half, 0.0f - yOffset, 0.0f, u0, v1);
	tessellator->addVertexWithUV(size - half, 0.0f - yOffset, 0.0f, u1, v1);
	tessellator->addVertexWithUV(size - half, size - yOffset, 0.0f, u1, v0);
	tessellator->addVertexWithUV(0.0f - half, size - yOffset, 0.0f, u0, v0);
	tessellator->draw();
}
