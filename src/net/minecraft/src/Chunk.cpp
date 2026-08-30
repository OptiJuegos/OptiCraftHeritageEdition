#include "Chunk.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <typeinfo>
#include <utility>

#include "platform/Log.h"
#include "platform/PlatformTuning.h"
#include "platform/WorldLoadTrace.h"
#include "java/Arithmetic.h"
#include "AxisAlignedBB.h"
#include "Block.h"
#include "BlockContainer.h"
#include "ChunkBlockMap.h"
#include "ChunkCoordIntPair.h"
#include "BiomeGenBase.h"
#include "WorldChunkManager.h"
#include "ExtendedBlockStorage.h"
#include "IChunkProvider.h"
#include "Material.h"
#include "MathHelper.h"
#include "TileEntity.h"
#include "World.h"
#include "WorldProvider.h"
#include "Entity.h"

bool Chunk::isLit = false;

namespace
{
	inline bool validBlockId(int_t blockId)
	{
		return blockId >= 0 && blockId < Block::BLOCK_REGISTRY_SIZE;
	}

	inline int_t localColumnIndex(int_t x, int_t z)
	{
		return z << 4 | x;
	}

	inline int_t clampSectionIndex(int_t section)
	{
		if (section < 0)
			return 0;
		if (section >= Chunk::SECTION_COUNT)
			return Chunk::SECTION_COUNT - 1;
		return section;
	}
}

Chunk::Chunk(World *world, int_t i, int_t j)
	: field_50120_o(false)
	, isChunkLoaded(false)
	, worldObj(world)
	, xPosition(i)
	, zPosition(j)
	, isTerrainPopulated(false)
	, isModified(false)
	, runtimeSaveRequired(false)
	, neverSave(false)
	, hasEntities(false)
	, lastSaveTime(0LL)
	, lastAccessTick(0LL)
	, blockBiomeArray(256u, (byte_t)0xff)
	, initialHeightMapReady(false)
	, isGapLightingUpdated(false)
	, queuedLightChecks(4096)
{
	std::fill(storageArrays, storageArrays + SECTION_COUNT, nullptr);
	std::fill(precipitationHeightMap, precipitationHeightMap + 256, -999);
	std::fill(updateSkylightColumns, updateSkylightColumns + 256, false);
	std::fill(heightMap, heightMap + 256, 0);
	std::fill(blockSectionRevision, blockSectionRevision + SECTION_COUNT, 0u);
}

Chunk::Chunk(World *world, std::vector<byte_t> abyte0, int_t i, int_t j)
	: Chunk(world, i, j)
{
	WORLD_LOAD_STAGE("Chunk fill");
	const int_t sourceHeight = std::min<int_t>((int_t)(abyte0.size() / 256u), WORLD_HEIGHT);
	std::vector<byte_t> *sectionBlocks[SECTION_COUNT] = {};

	for (int_t x = 0; x < 16; ++x)
	{
		for (int_t z = 0; z < 16; ++z)
		{
			for (int_t y = 0; y < sourceHeight; ++y)
			{
				const std::size_t sourceIndex = static_cast<std::size_t>(x << 11 | z << 7 | y);
				if (sourceIndex >= abyte0.size())
					continue;

				const byte_t blockId = abyte0[sourceIndex];
				if ((blockId & 0xff) == 0)
					continue;

#if PLATFORM_PRECOMPUTE_INITIAL_HEIGHTMAP
				const int_t blockValue = blockId & 0xff;
				if (validBlockId(blockValue) && Block::lightOpacity[blockValue] != 0)
					heightMap[localColumnIndex(x, z)] = y + 1;
#endif

				const int_t sectionIndex = y >> 4;
				std::vector<byte_t> *&sectionData = sectionBlocks[sectionIndex];
				if (sectionData == nullptr)
					sectionData = &ensureBlockStorage(sectionIndex)->func_48692_g();

				const std::size_t storageIndex = static_cast<std::size_t>(
					((y & 15) << 8) | (z << 4) | x);
				(*sectionData)[storageIndex] = blockId;
			}
		}
	}

	for (int_t section = 0; section < SECTION_COUNT; ++section)
	{
		if (storageArrays[section] != nullptr)
			storageArrays[section]->recalculateBlockCounts();
	}

#if PLATFORM_PRECOMPUTE_INITIAL_HEIGHTMAP
	const int_t scanTop = std::min(getTopFilledSegment() + 15, WORLD_HEIGHT - 1);
	for (int_t x = 0; x < 16; ++x)
	{
		for (int_t z = 0; z < 16; ++z)
		{
			const int_t columnIndex = localColumnIndex(x, z);
			if (heightMap[columnIndex] != scanTop + 1)
				continue;

			heightMap[columnIndex] = 0;
			for (int_t y = scanTop - 1; y >= 0; --y)
			{
				if (getBlockLightOpacity(x, y, z) == 0)
					continue;
				heightMap[columnIndex] = y + 1;
				break;
			}
		}
	}
	initialHeightMapReady = true;
#endif
}

Chunk::~Chunk()
{
	for (int_t section = 0; section < SECTION_COUNT; ++section)
	{
		delete storageArrays[section];
		storageArrays[section] = nullptr;
	}
}

bool Chunk::isAtLocation(int_t i, int_t j)
{
	return i == xPosition && j == zPosition;
}

int_t Chunk::getHeightValue(int_t i, int_t j)
{
	return heightMap[j << 4 | i];
}

int_t Chunk::getTopFilledSegment() const
{
	for (int_t section = SECTION_COUNT - 1; section >= 0; --section)
	{
		if (storageArrays[section] != nullptr)
			return storageArrays[section]->getYLocation();
	}
	return 0;
}

void Chunk::func_4143_d()
{
    // Intentionally empty in Minecraft 1.2.5.
}

ExtendedBlockStorage **Chunk::getBlockStorageArray()
{
	return storageArrays;
}

ExtendedBlockStorage *const *Chunk::getBlockStorageArray() const
{
	return storageArrays;
}

void Chunk::setStorageArrays(ExtendedBlockStorage *const *storage, int_t count)
{
    // Java replaces the section-array reference.  The C++ chunk owns individual
    // section objects, so transfer the supplied pointers while avoiding deletes
    // when a caller passes the chunk's current array back to itself.
    ExtendedBlockStorage *incoming[SECTION_COUNT] = {};
    const int_t limit = std::max<int_t>(0, std::min<int_t>(count, SECTION_COUNT));
    for (int_t section = 0; section < limit; ++section)
        incoming[section] = storage != nullptr ? storage[section] : nullptr;

    for (int_t section = 0; section < SECTION_COUNT; ++section)
    {
        if (storageArrays[section] != incoming[section])
            delete storageArrays[section];
        storageArrays[section] = incoming[section];
        ++blockSectionRevision[section];
    }
}

ExtendedBlockStorage *Chunk::getBlockStorage(int_t sectionY)
{
	return sectionY >= 0 && sectionY < SECTION_COUNT ? storageArrays[sectionY] : nullptr;
}

const ExtendedBlockStorage *Chunk::getBlockStorage(int_t sectionY) const
{
	return sectionY >= 0 && sectionY < SECTION_COUNT ? storageArrays[sectionY] : nullptr;
}

ExtendedBlockStorage *Chunk::ensureBlockStorage(int_t sectionY)
{
	if (sectionY < 0 || sectionY >= SECTION_COUNT)
		return nullptr;
	if (storageArrays[sectionY] == nullptr)
		storageArrays[sectionY] = new ExtendedBlockStorage(sectionY << 4);
	return storageArrays[sectionY];
}

