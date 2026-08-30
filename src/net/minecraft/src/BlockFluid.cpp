#include "BlockFluid.h"
#include "java/Math.h"

#include "IBlockAccess.h"
#include "BiomeGenBase.h"
#include "Material.h"
#include "Vec3D.h"
#include "World.h"
#include "java/Random.h"
#include <cmath>

BlockFluid::BlockFluid(int_t i, Material *material) :
	Block(i, (material != Material::lava ? 12 : 14) * 16 + 13, material)
{
	float f = 0.0f;
	float f1 = 0.0f;
	setBlockBounds(0.0f + f1, 0.0f + f, 0.0f + f1, 1.0f + f1, 1.0f + f, 1.0f + f1);
	setTickOnLoad(true);
}

int_t BlockFluid::colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	if (blockMaterial != Material::water)
	{
		return 0xffffff;
	}

	int_t red = 0;
	int_t green = 0;
	int_t blue = 0;
	for (int_t dz = -1; dz <= 1; ++dz)
	{
		for (int_t dx = -1; dx <= 1; ++dx)
		{
			BiomeGenBase *biome = iblockaccess->getBiomeGenForCoords(i + dx, k + dz);
			const int_t color = biome != nullptr ? biome->waterColorMultiplier : 0xffffff;
			red += (color & 0xff0000) >> 16;
			green += (color & 0x00ff00) >> 8;
			blue += color & 0x0000ff;
		}
	}

	return ((red / 9) & 0xff) << 16 | ((green / 9) & 0xff) << 8 | ((blue / 9) & 0xff);
}

float BlockFluid::getPercentAir(int_t i)
{
	if (i >= 8)
	{
		i = 0;
	}
	float f = (float)(i + 1) / 9.0f;
	return f;
}

float BlockFluid::getFluidHeightPercent(int_t i)
{
	return getPercentAir(i);
}

int_t BlockFluid::getBlockTextureFromSide(int_t i)
{
	if (i == 0 || i == 1)
	{
		return blockIndexInTexture;
	}
	else
	{
		return blockIndexInTexture + 1;
	}
}

int_t BlockFluid::getFlowDecay(World *world, int_t i, int_t j, int_t k)
{
	if (world->getBlockMaterial(i, j, k) != blockMaterial)
	{
		return -1;
	}
	else
	{
		return world->getBlockMetadata(i, j, k);
	}
}

int_t BlockFluid::getEffectiveFlowDecay(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	if (iblockaccess->getBlockMaterial(i, j, k) != blockMaterial)
	{
		return -1;
	}
	int_t l = iblockaccess->getBlockMetadata(i, j, k);
	if (l >= 8)
	{
		l = 0;
	}
	return l;
}

bool BlockFluid::renderAsNormalBlock()
{
	return false;
}

bool BlockFluid::isOpaqueCube()
{
	return false;
}

bool BlockFluid::canCollideCheck(int_t i, bool flag)
{
	return flag && i == 0;
}

bool BlockFluid::getBlocksMovement(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	return blockMaterial != Material::lava;
}

int_t BlockFluid::getBlockColor()
{
	return 0xffffff;
}

int_t BlockFluid::getMixedBrightnessForBlock(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	const int_t current = iblockaccess->getLightBrightnessForSkyBlocks(i, j, k, 0);
	const int_t above = iblockaccess->getLightBrightnessForSkyBlocks(i, j + 1, k, 0);
	const int_t currentBlock = current & 255;
	const int_t aboveBlock = above & 255;
	const int_t currentSky = (current >> 16) & 255;
	const int_t aboveSky = (above >> 16) & 255;
	return (currentBlock > aboveBlock ? currentBlock : aboveBlock) |
		((currentSky > aboveSky ? currentSky : aboveSky) << 16);
}

bool BlockFluid::isBlockSolid(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	return getIsBlockSolid(iblockaccess, i, j, k, l);
}

bool BlockFluid::getIsBlockSolid(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	Material *material = iblockaccess->getBlockMaterial(i, j, k);
	if (material == blockMaterial)
	{
		return false;
	}
	if (l == 1)
	{
		return true;
	}
	if (material == Material::ice)
	{
		return false;
	}
	else
	{
		return Block::getIsBlockSolid(iblockaccess, i, j, k, l);
	}
}

bool BlockFluid::shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	Material *material = iblockaccess->getBlockMaterial(i, j, k);
	if (material == blockMaterial)
	{
		return false;
	}
	if (l == 1)
	{
		return true;
	}
	if (material == Material::ice)
	{
		return false;
	}
	else
	{
		return Block::shouldSideBeRendered(iblockaccess, i, j, k, l);
	}
}

AxisAlignedBB *BlockFluid::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	return nullptr;
}

int_t BlockFluid::getRenderType()
{
	return 4;
}

