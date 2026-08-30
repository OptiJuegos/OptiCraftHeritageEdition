#include "WorldGenLiquids.h"

#include "java/Arithmetic.h"
#include "Block.h"
#include "World.h"

WorldGenLiquids::WorldGenLiquids(int_t i)
{
	liquidBlockId = i;
}

bool WorldGenLiquids::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	const int_t stoneId = Block::stone->blockID;
	if (world->getBlockIdForPopulation(i, JavaArithmetic::intAdd(j, 1), k) != stoneId)
		return false;
	if (world->getBlockIdForPopulation(i, JavaArithmetic::intSub(j, 1), k) != stoneId)
		return false;

	const int_t centerId = world->getBlockIdForPopulation(i, j, k);
	if (centerId != 0 && centerId != stoneId)
		return false;

	const int_t westId = world->getBlockIdForPopulation(JavaArithmetic::intSub(i, 1), j, k);
	const int_t eastId = world->getBlockIdForPopulation(JavaArithmetic::intAdd(i, 1), j, k);
	const int_t northId = world->getBlockIdForPopulation(i, j, JavaArithmetic::intSub(k, 1));
	const int_t southId = world->getBlockIdForPopulation(i, j, JavaArithmetic::intAdd(k, 1));

	const int_t stoneNeighbors =
		(westId == stoneId ? 1 : 0) +
		(eastId == stoneId ? 1 : 0) +
		(northId == stoneId ? 1 : 0) +
		(southId == stoneId ? 1 : 0);
	const int_t airNeighbors =
		(westId == 0 ? 1 : 0) +
		(eastId == 0 ? 1 : 0) +
		(northId == 0 ? 1 : 0) +
		(southId == 0 ? 1 : 0);

	if (stoneNeighbors == 3 && airNeighbors == 1)
	{
		world->setBlockWithNotify(i, j, k, liquidBlockId);
		world->scheduledUpdatesAreImmediate = true;
		Block::blocksList[liquidBlockId]->updateTick(world, i, j, k, random);
		world->scheduledUpdatesAreImmediate = false;
	}
	return true;
}