void Chunk::clearBlockStorage(int_t sectionY)
{
	if (sectionY < 0 || sectionY >= SECTION_COUNT)
		return;
	delete storageArrays[sectionY];
	storageArrays[sectionY] = nullptr;
	++blockSectionRevision[sectionY];
}

bool Chunk::isValidLocalPosition(int_t x, int_t y, int_t z) const
{
	return x >= 0 && x < 16 && y >= WorldHeight::MIN_Y && y < WORLD_HEIGHT && z >= 0 && z < 16;
}

void Chunk::initBlockLight()
{
}

void Chunk::generateHeightMap()
{
	const int_t top = getTopFilledSegment();
	for (int_t x = 0; x < 16; ++x)
	{
		for (int_t z = 0; z < 16; ++z)
		{
			precipitationHeightMap[x + (z << 4)] = -999;
			heightMap[z << 4 | x] = 0;
			for (int_t y = std::min(top + 15, WORLD_HEIGHT - 1); y > 0; --y)
			{
				const int_t blockId = getBlockID(x, y - 1, z);
				if (validBlockId(blockId) && Block::lightOpacity[blockId] != 0)
				{
					heightMap[z << 4 | x] = y;
					break;
				}
			}
		}
	}
	isModified = true;
}

void Chunk::generateSkylightMap()
{
	WORLD_LOAD_STAGE("generateSkylightMap");
	WorldLoadTrace::step("columns");
	const int_t top = getTopFilledSegment();
	const int_t scanTop = std::min(top + 15, WORLD_HEIGHT - 1);
#if PLATFORM_BATCH_INITIAL_SKYLIGHT_RENDER_UPDATES
	bool initialSkylightDirty[SECTION_COUNT] = {};
#endif
	for (int_t x = 0; x < 16; ++x)
	{
		for (int_t z = 0; z < 16; ++z)
		{
			const int_t columnIndex = z << 4 | x;
			precipitationHeightMap[x + (z << 4)] = -999;

#if PLATFORM_PRECOMPUTE_INITIAL_HEIGHTMAP
			if (!initialHeightMapReady)
#endif
			{
				heightMap[columnIndex] = 0;
				for (int_t y = scanTop; y > 0; --y)
				{
					if (getBlockLightOpacity(x, y - 1, z) != 0)
					{
						heightMap[columnIndex] = y;
						break;
					}
				}
			}

			if (worldObj == nullptr || worldObj->worldProvider == nullptr || worldObj->worldProvider->hasNoSky)
				continue;

#if PLATFORM_PRECOMPUTE_INITIAL_HEIGHTMAP
			const int_t columnHeight = heightMap[columnIndex];
			int_t light = 15;
			if (scanTop > 0)
			{
				light -= getBlockLightOpacity(x, scanTop, z);
				if (light > 0)
				{
					ExtendedBlockStorage *section = storageArrays[scanTop >> 4];
					if (section != nullptr)
					{
						section->setExtSkylightValue(x, scanTop & 15, z, light);
#if PLATFORM_BATCH_INITIAL_SKYLIGHT_RENDER_UPDATES
						initialSkylightDirty[scanTop >> 4] = true;
#else
						worldObj->markBlockAsNeedsUpdate(JavaArithmetic::intAdd(JavaArithmetic::intShl(xPosition, 4), x), scanTop, JavaArithmetic::intAdd(JavaArithmetic::intShl(zPosition, 4), z));
#endif
					}
				}
			}

			for (int_t lightY = scanTop - 1; lightY > 0 && lightY >= columnHeight && light > 0; --lightY)
			{
				ExtendedBlockStorage *section = storageArrays[lightY >> 4];
				if (section != nullptr)
				{
					section->setExtSkylightValue(x, lightY & 15, z, light);
#if PLATFORM_BATCH_INITIAL_SKYLIGHT_RENDER_UPDATES
					initialSkylightDirty[lightY >> 4] = true;
#else
					worldObj->markBlockAsNeedsUpdate(JavaArithmetic::intAdd(JavaArithmetic::intShl(xPosition, 4), x), lightY, JavaArithmetic::intAdd(JavaArithmetic::intShl(zPosition, 4), z));
#endif
				}
			}

			for (int_t lightY = columnHeight - 1; lightY > 0 && light > 0; --lightY)
			{
				light -= getBlockLightOpacity(x, lightY, z);
				if (light <= 0)
					continue;
				ExtendedBlockStorage *section = storageArrays[lightY >> 4];
				if (section != nullptr)
				{
					section->setExtSkylightValue(x, lightY & 15, z, light);
#if PLATFORM_BATCH_INITIAL_SKYLIGHT_RENDER_UPDATES
					initialSkylightDirty[lightY >> 4] = true;
#else
					worldObj->markBlockAsNeedsUpdate(JavaArithmetic::intAdd(JavaArithmetic::intShl(xPosition, 4), x), lightY, JavaArithmetic::intAdd(JavaArithmetic::intShl(zPosition, 4), z));
#endif
				}
			}
#else
			int_t light = 15;
			for (int_t lightY = scanTop; lightY > 0 && light > 0; --lightY)
			{
				light -= getBlockLightOpacity(x, lightY, z);
				if (light <= 0)
					continue;
				ExtendedBlockStorage *section = storageArrays[lightY >> 4];
				if (section != nullptr)
				{
					section->setExtSkylightValue(x, lightY & 15, z, light);
#if PLATFORM_BATCH_INITIAL_SKYLIGHT_RENDER_UPDATES
					initialSkylightDirty[lightY >> 4] = true;
#else
					worldObj->markBlockAsNeedsUpdate(JavaArithmetic::intAdd(JavaArithmetic::intShl(xPosition, 4), x), lightY, JavaArithmetic::intAdd(JavaArithmetic::intShl(zPosition, 4), z));
#endif
				}
			}
#endif
		}
	}

#if PLATFORM_PRECOMPUTE_INITIAL_HEIGHTMAP
	initialHeightMapReady = false;
#endif

#if PLATFORM_BATCH_INITIAL_SKYLIGHT_RENDER_UPDATES
	WorldLoadTrace::step("markBlocksDirty");
	if (worldObj != nullptr)
	{
		const int_t worldX0 = JavaArithmetic::intShl(xPosition, 4);
		const int_t worldZ0 = JavaArithmetic::intShl(zPosition, 4);
		for (int_t sectionIndex = 0; sectionIndex < SECTION_COUNT; ++sectionIndex)
		{
			if (!initialSkylightDirty[sectionIndex])
				continue;
			const int_t minY = sectionIndex << 4;
			const int_t maxY = std::min(minY + 15, WORLD_HEIGHT - 1);
			worldObj->markBlocksDirty(worldX0, minY, worldZ0, worldX0 + 15, maxY, worldZ0 + 15);
		}
	}
#endif

	WorldLoadTrace::step("propagateOcclusion");
	isModified = true;
	std::fill(updateSkylightColumns, updateSkylightColumns + 256, true);
	isGapLightingUpdated = true;
}

void Chunk::onChunkLoadData()
{
}

void Chunk::propagateSkylightOcclusion(int_t i, int_t j)
{
	updateSkylightColumns[i + j * 16] = true;
	isGapLightingUpdated = true;
}

