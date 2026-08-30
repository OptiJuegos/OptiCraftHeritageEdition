#pragma once

#include "Block.h"

class EntityPlayer;
class Entity;

// net.minecraft.src.BlockRedstoneOre
class BlockRedstoneOre : public Block
{
public:
	BlockRedstoneOre(int_t i, int_t j, bool flag);
	int_t tickRate() override;
	void onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
	void onEntityWalking(World *world, int_t i, int_t j, int_t k, Entity *entity) override;
	bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	int_t idDropped(int_t i, Random &random) override;
	int_t quantityDroppedWithBonus(int_t fortune, Random &random) override;
	int_t quantityDropped(Random &random) override;
	void randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random) override;

protected:
	ItemStack *createStackedBlock(int_t metadata) override;

private:
	void getGlowingState(World *world, int_t i, int_t j, int_t k);
	void getLit(World *world, int_t i, int_t j, int_t k);
	bool shiftedIndex;
};
