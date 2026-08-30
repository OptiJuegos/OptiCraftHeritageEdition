#include "WorldGenCactus.h"

#include "Block.h"
#include "World.h"
#include "java/Arithmetic.h"

bool WorldGenCactus::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	for (int_t l = 0; l < 10; l++)
	{
		int_t i1 = random.nextIntOffset(i, 8);
		int_t j1 = random.nextIntOffset(j, 4);
		int_t k1 = random.nextIntOffset(k, 8);
		if (!world->isAirBlock(i1, j1, k1))
			continue;
		int_t l1 = 1 + random.nextInt(random.nextInt(3) + 1);
		for (int_t i2 = 0; i2 < l1; i2++)
		{
			const int_t blockY = JavaArithmetic::intAdd(j1, i2);
			if (Block::cactus->canBlockStay(world, i1, blockY, k1))
				world->setBlock(i1, blockY, k1, Block::cactus->blockID);
		}
	}
	return true;
}
