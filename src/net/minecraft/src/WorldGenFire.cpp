#include "WorldGenFire.h"

#include "java/Arithmetic.h"
#include "Block.h"
#include "BlockFire.h"
#include "World.h"

bool WorldGenFire::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	for (int_t l = 0; l < 64; l++)
	{
		int_t i1 = random.nextIntOffset(i, 8);
		int_t j1 = random.nextIntOffset(j, 4);
		int_t k1 = random.nextIntOffset(k, 8);
		if (world->isAirBlock(i1, j1, k1) && world->getBlockId(i1, JavaArithmetic::intSub(j1, 1), k1) == Block::netherrack->blockID)
			world->setBlockWithNotify(i1, j1, k1, Block::fire->blockID);
	}
	return true;
}
