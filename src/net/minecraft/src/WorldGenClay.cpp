#include "WorldGenClay.h"

#include "Block.h"
#include "Material.h"
#include "World.h"
#include "java/Arithmetic.h"
#include "platform/PlatformTuning.h"

WorldGenClay::WorldGenClay(int_t i)
{
	clayBlockId = Block::blockClay->blockID;
	numberOfBlocks = i;
}

bool WorldGenClay::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	if (world->getBlockMaterial(i, j, k) != Material::water)
		return false;

	// Minecraft 1.2.5 WorldGenClay: choose one horizontal radius and replace
	// dirt/clay in a three-block-high disk around the water position.  This is
	// intentionally not the ore-vein algorithm used by WorldGenMinable; using
	// that algorithm changes both clay shapes and the decorator RNG sequence.
	const int_t radius = JavaArithmetic::intAdd(random.nextInt(JavaArithmetic::intSub(numberOfBlocks, 2)), 2);
	const int_t radiusSq = JavaArithmetic::intMul(radius, radius);
	const int_t verticalRadius = 1;

	for (int_t x = JavaArithmetic::intSub(i, radius);
	     x <= JavaArithmetic::intAdd(i, radius);
	     x = JavaArithmetic::intAdd(x, 1))
	{
		for (int_t z = JavaArithmetic::intSub(k, radius);
		     z <= JavaArithmetic::intAdd(k, radius);
		     z = JavaArithmetic::intAdd(z, 1))
		{
			const int_t dx = JavaArithmetic::intSub(x, i);
			const int_t dz = JavaArithmetic::intSub(z, k);
			const int_t distanceSq = JavaArithmetic::intAdd(
			    JavaArithmetic::intMul(dx, dx), JavaArithmetic::intMul(dz, dz));
			if (distanceSq > radiusSq)
				continue;

			for (int_t y = JavaArithmetic::intSub(j, verticalRadius);
			     y <= JavaArithmetic::intAdd(j, verticalRadius);
			     y = JavaArithmetic::intAdd(y, 1))
			{
				const int_t blockId = world->getBlockIdForPopulation(x, y, z);
				if (blockId == Block::dirt->blockID)
				{
#if PLATFORM_BOUNDED_WORLD
					world->replaceBlockForPopulation(x, y, z, blockId, clayBlockId);
#else
					world->setBlock(x, y, z, clayBlockId);
#endif
				}
			}
		}
	}

	return true;
}