void Chunk::updateSkylight_do()
{
	if (worldObj == nullptr || !worldObj->doChunksNearChunkExist(JavaArithmetic::intAdd(JavaArithmetic::intMul(xPosition, 16), 8), 0, JavaArithmetic::intAdd(JavaArithmetic::intMul(zPosition, 16), 8), 16))
		return;

	for (int_t x = 0; x < 16; ++x)
	{
		for (int_t z = 0; z < 16; ++z)
		{
			const int_t index = x + z * 16;
			if (!updateSkylightColumns[index])
				continue;
			updateSkylightColumns[index] = false;

			const int_t height = getHeightValue(x, z);
			const int_t worldX = JavaArithmetic::intAdd(JavaArithmetic::intMul(xPosition, 16), x);
			const int_t worldZ = JavaArithmetic::intAdd(JavaArithmetic::intMul(zPosition, 16), z);
			int_t neighborHeight = worldObj->getHeightValue(worldX - 1, worldZ);
			neighborHeight = std::min(neighborHeight, worldObj->getHeightValue(worldX + 1, worldZ));
			neighborHeight = std::min(neighborHeight, worldObj->getHeightValue(worldX, worldZ - 1));
			neighborHeight = std::min(neighborHeight, worldObj->getHeightValue(worldX, worldZ + 1));

			checkSkylightNeighborHeight(worldX, worldZ, neighborHeight);
			checkSkylightNeighborHeight(worldX - 1, worldZ, height);
			checkSkylightNeighborHeight(worldX + 1, worldZ, height);
			checkSkylightNeighborHeight(worldX, worldZ - 1, height);
			checkSkylightNeighborHeight(worldX, worldZ + 1, height);
		}
	}
	isGapLightingUpdated = false;
}

void Chunk::checkSkylightNeighborHeight(int_t i, int_t j, int_t k)
{
	if (worldObj == nullptr)
		return;
	const int_t height = worldObj->getHeightValue(i, j);
	if (height > k)
		updateSkylightNeighborHeight(i, j, k, height + 1);
	else if (height < k)
		updateSkylightNeighborHeight(i, j, height, k + 1);
}

void Chunk::updateSkylightNeighborHeight(int_t i, int_t j, int_t minY, int_t maxY)
{
	if (worldObj == nullptr || maxY <= minY || !worldObj->doChunksNearChunkExist(i, 0, j, 16))
		return;
	minY = std::max(minY, WorldHeight::MIN_Y);
	maxY = std::min(maxY, WORLD_HEIGHT);
	if (maxY <= minY)
		return;
	worldObj->scheduleLightingUpdate(EnumSkyBlock::Sky, i, minY, j, i, maxY - 1, j);
	isModified = true;
}

void Chunk::relightBlock(int_t i, int_t j, int_t k)
{
	const int_t columnIndex = k << 4 | i;
	const int_t oldHeight = heightMap[columnIndex];
	int_t newHeight = std::max(j, oldHeight);
	while (newHeight > 0 && getBlockLightOpacity(i, newHeight - 1, k) == 0)
		--newHeight;
	if (newHeight == oldHeight)
		return;

	if (worldObj != nullptr)
		worldObj->markBlocksDirtyVertical(i, k, newHeight, oldHeight);
	heightMap[columnIndex] = newHeight;

	const int_t worldX = JavaArithmetic::intAdd(JavaArithmetic::intMul(xPosition, 16), i);
	const int_t worldZ = JavaArithmetic::intAdd(JavaArithmetic::intMul(zPosition, 16), k);
	if (worldObj != nullptr && worldObj->worldProvider != nullptr && !worldObj->worldProvider->hasNoSky)
	{
		if (newHeight < oldHeight)
		{
			for (int_t y = newHeight; y < oldHeight; ++y)
			{
				ExtendedBlockStorage *section = storageArrays[y >> 4];
				if (section != nullptr)
				{
					section->setExtSkylightValue(i, y & 15, k, 15);
					worldObj->markBlockAsNeedsUpdate(worldX, y, worldZ);
				}
			}
		}
		else
		{
			for (int_t y = oldHeight; y < newHeight; ++y)
			{
				ExtendedBlockStorage *section = storageArrays[y >> 4];
				if (section != nullptr)
				{
					section->setExtSkylightValue(i, y & 15, k, 0);
					worldObj->markBlockAsNeedsUpdate(worldX, y, worldZ);
				}
			}
		}

		int_t light = 15;
		int_t scanY = newHeight;
		while (scanY > 0 && light > 0)
		{
			--scanY;
			int_t opacity = getBlockLightOpacity(i, scanY, k);
			if (opacity == 0)
				opacity = 1;
			light = std::max(0, light - opacity);
			ExtendedBlockStorage *section = storageArrays[scanY >> 4];
			if (section != nullptr)
				section->setExtSkylightValue(i, scanY & 15, k, light);
		}

		const int_t low = std::min(oldHeight, newHeight);
		const int_t high = std::max(oldHeight, newHeight);
		updateSkylightNeighborHeight(worldX - 1, worldZ, low, high);
		updateSkylightNeighborHeight(worldX + 1, worldZ, low, high);
		updateSkylightNeighborHeight(worldX, worldZ - 1, low, high);
		updateSkylightNeighborHeight(worldX, worldZ + 1, low, high);
		updateSkylightNeighborHeight(worldX, worldZ, low, high);
	}
	isModified = true;
}

int_t Chunk::getBlockLightOpacity(int_t i, int_t j, int_t k)
{
	const int_t blockId = getBlockID(i, j, k);
	return validBlockId(blockId) ? Block::lightOpacity[blockId] : 0;
}

int_t Chunk::getBlockID(int_t i, int_t j, int_t k)
{
	if (!isValidLocalPosition(i, j, k))
		return 0;
	ExtendedBlockStorage *section = storageArrays[j >> 4];
	return section != nullptr ? section->getExtBlockID(i, j & 15, k) : 0;
}

int_t Chunk::getBlockMetadata(int_t i, int_t j, int_t k)
{
	if (!isValidLocalPosition(i, j, k))
		return 0;
	ExtendedBlockStorage *section = storageArrays[j >> 4];
	return section != nullptr ? section->getExtBlockMetadata(i, j & 15, k) : 0;
}

bool Chunk::setBlockID(int_t i, int_t j, int_t k, int_t l)
{
	return setBlockIDWithMetadata(i, j, k, l, 0);
}

