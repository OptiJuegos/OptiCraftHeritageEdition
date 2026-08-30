#pragma once

#include "Block.h"

// net.minecraft.src.BlockCloth
class BlockCloth : public Block
{
public:
	BlockCloth();
	int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;

	static int_t getColorFromDamage(int_t i);
	static int_t getColorFromItemStack(int_t i);
	// Java 1.2.5 public API aliases.
	static int_t getBlockFromDye(int_t i);
	static int_t getDyeFromBlock(int_t i);

protected:
	int_t damageDropped(int_t i) override;
};
