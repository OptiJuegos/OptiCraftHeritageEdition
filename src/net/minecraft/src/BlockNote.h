#pragma once

#include "BlockContainer.h"

class TileEntityNote;
class EntityPlayer;

// net.minecraft.src.BlockNote
class BlockNote : public BlockContainer
{
public:
	BlockNote(int_t i);
	int_t getBlockTextureFromSide(int_t i) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
	void onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
	void playBlock(World *world, int_t i, int_t j, int_t k, int_t l, int_t i1);

protected:
	TileEntity *getBlockEntity() override;
};
