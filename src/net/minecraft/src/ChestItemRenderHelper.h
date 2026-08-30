#pragma once

class Block;
class TileEntityChest;

// net.minecraft.src.ChestItemRenderHelper
class ChestItemRenderHelper
{
public:
	ChestItemRenderHelper();
	~ChestItemRenderHelper();

	void renderChest(Block *block, int metadata, float brightness);

	static ChestItemRenderHelper instance;

private:
	TileEntityChest *chest;
};