bool Chunk::setBlockIDWithMetadata(int_t i, int_t j, int_t k, int_t l, int_t i1)
{
	if (!isValidLocalPosition(i, j, k) || !validBlockId(l))
		return false;

	const int_t columnIndex = localColumnIndex(i, k);
	if (j >= precipitationHeightMap[columnIndex] - 1)
		precipitationHeightMap[columnIndex] = -999;

	const int_t oldHeight = heightMap[columnIndex];
	const int_t oldId = getBlockID(i, j, k);
	if (oldId == l && getBlockMetadata(i, j, k) == i1)
		return false;

	ExtendedBlockStorage *section = storageArrays[j >> 4];
	bool createdAboveHeight = false;
	if (section == nullptr)
	{
		if (l == 0)
			return false;
		section = ensureBlockStorage(j >> 4);
		createdAboveHeight = j >= oldHeight;
	}

	section->setExtBlockID(i, j & 15, k, l);
	const int_t worldX = JavaArithmetic::intAdd(JavaArithmetic::intMul(xPosition, 16), i);
	const int_t worldZ = JavaArithmetic::intAdd(JavaArithmetic::intMul(zPosition, 16), k);

	if (oldId != 0 && validBlockId(oldId) && Block::blocksList[oldId] != nullptr)
	{
		if (worldObj != nullptr && !worldObj->multiplayerWorld)
			Block::blocksList[oldId]->onBlockRemoval(worldObj, worldX, j, worldZ);
		else if (oldId != l && Block::isBlockContainer[oldId] && worldObj != nullptr)
			worldObj->removeBlockTileEntity(worldX, j, worldZ);
	}

	if (section->getExtBlockID(i, j & 15, k) != l)
		return false;

	section->setExtBlockMetadata(i, j & 15, k, i1);
	if (createdAboveHeight)
	{
		// Deferred to endPopulationFastPath() when this write is part of the
		// active population step: see Chunk.h's skylightRegenPending for why.
		if (worldObj != nullptr && worldObj->isPopulationFastPathChunk(this))
			skylightRegenPending = true;
		else
			generateSkylightMap();
	}
	else
	{
		if (Block::lightOpacity[l] > 0)
		{
			if (j >= oldHeight)
				relightBlock(i, j + 1, k);
		}
		else if (j == oldHeight - 1)
		{
			relightBlock(i, j, k);
		}
		propagateSkylightOcclusion(i, k);
	}

	if (worldObj != nullptr)
	{
		worldObj->scheduleLightingUpdate(EnumSkyBlock::Block, worldX, j, worldZ, worldX, j, worldZ);
		if (!worldObj->worldProvider->hasNoSky)
			worldObj->scheduleLightingUpdate(EnumSkyBlock::Sky, worldX, j, worldZ, worldX, j, worldZ);
	}

	if (l != 0 && Block::blocksList[l] != nullptr)
	{
		if (worldObj != nullptr && !worldObj->multiplayerWorld)
			Block::blocksList[l]->onBlockAdded(worldObj, worldX, j, worldZ);

		if (Block::isBlockContainer[l] && worldObj != nullptr)
		{
			TileEntity *tileEntity = getChunkBlockTileEntity(i, j, k);
			if (tileEntity == nullptr)
			{
				BlockContainer *container = dynamic_cast<BlockContainer *>(Block::blocksList[l]);
				if (container != nullptr)
				{
					tileEntity = container->getBlockEntity();
					worldObj->setBlockTileEntity(worldX, j, worldZ, tileEntity);
				}
			}

			if (tileEntity != nullptr)
				tileEntity->updateContainingBlockInfo();
		}
	}
	else if (oldId > 0 && validBlockId(oldId) && Block::isBlockContainer[oldId])
	{
		TileEntity *tileEntity = getChunkBlockTileEntity(i, j, k);
		if (tileEntity != nullptr)
			tileEntity->updateContainingBlockInfo();
	}

	++blockSectionRevision[j >> 4];
	isModified = true;
#if PLATFORM_SAVE_RUNTIME_CHUNK_EDITS_ON_UNLOAD
	if (worldObj == nullptr || !worldObj->isPopulationFastPathChunk(this))
		markRuntimeSaveRequired();
#endif
	return true;
}

bool Chunk::replaceBlockIDForPopulation(int_t i, int_t j, int_t k,
	                                    int_t expectedId, int_t newId)
{
	if (!isValidLocalPosition(i, j, k) || !validBlockId(newId) || expectedId == newId)
		return false;
	ExtendedBlockStorage *section = storageArrays[j >> 4];
	if (section == nullptr || section->getExtBlockID(i, j & 15, k) != expectedId)
		return false;
	section->setExtBlockID(i, j & 15, k, newId);
	section->setExtBlockMetadata(i, j & 15, k, 0);
	++blockSectionRevision[j >> 4];
	isModified = true;
	return true;
}

bool Chunk::setBlockMetadata(int_t i, int_t j, int_t k, int_t l)
{
	if (!isValidLocalPosition(i, j, k))
		return false;
	ExtendedBlockStorage *section = storageArrays[j >> 4];
	if (section == nullptr)
		return false;
	const int_t oldMetadata = section->getExtBlockMetadata(i, j & 15, k);
	if (oldMetadata == l)
		return false;

	section->setExtBlockMetadata(i, j & 15, k, l);
	++blockSectionRevision[j >> 4];
	isModified = true;
#if PLATFORM_SAVE_RUNTIME_CHUNK_EDITS_ON_UNLOAD
	if (worldObj == nullptr || !worldObj->isPopulationFastPathChunk(this))
		markRuntimeSaveRequired();
#endif

	const int_t blockId = section->getExtBlockID(i, j & 15, k);
	if (blockId > 0 && validBlockId(blockId) && Block::isBlockContainer[blockId])
	{
		TileEntity *tileEntity = getChunkBlockTileEntity(i, j, k);
		if (tileEntity != nullptr)
		{
			tileEntity->updateContainingBlockInfo();
			tileEntity->blockMetadata = l;
		}
	}
	return true;
}

std::uint32_t Chunk::getBlockSectionRevision(int_t sectionY) const
{
	return sectionY >= 0 && sectionY < SECTION_COUNT ? blockSectionRevision[sectionY] : 0u;
}

int_t Chunk::getSavedLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k)
{
	if (!isValidLocalPosition(i, j, k))
		return enumskyblock != nullptr ? enumskyblock->defaultLightValue : 0;
	ExtendedBlockStorage *section = storageArrays[j >> 4];
	if (section == nullptr)
		return enumskyblock != nullptr ? enumskyblock->defaultLightValue : 0;
	if (enumskyblock == EnumSkyBlock::Sky)
		return section->getExtSkylightValue(i, j & 15, k);
	if (enumskyblock == EnumSkyBlock::Block)
		return section->getExtBlocklightValue(i, j & 15, k);
	return enumskyblock != nullptr ? enumskyblock->defaultLightValue : 0;
}

void Chunk::setLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l)
{
	if (!isValidLocalPosition(i, j, k) || enumskyblock == nullptr)
		return;
	ExtendedBlockStorage *section = storageArrays[j >> 4];
	if (section == nullptr)
	{
		section = ensureBlockStorage(j >> 4);
		generateSkylightMap();
	}
	if (section == nullptr)
		return;

	isModified = true;
	if (enumskyblock == EnumSkyBlock::Sky)
	{
		if (worldObj == nullptr || worldObj->worldProvider == nullptr || !worldObj->worldProvider->hasNoSky)
			section->setExtSkylightValue(i, j & 15, k, l);
	}
	else if (enumskyblock == EnumSkyBlock::Block)
	{
		section->setExtBlocklightValue(i, j & 15, k, l);
	}
}

int_t Chunk::getBlockLightValue(int_t i, int_t j, int_t k, int_t l)
{
	if (!isValidLocalPosition(i, j, k))
		return 0;
	ExtendedBlockStorage *section = storageArrays[j >> 4];
	const bool hasSky = worldObj == nullptr || worldObj->worldProvider == nullptr || !worldObj->worldProvider->hasNoSky;
	if (section == nullptr)
		return hasSky && l < EnumSkyBlock::Sky->defaultLightValue ? EnumSkyBlock::Sky->defaultLightValue - l : 0;

	int_t skyLight = hasSky ? section->getExtSkylightValue(i, j & 15, k) : 0;
	if (skyLight > 0)
		isLit = true;
	skyLight -= l;
	const int_t blockLight = section->getExtBlocklightValue(i, j & 15, k);
	return std::max(blockLight, skyLight);
}

