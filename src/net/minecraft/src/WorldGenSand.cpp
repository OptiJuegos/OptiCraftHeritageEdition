#include "WorldGenSand.h"

#include "Block.h"
#include "BlockGrass.h"
#include "Material.h"
#include "World.h"
#include "java/Arithmetic.h"
#include "platform/PlatformTuning.h"

WorldGenSand::WorldGenSand(int_t radiusValue, int_t sandId)
    : sandID(sandId),
      radius(radiusValue)
{
}

bool WorldGenSand::generate(World *world, Random &random, int_t x, int_t y, int_t z)
{
    if (world->getBlockMaterial(x, y, z) != Material::water)
        return false;

	const int_t generatedRadius = JavaArithmetic::intAdd(random.nextInt(JavaArithmetic::intSub(radius, 2)), 2);
	const int_t radiusSq = JavaArithmetic::intMul(generatedRadius, generatedRadius);
    constexpr int_t verticalRadius = 2;

	for (int_t blockX = JavaArithmetic::intSub(x, generatedRadius);
	     blockX <= JavaArithmetic::intAdd(x, generatedRadius);
	     blockX = JavaArithmetic::intAdd(blockX, 1))
    {
		for (int_t blockZ = JavaArithmetic::intSub(z, generatedRadius);
		     blockZ <= JavaArithmetic::intAdd(z, generatedRadius);
		     blockZ = JavaArithmetic::intAdd(blockZ, 1))
        {
			const int_t offsetX = JavaArithmetic::intSub(blockX, x);
			const int_t offsetZ = JavaArithmetic::intSub(blockZ, z);
			const int_t distanceSq = JavaArithmetic::intAdd(
			    JavaArithmetic::intMul(offsetX, offsetX), JavaArithmetic::intMul(offsetZ, offsetZ));
			if (distanceSq > radiusSq)
                continue;

			for (int_t blockY = JavaArithmetic::intSub(y, verticalRadius);
			     blockY <= JavaArithmetic::intAdd(y, verticalRadius);
			     blockY = JavaArithmetic::intAdd(blockY, 1))
            {
                const int_t blockId = world->getBlockIdForPopulation(blockX, blockY, blockZ);
                if (blockId == Block::dirt->blockID || blockId == Block::grass->blockID)
                {
#if PLATFORM_BOUNDED_WORLD
                    world->replaceBlockForPopulation(blockX, blockY, blockZ, blockId, sandID);
#else
                    world->setBlock(blockX, blockY, blockZ, sandID);
#endif
                }
            }
        }
    }

    return true;
}
