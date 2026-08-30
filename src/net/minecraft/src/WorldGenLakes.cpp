#include "WorldGenLakes.h"

#include "Block.h"
#include "BlockGrass.h"
#include "BlockMycelium.h"
#include "BiomeGenBase.h"
#include "EnumSkyBlock.h"
#include "Material.h"
#include "World.h"
#include "java/Arithmetic.h"
#include "platform/PlatformTuning.h"


#if PLATFORM_FLOAT_FEATURE_GENERATION
using LakeGenReal = float;
#define LAKE_R(v) static_cast<LakeGenReal>(v##f)
#else
using LakeGenReal = double;
#define LAKE_R(v) static_cast<LakeGenReal>(v)
#endif

WorldGenLakes::WorldGenLakes(int_t i)
{
	field_15235_a = i;
}

bool WorldGenLakes::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	i = JavaArithmetic::intSub(i, 8);
	k = JavaArithmetic::intSub(k, 8);
	for (; j > 5 && world->isAirBlock(i, j, k); j = JavaArithmetic::intSub(j, 1))
	{
	}
	if (j <= 4)
		return false;
	j = JavaArithmetic::intSub(j, 4);
	bool aflag[2048] = {};
	int_t l = random.nextInt(4) + 4;
#if PLATFORM_FLOAT_FEATURE_GENERATION
	auto nextLakeReal = [&random]() -> LakeGenReal { return random.nextDoubleFloat(); };
#else
	auto nextLakeReal = [&random]() -> LakeGenReal { return random.nextDouble(); };
