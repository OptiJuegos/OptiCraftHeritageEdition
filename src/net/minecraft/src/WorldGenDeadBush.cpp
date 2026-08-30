#include "WorldGenDeadBush.h"

#include "Block.h"
#include "BlockFlower.h"
#include "BlockLeaves.h"
#include "World.h"

WorldGenDeadBush::WorldGenDeadBush(int_t i)
{
	field_28058_a = i;
}

bool WorldGenDeadBush::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	for (int_t l = 0; ((l = world->getBlockId(i, j, k)) == 0 || l == Block::leaves->blockID) && j > 0; j--)
	{
	}
	for (int_t i1 = 0; i1 < 4; i1++)
	{
		int_t j1 = random.nextIntOffset(i, 8);
		int_t k1 = random.nextIntOffset(j, 4);
		int_t l1 = random.nextIntOffset(k, 8);
		BlockFlower *flower = dynamic_cast<BlockFlower *>(Block::blocksList[field_28058_a]);
		if (world->isAirBlock(j1, k1, l1) && flower && flower->canBlockStay(world, j1, k1, l1))
			world->setBlock(j1, k1, l1, field_28058_a);
	}
	return true;
}
