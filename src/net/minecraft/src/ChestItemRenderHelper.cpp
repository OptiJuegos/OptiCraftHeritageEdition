#include "ChestItemRenderHelper.h"

#include "TileEntityChest.h"
#include "TileEntityRenderer.h"

ChestItemRenderHelper ChestItemRenderHelper::instance;

ChestItemRenderHelper::ChestItemRenderHelper()
	: chest(new TileEntityChest())
{
}

ChestItemRenderHelper::~ChestItemRenderHelper()
{
	delete chest;
}

void ChestItemRenderHelper::renderChest(Block *, int, float)
{
	TileEntityRenderer::instance.renderTileEntityAt(chest, 0.0, 0.0, 0.0, 0.0f);
}
