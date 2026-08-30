#include "WorldGenGlowStone1.h"

#include "java/Arithmetic.h"
#include "Block.h"
#include "World.h"

bool WorldGenGlowStone1::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	if (!world->isAirBlock(i, j, k))
		return false;
	if (world->getBlockId(i, JavaArithmetic::intAdd(j, 1), k) != Block::netherrack->blockID)
		return false;
	world->setBlockWithNotify(i, j, k, Block::glowStone->blockID);
	for (int_t l = 0; l < 1500; l++)
	{
		int_t i1 = random.nextIntOffset(i, 8);
		int_t j1 = JavaArithmetic::intSub(j, random.nextInt(12));
		int_t k1 = random.nextIntOffset(k, 8);
		if (world->getBlockId(i1, j1, k1) != 0)
			continue;
		int_t l1 = 0;
		for (int_t i2 = 0; i2 < 6; i2++)
		{
			int_t j2 = 0;
			if (i2 == 0) j2 = world->getBlockId(JavaArithmetic::intSub(i1, 1), j1, k1);
			if (i2 == 1) j2 = world->getBlockId(JavaArithmetic::intAdd(i1, 1), j1, k1);
			if (i2 == 2) j2 = world->getBlockId(i1, JavaArithmetic::intSub(j1, 1), k1);
			if (i2 == 3) j2 = world->getBlockId(i1, JavaArithmetic::intAdd(j1, 1), k1);
			if (i2 == 4) j2 = world->getBlockId(i1, j1, JavaArithmetic::intSub(k1, 1));
			if (i2 == 5) j2 = world->getBlockId(i1, j1, JavaArithmetic::intAdd(k1, 1));
			if (j2 == Block::glowStone->blockID)
				l1++;
		}
		if (l1 == 1)
			world->setBlockWithNotify(i1, j1, k1, Block::glowStone->blockID);
	}
	return true;
}
