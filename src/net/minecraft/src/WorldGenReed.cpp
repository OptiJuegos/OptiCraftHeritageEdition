#include "WorldGenReed.h"

#include "Block.h"
#include "Material.h"
#include "World.h"
#include "java/Arithmetic.h"

bool WorldGenReed::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	for (int_t l = 0; l < 20; l++)
	{
		int_t i1 = random.nextIntOffset(i, 4);
		int_t j1 = j;
		int_t k1 = random.nextIntOffset(k, 4);
		const int_t belowY = JavaArithmetic::intSub(j1, 1);
		if (!world->isAirBlock(i1, j1, k1) ||
		    world->getBlockMaterial(JavaArithmetic::intSub(i1, 1), belowY, k1) != Material::water &&
		    world->getBlockMaterial(JavaArithmetic::intAdd(i1, 1), belowY, k1) != Material::water &&
		    world->getBlockMaterial(i1, belowY, JavaArithmetic::intSub(k1, 1)) != Material::water &&
		    world->getBlockMaterial(i1, belowY, JavaArithmetic::intAdd(k1, 1)) != Material::water)
			continue;
		int_t l1 = 2 + random.nextInt(random.nextInt(3) + 1);
		for (int_t i2 = 0; i2 < l1; i2++)
		{
			const int_t blockY = JavaArithmetic::intAdd(j1, i2);
			if (Block::reed->canBlockStay(world, i1, blockY, k1))
				world->setBlock(i1, blockY, k1, Block::reed->blockID);
		}
	}
	return true;
}
