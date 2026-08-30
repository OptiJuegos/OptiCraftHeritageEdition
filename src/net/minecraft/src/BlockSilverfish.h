#pragma once

#include "Block.h"

// net.minecraft.src.BlockSilverfish
class BlockSilverfish : public Block
{
public:
	explicit BlockSilverfish(int_t id);

	void harvestBlock(World *world, EntityPlayer *entityplayer, int_t x, int_t y, int_t z, int_t metadata) override;
	int_t getBlockTextureFromSideAndMetadata(int_t side, int_t metadata) override;
	void onBlockDestroyedByPlayer(World *world, int_t x, int_t y, int_t z, int_t metadata) override;
	int_t quantityDropped(Random &random) override;
	ItemStack *createStackedBlock(int_t metadata) override;

	static bool getPosingIdByMetadata(int_t blockId);
	static int_t getMetadataForBlockType(int_t blockId);
};
