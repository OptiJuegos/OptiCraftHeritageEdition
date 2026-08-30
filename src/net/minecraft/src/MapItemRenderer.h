#pragma once

#include "java/Type.h"
#include <vector>

class FontRenderer;
class GameSettings;
class RenderEngine;
class MapData;
class EntityPlayer;
class Tessellator;

// net.minecraft.src.MapItemRenderer
class MapItemRenderer
{
public:
	MapItemRenderer(FontRenderer *fontrenderer, GameSettings *gamesettings, RenderEngine *renderengine);

	void renderMap(EntityPlayer *entityplayer, RenderEngine *renderengine, MapData *mapdata);

private:
	int_t buffer[16384];
	int_t textureId;
	GameSettings *gameSettings;
	FontRenderer *fontRenderer;
};