void Chunk::addEntity(Entity *entity)
{
	if (entity == nullptr)
		return;

	// Java's GC tolerates duplicate references much better than C++ ownership does.
	// A stale duplicate in another vertical entity bucket becomes a dangling pointer
	// after World::destroyEntity() frees the object, and chunk saving later
	// dereferences every bucket. Scrub this chunk before inserting the authoritative
	// entry so vertical moves/reloads cannot leave a second copy behind.
	removeEntityFromAllSections(entity);
	hasEntities = true;
	const int_t chunkX = MathHelper::floor_double(entity->posX / 16.0);
	const int_t chunkZ = MathHelper::floor_double(entity->posZ / 16.0);
	if (chunkX != xPosition || chunkZ != zPosition)
		MC_LOG_INFO("chunk", "Wrong entity location for chunk (%d,%d): entity belongs to (%d,%d)\n", xPosition, zPosition, chunkX, chunkZ);

	int_t section = clampSectionIndex(MathHelper::floor_double(entity->posY / 16.0));
	entity->addedToChunk = true;
	entity->chunkCoordX = xPosition;
	entity->chunkCoordY = section;
	entity->chunkCoordZ = zPosition;
	entities[section].push_back(entity);
}

void Chunk::removeEntity(Entity *entity)
{
	if (entity == nullptr)
		return;
	removeEntityAtIndex(entity, entity->chunkCoordY);
}

void Chunk::removeEntityAtIndex(Entity *entity, int_t i)
{
	if (entity == nullptr)
		return;
	i = clampSectionIndex(i);
	auto &list = entities[i];
	list.erase(std::remove(list.begin(), list.end(), entity), list.end());
}

void Chunk::removeEntityFromAllSections(Entity *entity)
{
	if (entity == nullptr)
		return;

	for (int_t section = 0; section < SECTION_COUNT; ++section)
	{
		auto &list = entities[section];
		list.erase(std::remove(list.begin(), list.end(), entity), list.end());
	}
}

bool Chunk::canBlockSeeTheSky(int_t i, int_t j, int_t k)
{
	if (i < 0 || i >= 16 || k < 0 || k >= 16)
		return false;
	return j >= heightMap[k << 4 | i];
}

TileEntity *Chunk::getChunkBlockTileEntity(int_t i, int_t j, int_t k)
{
	if (!isValidLocalPosition(i, j, k))
		return nullptr;
	ChunkPosition chunkposition(i, j, k);
	auto it = chunkTileEntityMap.find(chunkposition);
	TileEntity *tileentity = (it != chunkTileEntityMap.end()) ? it->second : nullptr;
	if (tileentity == nullptr)
	{
		const int_t blockId = getBlockID(i, j, k);
		if (!validBlockId(blockId) || blockId <= 0 || !Block::isBlockContainer[blockId] || Block::blocksList[blockId] == nullptr)
			return nullptr;
		BlockContainer *blockcontainer = dynamic_cast<BlockContainer *>(Block::blocksList[blockId]);
		if (blockcontainer == nullptr || worldObj == nullptr)
			return nullptr;

		// Minecraft 1.2.5 creates the missing tile entity directly here. Calling
		// onBlockAdded() is not equivalent: subclasses may perform neighbour or
		// structure side effects when a block is placed, and a lazy tile-entity
		// lookup must not replay those effects.
		tileentity = blockcontainer->getBlockEntity();
		worldObj->setBlockTileEntity(
			JavaArithmetic::intAdd(JavaArithmetic::intMul(xPosition, 16), i),
			j,
			JavaArithmetic::intAdd(JavaArithmetic::intMul(zPosition, 16), k),
			tileentity);

		it = chunkTileEntityMap.find(chunkposition);
		tileentity = (it != chunkTileEntityMap.end()) ? it->second : nullptr;
		if (tileentity == nullptr)
		{
			tileentity = worldObj->getPendingTileEntity(
				JavaArithmetic::intAdd(JavaArithmetic::intMul(xPosition, 16), i), j,
				JavaArithmetic::intAdd(JavaArithmetic::intMul(zPosition, 16), k));
			if (tileentity != nullptr)
				setChunkBlockTileEntity(i, j, k, tileentity);
		}
	}
	if (tileentity != nullptr && tileentity->isInvalid())
	{
		chunkTileEntityMap.erase(chunkposition);
		chunkTileEntityOrder.remove(chunkposition);
		return nullptr;
	}
	return tileentity;
}

void Chunk::addTileEntity(TileEntity *tileentity)
{
	if (tileentity == nullptr)
		return;
	const int_t i = JavaArithmetic::intSub(tileentity->xCoord, JavaArithmetic::intMul(xPosition, 16));
	const int_t j = tileentity->yCoord;
	const int_t k = JavaArithmetic::intSub(tileentity->zCoord, JavaArithmetic::intMul(zPosition, 16));
	setChunkBlockTileEntity(i, j, k, tileentity);
	if (isChunkLoaded && worldObj != nullptr &&
	    std::find(worldObj->loadedTileEntityList.begin(), worldObj->loadedTileEntityList.end(), tileentity) ==
	        worldObj->loadedTileEntityList.end())
	{
		worldObj->loadedTileEntityList.push_back(tileentity);
	}
}

void Chunk::setChunkBlockTileEntity(int_t i, int_t j, int_t k, TileEntity *tileentity)
{
	if (tileentity == nullptr || !isValidLocalPosition(i, j, k))
		return;
	ChunkPosition chunkposition(i, j, k);
	tileentity->worldObj = worldObj;
	tileentity->xCoord = JavaArithmetic::intAdd(JavaArithmetic::intMul(xPosition, 16), i);
	tileentity->yCoord = j;
	tileentity->zCoord = JavaArithmetic::intAdd(JavaArithmetic::intMul(zPosition, 16), k);
	const int_t blockId = getBlockID(i, j, k);
	if (!validBlockId(blockId) || blockId == 0 || Block::blocksList[blockId] == nullptr || !Block::isBlockContainer[blockId])
	{
		MC_LOG_INFO("chunk", "Attempted to place a tile entity where there was no entity tile!\n");
		return;
	}

	auto oldIt = chunkTileEntityMap.find(chunkposition);
	if (oldIt != chunkTileEntityMap.end() && oldIt->second != nullptr && oldIt->second != tileentity)
	{
		TileEntity *oldTileEntity = oldIt->second;
		if (worldObj != nullptr)
		{
			worldObj->closeContainersUsing(oldTileEntity);
			worldObj->detachTileEntityForChunkUnload(oldTileEntity);
		}
		oldTileEntity->invalidate();
		delete oldTileEntity;
	}

	tileentity->validate();
	chunkTileEntityOrder.add(chunkposition);
	chunkTileEntityMap[chunkposition] = tileentity;
}

void Chunk::removeChunkBlockTileEntity(int_t i, int_t j, int_t k)
{
	if (!isValidLocalPosition(i, j, k))
		return;
	ChunkPosition chunkposition(i, j, k);
	if (!isChunkLoaded)
		return;
	auto it = chunkTileEntityMap.find(chunkposition);
	if (it == chunkTileEntityMap.end())
		return;
	TileEntity *tileentity = it->second;
	chunkTileEntityMap.erase(it);
	chunkTileEntityOrder.remove(chunkposition);
	if (tileentity != nullptr)
	{
		if (worldObj != nullptr)
			worldObj->notifyTileEntityRenderersRemoved(tileentity);
		tileentity->invalidate();
	}
}

