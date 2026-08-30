#pragma once

#include "java/Type.h"

class Random;

// net.minecraft.src.StructurePieceBlockSelector
class StructurePieceBlockSelector
{
public:
	virtual ~StructurePieceBlockSelector() = default;
	virtual void selectBlocks(Random &random, int_t x, int_t y, int_t z, bool isBoundary) = 0;

	int_t getSelectedBlockId() const;
	int_t getSelectedBlockMetaData() const;

protected:
	int_t selectedBlockId = 0;
	int_t selectedBlockMetaData = 0;
};
