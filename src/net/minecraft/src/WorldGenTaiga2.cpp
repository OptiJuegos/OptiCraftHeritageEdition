#include "WorldGenTaiga2.h"

#include "java/Arithmetic.h"
#include <cstdlib>
#include "Block.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "World.h"
WorldGenTaiga2::WorldGenTaiga2()
    : WorldGenTaiga2(false)
{
}

WorldGenTaiga2::WorldGenTaiga2(bool notify)
    : WorldGenerator(notify)
{
}

bool WorldGenTaiga2::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	int_t l = JavaArithmetic::intAdd(random.nextInt(4), 6);
	int_t i1 = JavaArithmetic::intAdd(1, random.nextInt(2));
	int_t j1 = JavaArithmetic::intSub(l, i1);
	int_t k1 = JavaArithmetic::intAdd(2, random.nextInt(2));
	bool flag = true;
	if (j < 1 || JavaArithmetic::intAdd(JavaArithmetic::intAdd(j, l), 1) > 256)
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
			for (int_t j3 = JavaArithmetic::intSub(k, j2); j3 <= JavaArithmetic::intAdd(k, j2) && flag; j3++)
			{
				if (l1 >= 0 && l1 < 256)
				{
					int_t k3 = world->getBlockId(l2, l1, j3);
					if (k3 != 0 && k3 != Block::leaves->blockID)
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
	if (i2 != Block::grass->blockID && i2 != Block::dirt->blockID || j >= JavaArithmetic::intSub(JavaArithmetic::intSub(256, l), 1))
		return false;
	setBlock(world, i, JavaArithmetic::intSub(j, 1), k, Block::dirt->blockID);
	int_t k2 = random.nextInt(2);
	int_t i3 = 1;
	bool flag1 = false;
	for (int_t l3 = 0; l3 <= j1; l3++)
	{
		int_t j4 = JavaArithmetic::intSub(JavaArithmetic::intAdd(j, l), l3);
		for (int_t l4 = JavaArithmetic::intSub(i, k2); l4 <= JavaArithmetic::intAdd(i, k2); l4++)
		{
			int_t j5 = JavaArithmetic::intSub(l4, i);
			for (int_t k5 = JavaArithmetic::intSub(k, k2); k5 <= JavaArithmetic::intAdd(k, k2); k5++)
			{
				int_t l5 = JavaArithmetic::intSub(k5, k);
				if ((JavaArithmetic::intAbs(j5) != k2 || JavaArithmetic::intAbs(l5) != k2 || k2 <= 0) && !Block::opaqueCubeLookup[world->getBlockId(l4, j4, k5)])
					setBlockAndMetadata(world, l4, j4, k5, Block::leaves->blockID, 1);
			}
		}
		if (k2 >= i3)
		{
			k2 = flag1 ? 1 : 0;
			flag1 = true;
			if (++i3 > k1)
				i3 = k1;
		}
		else
		{
			k2++;
		}
	}
	int_t i4 = random.nextInt(3);
	for (int_t k4 = 0; k4 < JavaArithmetic::intSub(l, i4); k4++)
	{
		int_t i5 = world->getBlockId(i, JavaArithmetic::intAdd(j, k4), k);
		if (i5 == 0 || i5 == Block::leaves->blockID)
			setBlockAndMetadata(world, i, JavaArithmetic::intAdd(j, k4), k, Block::wood->blockID, 1);
	}
	return true;
}
