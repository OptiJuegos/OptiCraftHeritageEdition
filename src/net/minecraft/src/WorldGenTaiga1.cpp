#include "WorldGenTaiga1.h"

#include "java/Arithmetic.h"
#include <cstdlib>
#include "Block.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "World.h"

bool WorldGenTaiga1::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	int_t l = JavaArithmetic::intAdd(random.nextInt(5), 7);
	int_t i1 = JavaArithmetic::intSub(JavaArithmetic::intSub(l, random.nextInt(2)), 3);
	int_t j1 = JavaArithmetic::intSub(l, i1);
	int_t k1 = JavaArithmetic::intAdd(1, random.nextInt(JavaArithmetic::intAdd(j1, 1)));
	bool flag = true;
	if (j < 1 || JavaArithmetic::intAdd(JavaArithmetic::intAdd(j, l), 1) > 128)
		return false;
	for (int_t l1 = j; l1 <= JavaArithmetic::intAdd(JavaArithmetic::intAdd(j, 1), l) && flag; l1++)
	{
		int_t j2 = 1;
		if (JavaArithmetic::intSub(l1, j) < i1)
			j2 = 0;
		else
			j2 = k1;
		for (int_t l2 = JavaArithmetic::intSub(i, j2); l2 <= JavaArithmetic::intAdd(i, j2) && flag; l2++)
		{
			for (int_t k3 = JavaArithmetic::intSub(k, j2); k3 <= JavaArithmetic::intAdd(k, j2) && flag; k3++)
			{
				if (l1 >= 0 && l1 < 128)
				{
					int_t j4 = world->getBlockId(l2, l1, k3);
					if (j4 != 0 && j4 != Block::leaves->blockID)
						flag = false;
				}
				else
				{
					flag = false;
				}
			}
		}
	}
	if (!flag)
		return false;
	int_t i2 = world->getBlockId(i, JavaArithmetic::intSub(j, 1), k);
	if (i2 != Block::grass->blockID && i2 != Block::dirt->blockID || j >= JavaArithmetic::intSub(JavaArithmetic::intSub(128, l), 1))
		return false;
	world->setBlock(i, JavaArithmetic::intSub(j, 1), k, Block::dirt->blockID);
	int_t k2 = 0;
	for (int_t i3 = JavaArithmetic::intAdd(j, l); i3 >= j + i1; i3--)
	{
		for (int_t l3 = JavaArithmetic::intSub(i, k2); l3 <= JavaArithmetic::intAdd(i, k2); l3++)
		{
			int_t k4 = JavaArithmetic::intSub(l3, i);
			for (int_t l4 = JavaArithmetic::intSub(k, k2); l4 <= JavaArithmetic::intAdd(k, k2); l4++)
			{
				int_t i5 = JavaArithmetic::intSub(l4, k);
				if ((JavaArithmetic::intAbs(k4) != k2 || JavaArithmetic::intAbs(i5) != k2 || k2 <= 0) && !Block::opaqueCubeLookup[world->getBlockId(l3, i3, l4)])
					world->setBlockAndMetadata(l3, i3, l4, Block::leaves->blockID, 1);
			}
		}
		if (k2 >= 1 && i3 == JavaArithmetic::intAdd(JavaArithmetic::intAdd(j, i1), 1))
		{
			k2--;
			continue;
		}
		if (k2 < k1)
			k2++;
	}
	for (int_t j3 = 0; j3 < JavaArithmetic::intSub(l, 1); j3++)
	{
		int_t i4 = world->getBlockId(i, JavaArithmetic::intAdd(j, j3), k);
		if (i4 == 0 || i4 == Block::leaves->blockID)
			world->setBlockAndMetadata(i, JavaArithmetic::intAdd(j, j3), k, Block::wood->blockID, 1);
	}
	return true;
}