void Chunk::onChunkLoad()
{
	isChunkLoaded = true;
	if (worldObj == nullptr)
		return;
	std::vector<TileEntity *> tileEntities;
	tileEntities.reserve(chunkTileEntityMap.size());
	for (const ChunkPosition &position : chunkTileEntityOrder.valuesInIterationOrder())
	{
		auto it = chunkTileEntityMap.find(position);
		if (it != chunkTileEntityMap.end())
			tileEntities.push_back(it->second);
	}
	worldObj->addLoadedTileEntities(tileEntities);
	for (int_t i = 0; i < SECTION_COUNT; ++i)
		worldObj->addLoadedEntities(entities[i]);
}

void Chunk::onChunkUnload()
{
	field_50120_o = false;
	isChunkLoaded = false;
	std::vector<TileEntity *> tileEntitiesToDelete;
	tileEntitiesToDelete.reserve(chunkTileEntityMap.size());
	for (const ChunkPosition &position : chunkTileEntityOrder.valuesInIterationOrder())
	{
		auto it = chunkTileEntityMap.find(position);
		if (it == chunkTileEntityMap.end())
			continue;
		TileEntity *tileEntity = it->second;
		if (tileEntity == nullptr)
			continue;
		if (worldObj != nullptr)
		{
			worldObj->closeContainersUsing(tileEntity);
			worldObj->detachTileEntityForChunkUnload(tileEntity);
		}
		tileEntity->invalidate();
		tileEntitiesToDelete.push_back(tileEntity);
	}
	chunkTileEntityMap.clear();
	chunkTileEntityOrder.clear();
	for (TileEntity *tileEntity : tileEntitiesToDelete)
		delete tileEntity;

	if (worldObj != nullptr)
	{
		for (int_t i = 0; i < SECTION_COUNT; ++i)
			worldObj->unloadEntities(entities[i]);
	}
}

void Chunk::setChunkModified()
{
	isModified = true;
}

void Chunk::markRuntimeSaveRequired()
{
	runtimeSaveRequired = true;
}

void Chunk::clearRuntimeSaveRequired()
{
	runtimeSaveRequired = false;
}

bool Chunk::isRuntimeSaveRequired() const
{
	return runtimeSaveRequired;
}

void Chunk::getEntitiesWithinAABBForEntity(Entity *entity, AxisAlignedBB *axisalignedbb, std::vector<Entity *> &list)
{
	if (axisalignedbb == nullptr)
		return;
	int_t minSection = MathHelper::floor_double((axisalignedbb->minY - 2.0) / 16.0);
	int_t maxSection = MathHelper::floor_double((axisalignedbb->maxY + 2.0) / 16.0);
	minSection = clampSectionIndex(minSection);
	maxSection = clampSectionIndex(maxSection);
	if (maxSection < minSection)
		return;
	for (int_t section = minSection; section <= maxSection; ++section)
	{
		auto &sectionEntities = entities[section];
		for (auto it = sectionEntities.begin(); it != sectionEntities.end();)
		{
			Entity *candidate = *it;

			// Chunk buckets are non-owning. If World no longer tracks this raw
			// address, dereferencing it can be a use-after-free during collision
			// queries. Purge the stale index entry before touching candidate.
			if (candidate == nullptr || worldObj == nullptr || !worldObj->isLoadedEntityPointer(candidate))
			{
				if (candidate != nullptr)
				{
					MC_LOG_ERROR("chunk",
					    "Purging stale entity pointer during AABB query chunk=%d,%d section=%d ptr=%p\n",
					    xPosition, zPosition, section, (void *)candidate);
				}
				it = sectionEntities.erase(it);
				continue;
			}

			++it;
			if (candidate == entity || candidate->boundingBox == nullptr ||
			    !candidate->boundingBox->intersectsWith(axisalignedbb))
			{
				continue;
			}

			list.push_back(candidate);
			const std::vector<Entity *> parts = candidate->getParts();
			for (Entity *part : parts)
			{
				if (part != nullptr && part != entity && part->boundingBox != nullptr &&
				    part->boundingBox->intersectsWith(axisalignedbb))
				{
					list.push_back(part);
				}
			}
		}
	}
}

void Chunk::getEntitiesOfTypeWithinAAAB(const std::type_info &type, AxisAlignedBB *axisalignedbb, std::vector<Entity *> &list)
{
	if (axisalignedbb == nullptr)
		return;
	int_t minSection = clampSectionIndex(MathHelper::floor_double((axisalignedbb->minY - 2.0) / 16.0));
	int_t maxSection = clampSectionIndex(MathHelper::floor_double((axisalignedbb->maxY + 2.0) / 16.0));
	if (maxSection < minSection)
		return;
	for (int_t section = minSection; section <= maxSection; ++section)
	{
		auto &sectionEntities = entities[section];
		for (auto it = sectionEntities.begin(); it != sectionEntities.end();)
		{
			Entity *candidate = *it;
			if (candidate == nullptr || worldObj == nullptr || !worldObj->isLoadedEntityPointer(candidate))
			{
				if (candidate != nullptr)
				{
					MC_LOG_ERROR("chunk",
					    "Purging stale entity pointer during AABB query chunk=%d,%d section=%d ptr=%p\n",
					    xPosition, zPosition, section, (void *)candidate);
				}
				it = sectionEntities.erase(it);
				continue;
			}

			++it;
			if (candidate->isAssignableTo(type) && candidate->boundingBox != nullptr &&
			    candidate->boundingBox->intersectsWith(axisalignedbb))
			{
				list.push_back(candidate);
			}
		}
	}
}

bool Chunk::needsSaving(bool flag)
{
	if (neverSave)
		return false;
	if (flag)
	{
		if (hasEntities && worldObj != nullptr && worldObj->getWorldTime() != lastSaveTime)
			return true;
	}
	else if (hasEntities && worldObj != nullptr && worldObj->getWorldTime() >= lastSaveTime + 600LL)
	{
		return true;
	}
	return isModified;
}