#endif
	for (int_t i1 = 0; i1 < l; i1++)
	{
		LakeGenReal d = nextLakeReal() * LAKE_R(6.0) + LAKE_R(3.0);
		LakeGenReal d1 = nextLakeReal() * LAKE_R(4.0) + LAKE_R(2.0);
		LakeGenReal d2 = nextLakeReal() * LAKE_R(6.0) + LAKE_R(3.0);
		LakeGenReal d3 = nextLakeReal() * (LAKE_R(16.0) - d - LAKE_R(2.0)) + LAKE_R(1.0) + d / LAKE_R(2.0);
		LakeGenReal d4 = nextLakeReal() * (LAKE_R(8.0) - d1 - LAKE_R(4.0)) + LAKE_R(2.0) + d1 / LAKE_R(2.0);
		LakeGenReal d5 = nextLakeReal() * (LAKE_R(16.0) - d2 - LAKE_R(2.0)) + LAKE_R(1.0) + d2 / LAKE_R(2.0);
		for (int_t j4 = 1; j4 < 15; j4++)
		{
			for (int_t k4 = 1; k4 < 15; k4++)
			{
				for (int_t l4 = 1; l4 < 7; l4++)
				{
					LakeGenReal d6 = (static_cast<LakeGenReal>(j4) - d3) / (d / LAKE_R(2.0));
					LakeGenReal d7 = (static_cast<LakeGenReal>(l4) - d4) / (d1 / LAKE_R(2.0));
					LakeGenReal d8 = (static_cast<LakeGenReal>(k4) - d5) / (d2 / LAKE_R(2.0));
					LakeGenReal d9 = d6 * d6 + d7 * d7 + d8 * d8;
					if (d9 < LAKE_R(1.0))
						aflag[(j4 * 16 + k4) * 8 + l4] = true;
				}
			}
		}
	}
	for (int_t j1 = 0; j1 < 16; j1++)
	{
		for (int_t j2 = 0; j2 < 16; j2++)
		{
			for (int_t j3 = 0; j3 < 8; j3++)
			{
				bool flag = !aflag[(j1 * 16 + j2) * 8 + j3] && (j1 < 15 && aflag[((j1 + 1) * 16 + j2) * 8 + j3] || j1 > 0 && aflag[((j1 - 1) * 16 + j2) * 8 + j3] || j2 < 15 && aflag[(j1 * 16 + (j2 + 1)) * 8 + j3] || j2 > 0 && aflag[(j1 * 16 + (j2 - 1)) * 8 + j3] || j3 < 7 && aflag[(j1 * 16 + j2) * 8 + (j3 + 1)] || j3 > 0 && aflag[(j1 * 16 + j2) * 8 + (j3 - 1)]);
				if (!flag)
					continue;
				Material *material = world->getBlockMaterial(JavaArithmetic::intAdd(i, j1), JavaArithmetic::intAdd(j, j3), JavaArithmetic::intAdd(k, j2));
				if (j3 >= 4 && material->getIsLiquid())
					return false;
				if (j3 < 4 && !material->isSolid() && world->getBlockId(JavaArithmetic::intAdd(i, j1), JavaArithmetic::intAdd(j, j3), JavaArithmetic::intAdd(k, j2)) != field_15235_a)
					return false;
			}
		}
	}
	for (int_t k1 = 0; k1 < 16; k1++)
	{
		for (int_t k2 = 0; k2 < 16; k2++)
		{
			for (int_t k3 = 0; k3 < 8; k3++)
			{
				if (aflag[(k1 * 16 + k2) * 8 + k3])
					world->setBlock(JavaArithmetic::intAdd(i, k1), JavaArithmetic::intAdd(j, k3), JavaArithmetic::intAdd(k, k2), k3 < 4 ? field_15235_a : 0);
			}
		}
	}
	for (int_t l1 = 0; l1 < 16; l1++)
	{
		for (int_t l2 = 0; l2 < 16; l2++)
		{
			for (int_t l3 = 4; l3 < 8; l3++)
			{
				if (aflag[(l1 * 16 + l2) * 8 + l3] && world->getBlockId(JavaArithmetic::intAdd(i, l1), JavaArithmetic::intSub(JavaArithmetic::intAdd(j, l3), 1), JavaArithmetic::intAdd(k, l2)) == Block::dirt->blockID && world->getSavedLightValue(EnumSkyBlock::Sky, JavaArithmetic::intAdd(i, l1), JavaArithmetic::intAdd(j, l3), JavaArithmetic::intAdd(k, l2)) > 0)
				{
					BiomeGenBase *biome = world->getBiomeGenForCoords(JavaArithmetic::intAdd(i, l1), JavaArithmetic::intAdd(k, l2));
					if (biome != nullptr && biome->topBlock == Block::mycelium->blockID)
						world->setBlock(JavaArithmetic::intAdd(i, l1), JavaArithmetic::intSub(JavaArithmetic::intAdd(j, l3), 1), JavaArithmetic::intAdd(k, l2), Block::mycelium->blockID);
					else
						world->setBlock(JavaArithmetic::intAdd(i, l1), JavaArithmetic::intSub(JavaArithmetic::intAdd(j, l3), 1), JavaArithmetic::intAdd(k, l2), Block::grass->blockID);
				}
			}
		}
	}
	if (Block::blocksList[field_15235_a]->blockMaterial == Material::lava)
	{
		for (int_t i2 = 0; i2 < 16; i2++)
		{
			for (int_t i3 = 0; i3 < 16; i3++)
			{
				for (int_t i4 = 0; i4 < 8; i4++)
				{
					bool flag1 = !aflag[(i2 * 16 + i3) * 8 + i4] && (i2 < 15 && aflag[((i2 + 1) * 16 + i3) * 8 + i4] || i2 > 0 && aflag[((i2 - 1) * 16 + i3) * 8 + i4] || i3 < 15 && aflag[(i2 * 16 + (i3 + 1)) * 8 + i4] || i3 > 0 && aflag[(i2 * 16 + (i3 - 1)) * 8 + i4] || i4 < 7 && aflag[(i2 * 16 + i3) * 8 + (i4 + 1)] || i4 > 0 && aflag[(i2 * 16 + i3) * 8 + (i4 - 1)]);
					if (flag1 && (i4 < 4 || random.nextInt(2) != 0) && world->getBlockMaterial(JavaArithmetic::intAdd(i, i2), JavaArithmetic::intAdd(j, i4), JavaArithmetic::intAdd(k, i3))->isSolid())
						world->setBlock(JavaArithmetic::intAdd(i, i2), JavaArithmetic::intAdd(j, i4), JavaArithmetic::intAdd(k, i3), Block::stone->blockID);
				}
			}
		}
	}
	if (Block::blocksList[field_15235_a]->blockMaterial == Material::water)
	{
		for (int_t x = 0; x < 16; ++x)
		{
			for (int_t z = 0; z < 16; ++z)
			{
				const int_t surfaceY = 4;
				if (world->isBlockHydratedDirectly(JavaArithmetic::intAdd(i, x), JavaArithmetic::intAdd(j, surfaceY), JavaArithmetic::intAdd(k, z)))
					world->setBlock(JavaArithmetic::intAdd(i, x), JavaArithmetic::intAdd(j, surfaceY), JavaArithmetic::intAdd(k, z), Block::ice->blockID);
			}
		}
	}
	return true;
}
