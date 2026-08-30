#pragma once

#include "BlockContainer.h"

class EntityPlayer;
class Random;
class TileEntity;
class World;

// net.minecraft.src.BlockEnchantmentTable
class BlockEnchantmentTable : public BlockContainer
{
public:
	explicit BlockEnchantmentTable(int_t id);

	bool renderAsNormalBlock() override;
	void randomDisplayTick(World *world, int_t x, int_t y, int_t z, Random &random) override;
	bool isOpaqueCube() override;
	int_t getBlockTextureFromSideAndMetadata(int_t side, int_t metadata) override;
	int_t getBlockTextureFromSide(int_t side) override;
	bool blockActivated(World *world, int_t x, int_t y, int_t z, EntityPlayer *player) override;

protected:
	TileEntity *getBlockEntity() override;
};
