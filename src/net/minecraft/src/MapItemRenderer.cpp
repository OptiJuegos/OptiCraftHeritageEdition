#include "MapItemRenderer.h"
#include "java/Arithmetic.h"

#include "platform/RenderAPI.h"
#include "MapData.h"
#include "MapColor.h"
#include "MapCoord.h"
#include "GameSettings.h"
#include "RenderEngine.h"
#include "Tessellator.h"
#include "FontRenderer.h"
#include "EntityPlayer.h"
#include "java/BufferedImage.h"

MapItemRenderer::MapItemRenderer(FontRenderer *fontrenderer, GameSettings *gamesettings, RenderEngine *renderengine) :
	textureId(0),
	gameSettings(gamesettings),
	fontRenderer(fontrenderer)
{
	BufferedImage mapTexture(128, 128);
	textureId = renderengine->allocateAndSetupTexture(&mapTexture);
	for (int_t i = 0; i < 16384; i++)
		buffer[i] = 0;
}

void MapItemRenderer::renderMap(EntityPlayer *entityplayer, RenderEngine *renderengine, MapData *mapdata)
{
	for (int_t i = 0; i < 16384; i++)
	{
		byte_t byte0 = mapdata->colors[i];
		if (byte0 / 4 == 0)
		{
			buffer[i] = JavaArithmetic::intShl(((i + i / 128 & 1) * 8 + 16), 24);
			continue;
		}
		int_t l = MapColor::mapColorArray[byte0 / 4]->colorValue;
		int_t i1 = byte0 & 3;
		int_t c = 220;
		if (i1 == 2) c = 255;
		if (i1 == 0) c = 180;
		int_t j1 = ((l >> 16) & 0xff) * c / 255;
		int_t k1 = ((l >> 8) & 0xff) * c / 255;
		int_t l1 = (l & 0xff) * c / 255;
		if (gameSettings->anaglyph)
		{
			int_t i2 = (j1 * 30 + k1 * 59 + l1 * 11) / 100;
			int_t j2 = (j1 * 30 + k1 * 70) / 100;
			int_t k2 = (j1 * 30 + l1 * 70) / 100;
			j1 = i2; k1 = j2; l1 = k2;
		}
		buffer[i] = 0xff000000 | (j1 << 16) | (k1 << 8) | l1;
	}

	int_t j = 0;
	int_t k = 0;
	Tessellator *tessellator = &Tessellator::instance;
	float f = 0.0f;
	renderengine->bindTexture(textureId);
	std::vector<unsigned char> rgba(128 * 128 * 4);
	for (int_t pixel = 0; pixel < 128 * 128; ++pixel)
	{
		int_t color = buffer[pixel];
		rgba[pixel * 4 + 0] = (unsigned char)((color >> 16) & 0xff);
		rgba[pixel * 4 + 1] = (unsigned char)((color >> 8) & 0xff);
		rgba[pixel * 4 + 2] = (unsigned char)(color & 0xff);
		rgba[pixel * 4 + 3] = (unsigned char)((color >> 24) & 0xff);
	}
	renderTextureSubImageRgba(0, 0, 0, 128, 128, rgba.data());
	renderEnable(RenderCapability::Blend);
	renderBlendFunc(RenderBlendFactor::One, RenderBlendFactor::OneMinusSrcAlpha);
	renderDisable(RenderCapability::AlphaTest);
	tessellator->startDrawingQuads();
	tessellator->setColorRGBA_F(f, f, f, 1.0f);
	tessellator->addVertexWithUV(j + 0, k + 128, 0.0, 0.0, 1.0);
	tessellator->addVertexWithUV(j + 128, k + 128, 0.0, 1.0, 1.0);
	tessellator->addVertexWithUV(j + 128, k + 0, 0.0, 1.0, 0.0);
	tessellator->addVertexWithUV(j + 0, k + 0, 0.0, 0.0, 0.0);
	tessellator->draw();

	renderengine->bindTexture(renderengine->getTexture("/misc/mapicons.png"));
	for (auto *mapcoord : mapdata->playersVisibleOnMap)
	{
		renderPushMatrix();
		renderTranslate((float)j + (float)mapcoord->centerX / 2.0f + 64.0f,
			(float)k + (float)mapcoord->centerZ / 2.0f + 64.0f, -0.02f);
		renderRotate((float)(mapcoord->iconRotation * 360) / 16.0f, 0.0f, 0.0f, 1.0f);
		renderScale(4.0f, 4.0f, 3.0f);
		renderTranslate(-0.125f, 0.125f, 0.0f);

		float f1 = (float)(mapcoord->iconSize % 4 + 0) / 4.0f;
		float f2 = (float)(mapcoord->iconSize / 4 + 0) / 4.0f;
		float f3 = (float)(mapcoord->iconSize % 4 + 1) / 4.0f;
		float f4 = (float)(mapcoord->iconSize / 4 + 1) / 4.0f;

		tessellator->startDrawingQuads();
		tessellator->addVertexWithUV(-1.0, 1.0, 0.0, f1, f2);
		tessellator->addVertexWithUV(1.0, 1.0, 0.0, f3, f2);
		tessellator->addVertexWithUV(1.0, -1.0, 0.0, f3, f4);
		tessellator->addVertexWithUV(-1.0, -1.0, 0.0, f1, f4);
		tessellator->draw();
		renderPopMatrix();
	}

	renderPushMatrix();
	renderTranslate(0.0f, 0.0f, -0.04f);
	renderScale(1.0f, 1.0f, 1.0f);
	renderPopMatrix();

	renderEnable(RenderCapability::AlphaTest);
	renderDisable(RenderCapability::Blend);
}
