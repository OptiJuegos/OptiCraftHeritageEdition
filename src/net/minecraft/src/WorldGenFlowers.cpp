#include "WorldGenFlowers.h"

#include "Block.h"
#include "BlockFlower.h"
#include "World.h"

WorldGenFlowers::WorldGenFlowers(int_t i)
{
	plantBlockId = i;
}

bool WorldGenFlowers::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	for (int_t l = 0; l < 64; l++)
	{
		int_t i1 = random.nextIntOffset(i, 8);
		int_t j1 = random.nextIntOffset(j, 4);
		int_t k1 = random.nextIntOffset(k, 8);
		BlockFlower *flower = dynamic_cast<BlockFlower *>(Block::blocksList[plantBlockId]);
		if (world->isAirBlock(i1, j1, k1) && flower && flower->canBlockStay(world, i1, j1, k1))
			world->setBlock(i1, j1, k1, plantBlockId);
	}
	return true;
}
