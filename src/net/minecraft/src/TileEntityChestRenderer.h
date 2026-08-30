#pragma once

#include "TileEntitySpecialRenderer.h"

class ModelChest;
class TileEntityChest;

// net.minecraft.src.TileEntityChestRenderer
class TileEntityChestRenderer : public TileEntitySpecialRenderer
{
public:
	TileEntityChestRenderer();
	~TileEntityChestRenderer() override;

	void renderTileEntityChestAt(TileEntityChest *chest, double x, double y, double z, float partialTick);
	void renderTileEntityAt(TileEntity *tileentity, double x, double y, double z, float partialTick) override;

private:
	ModelChest *chestModel;
	ModelChest *largeChestModel;
};
