#pragma once

#include "TileEntitySpecialRenderer.h"

class ModelBook;
class TileEntityEnchantmentTable;

// net.minecraft.src.RenderEnchantmentTable
class RenderEnchantmentTable : public TileEntitySpecialRenderer
{
public:
	RenderEnchantmentTable();
	~RenderEnchantmentTable() override;

	void renderEnchantmentTableAt(TileEntityEnchantmentTable *table, double x, double y, double z, float partialTick);
	void renderTileEntityAt(TileEntity *tileentity, double x, double y, double z, float partialTick) override;

private:
	ModelBook *bookModel;
};
