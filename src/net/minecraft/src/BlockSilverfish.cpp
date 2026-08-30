#include "BlockSilverfish.h"

#include "EntitySilverfish.h"
#include "ItemStack.h"
#include "Material.h"
#include "World.h"

BlockSilverfish::BlockSilverfish(int_t id) :
	Block(id, 1, Material::clay)
{
	setHardness(0.0f);
}

void BlockSilverfish::harvestBlock(World *world, EntityPlayer *entityplayer, int_t x, int_t y, int_t z, int_t metadata)
{
	Block::harvestBlock(world, entityplayer, x, y, z, metadata);
}

int_t BlockSilverfish::getBlockTextureFromSideAndMetadata(int_t side, int_t metadata)
{
	if (metadata == 1) return Block::cobblestone->blockIndexInTexture;
	if (metadata == 2 && Block::stoneBrick != nullptr) return Block::stoneBrick->blockIndexInTexture;
	return Block::stone->blockIndexInTexture;
}

void BlockSilverfish::onBlockDestroyedByPlayer(World *world, int_t x, int_t y, int_t z, int_t metadata)
{
	if (!world->multiplayerWorld)
	{
		EntitySilverfish *silverfish = new EntitySilverfish(world);
		silverfish->setLocationAndAngles((double)x + 0.5, (double)y, (double)z + 0.5, 0.0f, 0.0f);
		bool spawned = world->spawnEntityInWorld(silverfish);
		silverfish->spawnExplosionParticle();
		if (!spawned)
			delete silverfish;
	}
	Block::onBlockDestroyedByPlayer(world, x, y, z, metadata);
}

int_t BlockSilverfish::quantityDropped(Random &random)
{
	return 0;
}

ItemStack *BlockSilverfish::createStackedBlock(int_t metadata)
{
	Block *block = Block::stone;
	if (metadata == 1)
		block = Block::cobblestone;
	else if (metadata == 2 && Block::stoneBrick != nullptr)
		block = Block::stoneBrick;
	return new ItemStack(block);
}

bool BlockSilverfish::getPosingIdByMetadata(int_t blockId)
{
	return blockId == Block::stone->blockID || blockId == Block::cobblestone->blockID ||
		(Block::stoneBrick != nullptr && blockId == Block::stoneBrick->blockID);
}

int_t BlockSilverfish::getMetadataForBlockType(int_t blockId)
{
	if (blockId == Block::cobblestone->blockID) return 1;
	if (Block::stoneBrick != nullptr && blockId == Block::stoneBrick->blockID) return 2;
	return 0;
}
