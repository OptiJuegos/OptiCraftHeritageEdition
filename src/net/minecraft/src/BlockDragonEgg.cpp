#include "BlockDragonEgg.h"

#include "BlockSand.h"
#include "EntityFallingSand.h"
#include "Material.h"
#include "World.h"
#include "java/Arithmetic.h"

BlockDragonEgg::BlockDragonEgg(int_t id, int_t texture) :
	Block(id, texture, Material::dragonEgg)
{
}

void BlockDragonEgg::onBlockAdded(World *world, int_t x, int_t y, int_t z)
{
	world->scheduleBlockUpdate(x, y, z, blockID, tickRate());
}

void BlockDragonEgg::onNeighborBlockChange(World *world, int_t x, int_t y, int_t z, int_t neighborId)
{
	world->scheduleBlockUpdate(x, y, z, blockID, tickRate());
}

void BlockDragonEgg::updateTick(World *world, int_t x, int_t y, int_t z, Random &random)
{
	fallIfPossible(world, x, y, z);
}

void BlockDragonEgg::fallIfPossible(World *world, int_t x, int_t y, int_t z)
{
	if (!BlockSand::canFallBelow(world, x, JavaArithmetic::intSub(y, 1), z) || y < 0)
	{
		return;
	}
	const int_t radius = 32;
	if (!BlockSand::fallInstantly && world->checkChunksExist(JavaArithmetic::intSub(x, radius), JavaArithmetic::intSub(y, radius), JavaArithmetic::intSub(z, radius),
        JavaArithmetic::intAdd(x, radius), JavaArithmetic::intAdd(y, radius), JavaArithmetic::intAdd(z, radius)))
	{
		EntityFallingSand *entity = new EntityFallingSand(world,
            static_cast<double>(static_cast<float>(x) + 0.5f),
            static_cast<double>(static_cast<float>(y) + 0.5f),
            static_cast<double>(static_cast<float>(z) + 0.5f), blockID);
		if (!world->spawnEntityInWorld(entity))
			delete entity;
		return;
	}

	world->setBlockWithNotify(x, y, z, 0);
	while (BlockSand::canFallBelow(world, x, JavaArithmetic::intSub(y, 1), z) && y > 0)
	{
		y = JavaArithmetic::intSub(y, 1);
	}
	if (y > 0)
	{
		world->setBlockWithNotify(x, y, z, blockID);
	}
}

bool BlockDragonEgg::blockActivated(World *world, int_t x, int_t y, int_t z, EntityPlayer *entityplayer)
{
	teleportNearby(world, x, y, z);
	return true;
}

void BlockDragonEgg::onBlockClicked(World *world, int_t x, int_t y, int_t z, EntityPlayer *entityplayer)
{
	teleportNearby(world, x, y, z);
}

void BlockDragonEgg::teleportNearby(World *world, int_t x, int_t y, int_t z)
{
	if (world->getBlockId(x, y, z) != blockID || world->multiplayerWorld)
	{
		return;
	}

	for (int_t attempt = 0; attempt < 1000; ++attempt)
	{
		const int_t targetX = JavaArithmetic::intAdd(x, world->rand.nextIntDifference(16));
		const int_t targetY = JavaArithmetic::intAdd(y, world->rand.nextIntDifference(8));
		const int_t targetZ = JavaArithmetic::intAdd(z, world->rand.nextIntDifference(16));
		if (world->getBlockId(targetX, targetY, targetZ) != 0)
		{
			continue;
		}

		world->setBlockAndMetadataWithNotify(targetX, targetY, targetZ, blockID, world->getBlockMetadata(x, y, z));
		world->setBlockWithNotify(x, y, z, 0);
		const int_t particleCount = 128;
		for (int_t particle = 0; particle < particleCount; ++particle)
		{
			const double interpolation = world->rand.nextDouble();
			const float motionX = (world->rand.nextFloat() - 0.5f) * 0.2f;
			const float motionY = (world->rand.nextFloat() - 0.5f) * 0.2f;
			const float motionZ = (world->rand.nextFloat() - 0.5f) * 0.2f;
			const double particleX = static_cast<double>(targetX) + static_cast<double>(JavaArithmetic::intSub(x, targetX)) * interpolation + (world->rand.nextDouble() - 0.5) + 0.5;
			const double particleY = static_cast<double>(targetY) + static_cast<double>(JavaArithmetic::intSub(y, targetY)) * interpolation + world->rand.nextDouble() - 0.5;
			const double particleZ = static_cast<double>(targetZ) + static_cast<double>(JavaArithmetic::intSub(z, targetZ)) * interpolation + (world->rand.nextDouble() - 0.5) + 0.5;
			world->spawnParticle("portal", particleX, particleY, particleZ, motionX, motionY, motionZ);
		}
		return;
	}
}

int_t BlockDragonEgg::tickRate()
{
	return 3;
}

bool BlockDragonEgg::canPlaceBlockAt(World *world, int_t x, int_t y, int_t z)
{
	return Block::canPlaceBlockAt(world, x, y, z);
}

bool BlockDragonEgg::isOpaqueCube()
{
	return false;
}

bool BlockDragonEgg::renderAsNormalBlock()
{
	return false;
}

int_t BlockDragonEgg::getRenderType()
{
	return 27;
}
