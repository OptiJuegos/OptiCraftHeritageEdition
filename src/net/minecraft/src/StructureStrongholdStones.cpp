#include "StructureStrongholdStones.h"

#include "Block.h"
#include "java/Random.h"

void StructureStrongholdStones::selectBlocks(Random &random, int_t, int_t, int_t, bool isBoundary)
{
	if (!isBoundary)
	{
		selectedBlockId = 0;
		selectedBlockMetaData = 0;
		return;
	}

	selectedBlockId = Block::stoneBrick->blockID;
	const float value = random.nextFloat();
	if (value < 0.2F)
		selectedBlockMetaData = 2;
	else if (value < 0.5F)
		selectedBlockMetaData = 1;
	else if (value < 0.55F)
	{
		selectedBlockId = Block::silverfish->blockID;
		selectedBlockMetaData = 2;
	}
	else
		selectedBlockMetaData = 0;
}
