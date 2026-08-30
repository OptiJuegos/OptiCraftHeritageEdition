#include "EmptyChunk.h"

#include "java/Type.h"
#include "World.h"

EmptyChunk::EmptyChunk(World *world, int_t i, int_t j)
	: Chunk(world, i, j)
{
	neverSave = true;
}

EmptyChunk::EmptyChunk(World *world, std::vector<byte_t> abyte0, int_t i, int_t j)
	: Chunk(world, std::move(abyte0), i, j)
{
	neverSave = true;
}

bool EmptyChunk::isAtLocation(int_t i, int_t j)
{
	return i == xPosition && j == zPosition;
}

int_t EmptyChunk::getHeightValue(int_t i, int_t j)
{
	return 0;
}

void EmptyChunk::initBlockLight()
{
}

void EmptyChunk::generateHeightMap()
{
}

void EmptyChunk::generateSkylightMap()
{
}

void EmptyChunk::onChunkLoadData()
{
}

int_t EmptyChunk::getBlockID(int_t i, int_t j, int_t k)
{
	return 0;
}

int_t EmptyChunk::getBlockLightOpacity(int_t i, int_t j, int_t k)
{
	return 255;
}

bool EmptyChunk::setBlockIDWithMetadata(int_t i, int_t j, int_t k, int_t l, int_t i1)
{
	return true;
}

bool EmptyChunk::setBlockID(int_t i, int_t j, int_t k, int_t l)
{
	return true;
}

int_t EmptyChunk::getBlockMetadata(int_t i, int_t j, int_t k)
{
	return 0;
}

bool EmptyChunk::setBlockMetadata(int_t i, int_t j, int_t k, int_t l)
{
	return false;
}

int_t EmptyChunk::getSavedLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k)
{
	return 0;
}

void EmptyChunk::setLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l)
{
}

int_t EmptyChunk::getBlockLightValue(int_t i, int_t j, int_t k, int_t l)
{
	return 0;
}

void EmptyChunk::addEntity(Entity *entity)
{
}

void EmptyChunk::removeEntity(Entity *entity)
{
}

void EmptyChunk::removeEntityAtIndex(Entity *entity, int_t i)
{
}

bool EmptyChunk::canBlockSeeTheSky(int_t i, int_t j, int_t k)
{
	return false;
}

TileEntity *EmptyChunk::getChunkBlockTileEntity(int_t i, int_t j, int_t k)
{
	return nullptr;
}

void EmptyChunk::addTileEntity(TileEntity *tileentity)
{
}

void EmptyChunk::setChunkBlockTileEntity(int_t i, int_t j, int_t k, TileEntity *tileentity)
{
}

void EmptyChunk::removeChunkBlockTileEntity(int_t i, int_t j, int_t k)
{
}

void EmptyChunk::onChunkLoad()
{
}

void EmptyChunk::onChunkUnload()
{
}

void EmptyChunk::setChunkModified()
{
}

void EmptyChunk::getEntitiesWithinAABBForEntity(Entity *entity, AxisAlignedBB *axisalignedbb, std::vector<Entity *> &list)
{
}

void EmptyChunk::getEntitiesOfTypeWithinAAAB(const std::type_info &type, AxisAlignedBB *axisalignedbb, std::vector<Entity *> &list)
{
}

bool EmptyChunk::needsSaving(bool flag)
{
	return false;
}

int_t EmptyChunk::setChunkData(byte_t *abyte0, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1, int_t k1)
{
	int_t l1 = l - i;
	int_t i2 = i1 - j;
	int_t j2 = j1 - k;
	int_t k2 = l1 * i2 * j2;
	return k2 + (k2 / 2) * 3;
}

bool EmptyChunk::isEmpty()
{
	return true;
}

bool EmptyChunk::isEmptyChunk()
{
	return true;
}

bool EmptyChunk::getAreLevelsEmpty(int_t minY, int_t maxY) const
{
	return true;
}