int_t Chunk::setChunkData(byte_t *abyte0, int_t i, int_t j, int_t k,
	                     int_t l, int_t i1, int_t j1, int_t k1)
{
	if (abyte0 == nullptr)
		return k1;
	const int_t minX = std::max(0, i);
	const int_t minY = std::max(0, j);
	const int_t minZ = std::max(0, k);
	const int_t maxX = std::min(16, l);
	const int_t maxY = std::min(WORLD_HEIGHT, i1);
	const int_t maxZ = std::min(16, j1);
	if (maxX <= minX || maxY <= minY || maxZ <= minZ)
		return k1;

	for (int_t x = minX; x < maxX; ++x)
	{
		for (int_t z = minZ; z < maxZ; ++z)
		{
			for (int_t y = minY; y < maxY; ++y)
			{
				const int_t blockId = abyte0[k1++] & 0xff;
				if (blockId != 0)
					ensureBlockStorage(y >> 4)->setExtBlockID(x, y & 15, z, blockId);
				else if (storageArrays[y >> 4] != nullptr)
					storageArrays[y >> 4]->setExtBlockID(x, y & 15, z, 0);
			}
		}
	}

	const int_t height = maxY - minY;
	const int_t packedPerColumn = height / 2;
	auto loadNibblePlane = [&](int plane)
	{
		for (int_t x = minX; x < maxX; ++x)
		{
			for (int_t z = minZ; z < maxZ; ++z)
			{
				for (int_t n = 0; n < packedPerColumn; ++n)
				{
					const byte_t packed = abyte0[k1++];
					const int_t y0 = minY + n * 2;
					const int_t y1Value = y0 + 1;
					ExtendedBlockStorage *s0 = ensureBlockStorage(y0 >> 4);
					if (s0 != nullptr)
					{
						const int_t v0 = packed & 0xf;
						if (plane == 0) s0->setExtBlockMetadata(x, y0 & 15, z, v0);
						else if (plane == 1) s0->setExtBlocklightValue(x, y0 & 15, z, v0);
						else s0->setExtSkylightValue(x, y0 & 15, z, v0);
					}
					if (y1Value < maxY)
					{
						ExtendedBlockStorage *s1 = ensureBlockStorage(y1Value >> 4);
						if (s1 != nullptr)
						{
							const int_t v1 = (packed >> 4) & 0xf;
							if (plane == 0) s1->setExtBlockMetadata(x, y1Value & 15, z, v1);
							else if (plane == 1) s1->setExtBlocklightValue(x, y1Value & 15, z, v1);
							else s1->setExtSkylightValue(x, y1Value & 15, z, v1);
						}
					}
				}
			}
		}
	};

	loadNibblePlane(0);
	loadNibblePlane(1);
	loadNibblePlane(2);
	for (int_t section = minY >> 4; section <= (maxY - 1) >> 4; ++section)
	{
		if (storageArrays[section] != nullptr)
			storageArrays[section]->func_48708_d();
		++blockSectionRevision[section];
	}
	generateHeightMap();
	return k1;
}

bool Chunk::func_48494_a(const byte_t *bytes, std::size_t length,
	                    int_t primaryBitMask, int_t addBitMask,
	                    bool includeBiomeData, std::size_t *bytesConsumed)
{
	if (bytesConsumed != nullptr)
		*bytesConsumed = 0u;
	if (bytes == nullptr)
		return false;

	std::size_t required = 0u;
	for (int_t section = 0; section < SECTION_COUNT; ++section)
	{
		if ((primaryBitMask & (1 << section)) != 0)
			required += 4096u;
	}
	for (int plane = 0; plane < 3; ++plane)
	{
		for (int_t section = 0; section < SECTION_COUNT; ++section)
		{
			if ((primaryBitMask & (1 << section)) != 0)
				required += 2048u;
		}
	}
	for (int_t section = 0; section < SECTION_COUNT; ++section)
	{
		if ((addBitMask & (1 << section)) != 0)
			required += 2048u;
	}
	if (includeBiomeData)
		required += 256u;
	if (length < required)
		return false;

	std::size_t offset = 0u;
	for (int_t section = 0; section < SECTION_COUNT; ++section)
	{
		if ((primaryBitMask & (1 << section)) != 0)
		{
			ExtendedBlockStorage *storage = ensureBlockStorage(section);
			std::vector<byte_t> &lsb = storage->func_48692_g();
			std::memcpy(lsb.data(), bytes + offset, lsb.size());
			offset += lsb.size();
		}
		else if (includeBiomeData)
		{
			clearBlockStorage(section);
		}
	}

	auto copyNibblePlane = [&](int plane)
	{
		for (int_t section = 0; section < SECTION_COUNT; ++section)
		{
			if ((primaryBitMask & (1 << section)) == 0)
				continue;
			ExtendedBlockStorage *storage = storageArrays[section];
			NibbleArray *array = nullptr;
			if (plane == 0) array = &storage->func_48697_j();
			else if (plane == 1) array = &storage->getBlocklightArray();
			else array = &storage->getSkylightArray();
			std::memcpy(array->data.data(), bytes + offset, array->data.size());
			offset += array->data.size();
		}
	};
	copyNibblePlane(0);
	copyNibblePlane(1);
	copyNibblePlane(2);

	for (int_t section = 0; section < SECTION_COUNT; ++section)
	{
		if ((addBitMask & (1 << section)) != 0)
		{
			ExtendedBlockStorage *storage = storageArrays[section];
			if (storage == nullptr)
			{
				offset += 2048u;
				continue;
			}
			NibbleArray *array = storage->getBlockMSBArray();
			if (array == nullptr)
				array = storage->createBlockMSBArray();
			std::memcpy(array->data.data(), bytes + offset, array->data.size());
			offset += array->data.size();
		}
		else if (includeBiomeData && storageArrays[section] != nullptr && storageArrays[section]->getBlockMSBArray() != nullptr)
		{
			storageArrays[section]->func_48715_h();
		}
	}

	if (includeBiomeData)
	{
		std::memcpy(blockBiomeArray.data(), bytes + offset, blockBiomeArray.size());
		offset += blockBiomeArray.size();
	}

	for (int_t section = 0; section < SECTION_COUNT; ++section)
	{
		if (storageArrays[section] != nullptr && (primaryBitMask & (1 << section)) != 0)
		{
			storageArrays[section]->func_48708_d();
			++blockSectionRevision[section];
		}
	}
	generateHeightMap();
	for (auto &entry : chunkTileEntityMap)
	{
		if (entry.second != nullptr)
			entry.second->updateContainingBlockInfo();
	}
	if (bytesConsumed != nullptr)
		*bytesConsumed = offset;
	return true;
}

bool Chunk::getAreLevelsEmpty(int_t minY, int_t maxY) const
{
	minY = std::max(minY, 0);
	maxY = std::min(maxY, WORLD_HEIGHT - 1);
	if (maxY < minY)
		return true;
	for (int_t y = minY; y <= maxY; y += SECTION_HEIGHT)
	{
		const ExtendedBlockStorage *section = storageArrays[y >> 4];
		if (section != nullptr && !section->getIsEmpty())
			return false;
	}
	return true;
}

void Chunk::removeUnknownBlocks()
{
	for (int_t section = 0; section < SECTION_COUNT; ++section)
	{
		if (storageArrays[section] != nullptr)
			storageArrays[section]->func_48711_e();
	}
}

void Chunk::populateChunk(IChunkProvider *provider, IChunkProvider *generator, int_t chunkX, int_t chunkZ)
{
	if (provider == nullptr || generator == nullptr)
		return;
	const int_t east = JavaArithmetic::intAdd(chunkX, 1);
	const int_t west = JavaArithmetic::intSub(chunkX, 1);
	const int_t south = JavaArithmetic::intAdd(chunkZ, 1);
	const int_t north = JavaArithmetic::intSub(chunkZ, 1);
	if (!isTerrainPopulated && provider->chunkExists(east, south) && provider->chunkExists(chunkX, south) && provider->chunkExists(east, chunkZ))
		provider->populate(generator, chunkX, chunkZ);
	if (provider->chunkExists(west, chunkZ) && !provider->provideChunk(west, chunkZ)->isTerrainPopulated && provider->chunkExists(west, south) && provider->chunkExists(chunkX, south))
		provider->populate(generator, west, chunkZ);
	if (provider->chunkExists(chunkX, north) && !provider->provideChunk(chunkX, north)->isTerrainPopulated && provider->chunkExists(east, north) && provider->chunkExists(east, chunkZ))
		provider->populate(generator, chunkX, north);
	if (provider->chunkExists(west, north) && !provider->provideChunk(west, north)->isTerrainPopulated && provider->chunkExists(chunkX, north) && provider->chunkExists(west, chunkZ))
		provider->populate(generator, west, north);
}