int_t BlockFluid::idDropped(int_t i, Random &random)
{
	return 0;
}

int_t BlockFluid::quantityDropped(Random &random)
{
	return 0;
}

Vec3D *BlockFluid::getFlowVector(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	Vec3D *vec3d = Vec3D::createVector(0.0, 0.0, 0.0);
	int_t l = getEffectiveFlowDecay(iblockaccess, i, j, k);
	for (int_t i1 = 0; i1 < 4; i1++)
	{
		int_t j1 = i;
		int_t k1 = j;
		int_t l1 = k;
		if (i1 == 0)
		{
			j1--;
		}
		if (i1 == 1)
		{
			l1--;
		}
		if (i1 == 2)
		{
			j1++;
		}
		if (i1 == 3)
		{
			l1++;
		}
		int_t i2 = getEffectiveFlowDecay(iblockaccess, j1, k1, l1);
		if (i2 < 0)
		{
			if (iblockaccess->getBlockMaterial(j1, k1, l1)->getIsSolid())
			{
				continue;
			}
			i2 = getEffectiveFlowDecay(iblockaccess, j1, k1 - 1, l1);
			if (i2 >= 0)
			{
				int_t j2 = i2 - (l - 8);
				vec3d = vec3d->addVector((j1 - i) * j2, (k1 - j) * j2, (l1 - k) * j2);
			}
			continue;
		}
		if (i2 >= 0)
		{
			int_t k2 = i2 - l;
			vec3d = vec3d->addVector((j1 - i) * k2, (k1 - j) * k2, (l1 - k) * k2);
		}
	}

	if (iblockaccess->getBlockMetadata(i, j, k) >= 8)
	{
		bool flag = false;
		if (flag || getIsBlockSolid(iblockaccess, i, j, k - 1, 2)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i, j, k + 1, 3)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i - 1, j, k, 4)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i + 1, j, k, 5)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i, j + 1, k - 1, 2)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i, j + 1, k + 1, 3)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i - 1, j + 1, k, 4)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i + 1, j + 1, k, 5)) flag = true;
		if (flag)
		{
			vec3d = vec3d->normalize()->addVector(0.0, -6.0, 0.0);
		}
	}
	vec3d = vec3d->normalize();
	return vec3d;
}

void BlockFluid::velocityToAddToEntity(World *world, int_t i, int_t j, int_t k, Entity *entity, Vec3D *vec3d)
{
	Vec3D *vec3d1 = getFlowVector(world, i, j, k);
	vec3d->xCoord += vec3d1->xCoord;
	vec3d->yCoord += vec3d1->yCoord;
	vec3d->zCoord += vec3d1->zCoord;
}

int_t BlockFluid::tickRate()
{
	if (blockMaterial == Material::water)
	{
		return 5;
	}
	return blockMaterial != Material::lava ? 0 : 30;
}

float BlockFluid::getBlockBrightness(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	float f = iblockaccess->getLightBrightness(i, j, k);
	float f1 = iblockaccess->getLightBrightness(i, j + 1, k);
	return f <= f1 ? f1 : f;
}

void BlockFluid::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	Block::updateTick(world, i, j, k, random);
}

int_t BlockFluid::getRenderBlockPass()
{
	return blockMaterial != Material::water ? 0 : 1;
}

void BlockFluid::randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	int_t metadata;
	if (blockMaterial == Material::water)
	{
		if (random.nextInt(10) == 0)
		{
			metadata = world->getBlockMetadata(i, j, k);
			if (metadata <= 0 || metadata >= 8)
			{
				const double particleX = (double)((float)i + random.nextFloat());
				const double particleY = (double)((float)j + random.nextFloat());
				const double particleZ = (double)((float)k + random.nextFloat());
				world->spawnParticle("suspended", particleX, particleY, particleZ, 0.0, 0.0, 0.0);
			}
		}

		// Minecraft 1.2.5 keeps this splash loop at zero iterations. Preserve it exactly.
		for (metadata = 0; metadata < 0; ++metadata)
		{
		}
	}

	if (blockMaterial == Material::water && random.nextInt(64) == 0)
	{
		metadata = world->getBlockMetadata(i, j, k);
		if (metadata > 0 && metadata < 8)
		{
			const float volume = random.nextFloat() * 0.25f + 0.75f;
			const float pitch = random.nextFloat() + 0.5f;
			world->playSoundEffect((double)((float)i + 0.5f), (double)((float)j + 0.5f),
			                       (double)((float)k + 0.5f), "liquid.water", volume, pitch);
		}
	}

	if (blockMaterial == Material::lava && world->getBlockMaterial(i, j + 1, k) == Material::air &&
	    !world->isBlockOpaqueCube(i, j + 1, k))
	{
		if (random.nextInt(100) == 0)
		{
			const double x = (double)((float)i + random.nextFloat());
			const double y = (double)j + maxY;
			const double z = (double)((float)k + random.nextFloat());
			world->spawnParticle("lava", x, y, z, 0.0, 0.0, 0.0);
			const float popVolume = 0.2f + random.nextFloat() * 0.2f;
			const float popPitch = 0.9f + random.nextFloat() * 0.15f;
			world->playSoundEffect(x, y, z, "liquid.lavapop", popVolume, popPitch);
		}

		if (random.nextInt(200) == 0)
		{
			const float lavaVolume = 0.2f + random.nextFloat() * 0.2f;
			const float lavaPitch = 0.9f + random.nextFloat() * 0.15f;
			world->playSoundEffect((double)i, (double)j, (double)k, "liquid.lava", lavaVolume, lavaPitch);
		}
	}

	if (random.nextInt(10) == 0 && world->isBlockNormalCube(i, j - 1, k) &&
	    !world->getBlockMaterial(i, j - 2, k)->blocksMovement())
	{
		const double x = (double)((float)i + random.nextFloat());
		const double y = (double)j - 1.05;
		const double z = (double)((float)k + random.nextFloat());
		world->spawnParticle(blockMaterial == Material::water ? "dripWater" : "dripLava",
		                     x, y, z, 0.0, 0.0, 0.0);
	}
}