int_t Chunk::getPrecipitationHeight(int_t localX, int_t localZ)
{
	if (localX < 0 || localX >= 16 || localZ < 0 || localZ >= 16)
		return -1;
	const int_t index = localX | localZ << 4;
	int_t result = precipitationHeightMap[index];
	if (result != -999)
		return result;

	int_t y = std::min(getTopFilledSegment() + 15, WORLD_HEIGHT - 1);
	result = -1;
	while (y > 0 && result == -1)
	{
		const int_t blockId = getBlockID(localX, y, localZ);
		Material *material = blockId == 0 || !validBlockId(blockId) || Block::blocksList[blockId] == nullptr
			? Material::air
			: Block::blocksList[blockId]->blockMaterial;
		if (material != nullptr && !material->getIsSolid() && !material->getIsLiquid())
			--y;
		else
			result = y + 1;
	}
	precipitationHeightMap[index] = result;
	return result;
}

ChunkCoordIntPair Chunk::getChunkCoordIntPair() const
{
	return ChunkCoordIntPair(xPosition, zPosition);
}

BiomeGenBase *Chunk::func_48490_a(int_t localX, int_t localZ, WorldChunkManager *manager)
{
	if (localX < 0 || localX >= 16 || localZ < 0 || localZ >= 16)
		return BiomeGenBase::plains;

	const std::size_t index = (std::size_t)(localZ << 4 | localX);
	int_t biomeId = blockBiomeArray[index] & 0xff;
	if (biomeId == 0xff && manager != nullptr)
	{
		BiomeGenBase *biome = manager->getBiomeGenAt(JavaArithmetic::intAdd(JavaArithmetic::intShl(xPosition, 4), localX),
		                                             JavaArithmetic::intAdd(JavaArithmetic::intShl(zPosition, 4), localZ));
		if (biome != nullptr)
		{
			biomeId = biome->biomeID;
			blockBiomeArray[index] = (byte_t)(biomeId & 0xff);
		}
	}

	if (biomeId >= 0 && biomeId < BiomeGenBase::BIOME_REGISTRY_SIZE && BiomeGenBase::biomeList[biomeId] != nullptr)
		return BiomeGenBase::biomeList[biomeId];
	return BiomeGenBase::plains;
}

void Chunk::updateSkylight()
{
	if (isGapLightingUpdated && worldObj != nullptr && worldObj->worldProvider != nullptr && !worldObj->worldProvider->hasNoSky)
		updateSkylight_do();
}

void Chunk::resetRelightChecks()
{
	queuedLightChecks = 0;
}

void Chunk::enqueueRelightChecks()
{
	if (worldObj == nullptr)
		return;
	for (int_t iteration = 0; iteration < 8; ++iteration)
	{
		if (queuedLightChecks >= 4096)
			return;
		const int_t section = queuedLightChecks % 16;
		const int_t localX = queuedLightChecks / 16 % 16;
		const int_t localZ = queuedLightChecks / 256;
		++queuedLightChecks;
		const int_t worldX = JavaArithmetic::intAdd(JavaArithmetic::intShl(xPosition, 4), localX);
		const int_t worldZ = JavaArithmetic::intAdd(JavaArithmetic::intShl(zPosition, 4), localZ);

		for (int_t localY = 0; localY < 16; ++localY)
		{
			const int_t worldY = (section << 4) + localY;
			const ExtendedBlockStorage *storage = storageArrays[section];
			const bool shouldCheck =
				(storage == nullptr && (localY == 0 || localY == 15 || localX == 0 || localX == 15 || localZ == 0 || localZ == 15)) ||
				(storage != nullptr && storage->getExtBlockID(localX, localY, localZ) == 0);
			if (!shouldCheck)
				continue;

			const int offsets[6][3] = {
				{0, -1, 0}, {0, 1, 0}, {-1, 0, 0},
				{1, 0, 0}, {0, 0, -1}, {0, 0, 1}
			};
			for (int n = 0; n < 6; ++n)
			{
				const int_t nx = JavaArithmetic::intAdd(worldX, offsets[n][0]);
				const int_t ny = JavaArithmetic::intAdd(worldY, offsets[n][1]);
				const int_t nz = JavaArithmetic::intAdd(worldZ, offsets[n][2]);
				const int_t blockId = worldObj->getBlockId(nx, ny, nz);
				if (validBlockId(blockId) && Block::lightValue[blockId] > 0)
					worldObj->scheduleLightingUpdate(EnumSkyBlock::Block, nx, ny, nz, nx, ny, nz);
			}
			worldObj->scheduleLightingUpdate(EnumSkyBlock::Block, worldX, worldY, worldZ, worldX, worldY, worldZ);
			if (worldObj->worldProvider != nullptr && !worldObj->worldProvider->hasNoSky)
				worldObj->scheduleLightingUpdate(EnumSkyBlock::Sky, worldX, worldY, worldZ, worldX, worldY, worldZ);
		}
	}
}

std::vector<byte_t> &Chunk::getBiomeArray()
{
	return blockBiomeArray;
}

const std::vector<byte_t> &Chunk::getBiomeArray() const
{
	return blockBiomeArray;
}

bool Chunk::setBiomeArray(std::vector<byte_t> biomeData)
{
	if (biomeData.size() != 256u)
		return false;
	blockBiomeArray = std::move(biomeData);
	isModified = true;
	return true;
}

Random Chunk::getRandomWithSeed(long_t l)
{
	const int_t xSquared = JavaArithmetic::intFromBits(static_cast<uint_t>(xPosition) * static_cast<uint_t>(xPosition));
	const int_t xSquaredTerm = JavaArithmetic::intFromBits(static_cast<uint_t>(xSquared) * 4987142u);
	const int_t xTerm = JavaArithmetic::intFromBits(static_cast<uint_t>(xPosition) * 5947611u);
	const int_t zSquared = JavaArithmetic::intFromBits(static_cast<uint_t>(zPosition) * static_cast<uint_t>(zPosition));
	const int_t zTerm = JavaArithmetic::intFromBits(static_cast<uint_t>(zPosition) * 389711u);

	ulong_t seedBits = static_cast<ulong_t>(worldObj != nullptr ? worldObj->getRandomSeed() : 0LL);
	seedBits += static_cast<ulong_t>(static_cast<long_t>(xSquaredTerm));
	seedBits += static_cast<ulong_t>(static_cast<long_t>(xTerm));
	seedBits += static_cast<ulong_t>(static_cast<long_t>(zSquared)) * 4392871ULL;
	seedBits += static_cast<ulong_t>(static_cast<long_t>(zTerm));
	seedBits ^= static_cast<ulong_t>(l);
	return Random(JavaArithmetic::longFromBits(seedBits));
}

bool Chunk::isEmpty()
{
	return false;
}

bool Chunk::isEmptyChunk()
{
	return isEmpty();
}

void Chunk::remapBlocks()
{
	for (int_t section = 0; section < SECTION_COUNT; ++section)
	{
		ExtendedBlockStorage *storage = storageArrays[section];
		if (storage == nullptr)
			continue;
		std::vector<byte_t> &blocks = storage->func_48692_g();
		ChunkBlockMap::remapBlockArray(blocks.data(), (int_t)blocks.size());
		storage->func_48708_d();
		++blockSectionRevision[section];
	}
}