Vec3D *BlockFluid::getFlowVectorForMaterial(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, Material *material)
{
	if (material == Material::water)
	{
		return ((BlockFluid *)Block::waterMoving)->getFlowVector(iblockaccess, i, j, k);
	}
	if (material == Material::lava)
	{
		return ((BlockFluid *)Block::lavaMoving)->getFlowVector(iblockaccess, i, j, k);
	}
	return nullptr;
}

double BlockFluid::getFlowDirection(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, Material *material)
{
	Vec3D *vec3d = getFlowVectorForMaterial(iblockaccess, i, j, k, material);
	if (vec3d == nullptr || (vec3d->xCoord == 0.0 && vec3d->zCoord == 0.0))
	{
		return -1000.0;
	}
	return JavaMath::atan2(vec3d->zCoord, vec3d->xCoord) - 1.5707963267948966;
}

float BlockFluid::getFlowDirectionFloat(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, Material *material)
{
	Vec3D *vec3d = getFlowVectorForMaterial(iblockaccess, i, j, k, material);
	if (vec3d == nullptr)
	{
		return -1000.0f;
	}

	const float flowX = static_cast<float>(vec3d->xCoord);
	const float flowZ = static_cast<float>(vec3d->zCoord);
	if (flowX == 0.0f && flowZ == 0.0f)
	{
		return -1000.0f;
	}
#if PLATFORM_WII
	return ::atan2f(flowZ, flowX) - 1.5707963267948966f;
#else
	return std::atan2(flowZ, flowX) - 1.5707963267948966f;
#endif
}

double BlockFluid::func_293_a(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, Material *material)
{
	return getFlowDirection(iblockaccess, i, j, k, material);
}


void BlockFluid::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	checkForHarden(world, i, j, k);
}

void BlockFluid::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	checkForHarden(world, i, j, k);
}

void BlockFluid::checkForHarden(World *world, int_t i, int_t j, int_t k)
{
	if (world->getBlockId(i, j, k) != blockID)
	{
		return;
	}
	if (blockMaterial == Material::lava)
	{
		bool flag = false;
		if (flag || world->getBlockMaterial(i, j, k - 1) == Material::water) flag = true;
		if (flag || world->getBlockMaterial(i, j, k + 1) == Material::water) flag = true;
		if (flag || world->getBlockMaterial(i - 1, j, k) == Material::water) flag = true;
		if (flag || world->getBlockMaterial(i + 1, j, k) == Material::water) flag = true;
		if (flag || world->getBlockMaterial(i, j + 1, k) == Material::water) flag = true;
		if (flag)
		{
			int_t l = world->getBlockMetadata(i, j, k);
			if (l == 0)
			{
				world->setBlockWithNotify(i, j, k, Block::obsidian->blockID);
			}
			else if (l <= 4)
			{
				world->setBlockWithNotify(i, j, k, Block::cobblestone->blockID);
			}
			triggerLavaMixEffects(world, i, j, k);
		}
	}
}

void BlockFluid::triggerLavaMixEffects(World *world, int_t i, int_t j, int_t k)
{
	world->playSoundEffect((float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, "random.fizz", 0.5f, 2.6f + world->rand.nextFloatDifference() * 0.8f);
	for (int_t l = 0; l < 8; l++)
	{
		const double particleX = (double)i + Math::random();
		const double particleZ = (double)k + Math::random();
		world->spawnParticle("largesmoke", particleX, (double)j + 1.2, particleZ, 0.0, 0.0, 0.0);
	}
}
