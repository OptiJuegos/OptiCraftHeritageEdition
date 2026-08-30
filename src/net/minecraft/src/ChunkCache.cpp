#include "ChunkCache.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include "java/Arithmetic.h"
#include "platform/PlatformTuning.h"

#include "World.h"
#include "Chunk.h"
#include "ExtendedBlockStorage.h"
#include "EnumSkyBlock.h"
#include "WorldHeight.h"
#include "Block.h"
#include "Material.h"
#include "WorldProvider.h"

#if PLATFORM_FAST_CHUNK_BLOCK_READS
namespace
{
	inline int_t sectionStorageIndex(int_t localX, int_t localY, int_t localZ)
	{
		return localY << 8 | localZ << 4 | localX;
	}

	inline int_t readNibbleAt(const byte_t *base, int_t index)
	{
		const byte_t packed = base[index >> 1];
		return (index & 1) == 0 ? (packed & 0xf) : ((packed >> 4) & 0xf);
	}
}
#endif

ChunkCache::ChunkCache(World *world, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1)
	: worldObj(world), levelsEmpty(true)
{
	chunkX = JavaArithmetic::intShr(i, 4);
	chunkZ = JavaArithmetic::intShr(k, 4);
	int_t k1 = JavaArithmetic::intShr(l, 4);
	int_t l1 = JavaArithmetic::intShr(j1, 4);
	chunkArrayWidth = JavaArithmetic::intAdd(JavaArithmetic::intSub(k1, chunkX), 1);
	chunkArrayDepth = JavaArithmetic::intAdd(JavaArithmetic::intSub(l1, chunkZ), 1);

	// Java stores this as a two-dimensional array, so each dimension is checked
	// independently by the VM. The flat C++ representation must additionally
	// validate the product before converting it to size_t; otherwise a wrapped
	// signed product can become a huge allocation or a too-small backing store.
	if (chunkArrayWidth <= 0 || chunkArrayDepth <= 0)
		throw std::length_error("Invalid ChunkCache dimensions");
	const uint64_t cellCount64 = static_cast<uint64_t>(static_cast<uint32_t>(chunkArrayWidth))
		* static_cast<uint64_t>(static_cast<uint32_t>(chunkArrayDepth));
	if (cellCount64 > static_cast<uint64_t>(std::numeric_limits<int_t>::max()))
		throw std::length_error("ChunkCache dimensions are too large");
	chunkCellCount = static_cast<size_t>(cellCount64);

	// Inline storage for every real caller (3x3 for a mesh build, up to 6x6 for
	// a pathfind); the heap vector only exists so an oversized span stays
	// correct rather than overrunning.
	if (chunkArrayWidth <= kInlineDim && chunkArrayDepth <= kInlineDim)
	{
		chunkArray = chunkInline;
	}
	else
	{
		chunkHeap.assign(chunkCellCount, nullptr);
		chunkArray = chunkHeap.data();
	}
	for (size_t c = 0; c < chunkCellCount; ++c)
		chunkArray[c] = nullptr;

	for (int_t i2 = chunkX; i2 <= k1; i2 = JavaArithmetic::intAdd(i2, 1))
	{
		for (int_t j2 = chunkZ; j2 <= l1; j2 = JavaArithmetic::intAdd(j2, 1))
		{
			// Never generate from a ChunkCache. getChunkFromChunkCoords() would
			// synchronously generate a missing chunk; for pathfinding (whose cache
			// spans ~64 blocks, well past the loaded area) that meant generating
			// chunks mid entity-tick -- a multi-ms stall and a reentrancy hazard
			// (generate -> populate -> spawn entities while updateEntities iterates).
			// Missing chunks are left null and read as air by the accessors below.
			Chunk *chunk = world->chunkExists(i2, j2) ? world->getChunkFromChunkCoords(i2, j2) : nullptr;
			const int_t localX = JavaArithmetic::intSub(i2, chunkX);
			const int_t localZ = JavaArithmetic::intSub(j2, chunkZ);
			if (localX >= 0 && localX < chunkArrayWidth && localZ >= 0 && localZ < chunkArrayDepth)
				chunkArray[cellIndex(localX, localZ)] = chunk;
			if (chunk != nullptr && !chunk->getAreLevelsEmpty(j, i1))
				levelsEmpty = false;
			if (j2 == l1)
				break;
		}
		if (i2 == k1)
			break;
	}

#if PLATFORM_FAST_CHUNK_BLOCK_READS
	fastBasesValid = resolveFastBases();
#endif
}

ChunkCache::~ChunkCache()
{
}

size_t ChunkCache::cellIndex(int_t localX, int_t localZ) const
{
	return static_cast<size_t>(localX) * static_cast<size_t>(chunkArrayDepth)
		+ static_cast<size_t>(localZ);
}

#if PLATFORM_FAST_CHUNK_BLOCK_READS
bool ChunkCache::resolveFastBases()
{
	if (chunkArrayWidth > kInlineDim || chunkArrayDepth > kInlineDim)
		return false;

	for (size_t c = 0; c < chunkCellCount; ++c)
	{
		for (int_t section = 0; section < WorldHeight::SECTION_COUNT; ++section)
			sectionBase[c * WorldHeight::SECTION_COUNT + section] = nullptr;

		Chunk *chunk = chunkArray[c];
		if (chunk == nullptr || chunk->isEmptyChunk())
			continue;

		for (int_t section = 0; section < WorldHeight::SECTION_COUNT; ++section)
			sectionBase[c * WorldHeight::SECTION_COUNT + section] = chunk->getBlockStorage(section);
	}
	return true;
}
#endif

bool ChunkCache::hasResidentChunkAtBlock(int_t i, int_t k) const
{
	const int_t localX = JavaArithmetic::intSub(JavaArithmetic::intShr(i, 4), chunkX);
	const int_t localZ = JavaArithmetic::intSub(JavaArithmetic::intShr(k, 4), chunkZ);
	if (localX < 0 || localX >= chunkArrayWidth || localZ < 0 || localZ >= chunkArrayDepth)
		return false;

	Chunk *chunk = chunkArray[cellIndex(localX, localZ)];
	return chunk != nullptr && !chunk->isEmptyChunk();
}

int_t ChunkCache::getBlockId(int_t i, int_t j, int_t k)
{
	if (j < WorldHeight::MIN_Y || j >= WorldHeight::HEIGHT)
		return 0;
	const int_t l = JavaArithmetic::intSub(JavaArithmetic::intShr(i, 4), chunkX);
	const int_t i1 = JavaArithmetic::intSub(JavaArithmetic::intShr(k, 4), chunkZ);
	if (l < 0 || l >= chunkArrayWidth || i1 < 0 || i1 >= chunkArrayDepth)
		return 0;
	const size_t cell = cellIndex(l, i1);
#if PLATFORM_FAST_CHUNK_BLOCK_READS
	if (fastBasesValid)
	{
		const ExtendedBlockStorage *section = sectionBase[cell * WorldHeight::SECTION_COUNT + (j >> 4)];
		return section != nullptr ? section->getExtBlockID(i & 0xf, j & 0xf, k & 0xf) : 0;
	}
#endif
	Chunk *chunk = chunkArray[cell];
	return chunk != nullptr ? chunk->getBlockID(i & 0xf, j, k & 0xf) : 0;
}

TileEntity *ChunkCache::getBlockTileEntity(int_t i, int_t j, int_t k)
{
	if (j < WorldHeight::MIN_Y || j >= WorldHeight::HEIGHT)
		return nullptr;
	int_t l  = JavaArithmetic::intSub(JavaArithmetic::intShr(i, 4), chunkX);
	int_t i1 = JavaArithmetic::intSub(JavaArithmetic::intShr(k, 4), chunkZ);
	if (l < 0 || l >= chunkArrayWidth || i1 < 0 || i1 >= chunkArrayDepth)
		return nullptr;
	Chunk *chunk = chunkArray[cellIndex(l, i1)];
	if (chunk == nullptr) return nullptr;
	return chunk->getChunkBlockTileEntity(i & 0xf, j, k & 0xf);
}

int_t ChunkCache::getLightBrightnessForSkyBlocks(int_t i, int_t j, int_t k, int_t minimumBlockLight)
{
	int_t skyLight = getSkyBlockTypeBrightness(EnumSkyBlock::Sky, i, j, k);
	int_t blockLight = getSkyBlockTypeBrightness(EnumSkyBlock::Block, i, j, k);
	if (blockLight < minimumBlockLight)
		blockLight = minimumBlockLight;
	return (skyLight << 20) | (blockLight << 4);
}

int_t ChunkCache::getSkyBlockTypeBrightness(EnumSkyBlock *type, int_t i, int_t j, int_t k)
{
	if (type == nullptr)
		return 0;
	if (j < 0)
		j = 0;
	if (j >= WorldHeight::HEIGHT)
		j = WorldHeight::MAX_Y;
	if (i < -30000000 || k < -30000000 || i >= 30000000 || k > 30000000)
		return type->defaultLightValue;

	const int_t blockId = getBlockId(i, j, k);
	if (blockId >= 0 && blockId < Block::BLOCK_REGISTRY_SIZE && Block::useNeighborBrightness[blockId])
	{
		int_t brightness = getSpecialBlockBrightness(type, i, j + 1, k);
		const int_t east = getSpecialBlockBrightness(type, i + 1, j, k);
		const int_t west = getSpecialBlockBrightness(type, i - 1, j, k);
		const int_t south = getSpecialBlockBrightness(type, i, j, k + 1);
		const int_t north = getSpecialBlockBrightness(type, i, j, k - 1);
		if (east > brightness) brightness = east;
		if (west > brightness) brightness = west;
		if (south > brightness) brightness = south;
		if (north > brightness) brightness = north;
		return brightness;
	}

	return getSpecialBlockBrightness(type, i, j, k);
}

int_t ChunkCache::getSpecialBlockBrightness(EnumSkyBlock *type, int_t i, int_t j, int_t k)
{
	if (type == nullptr)
		return 0;
	if (j < 0)
		j = 0;
	if (j >= WorldHeight::HEIGHT)
		j = WorldHeight::MAX_Y;
	if (i < -30000000 || k < -30000000 || i >= 30000000 || k > 30000000)
		return type->defaultLightValue;

	const int_t localChunkX = JavaArithmetic::intSub(JavaArithmetic::intShr(i, 4), chunkX);
	const int_t localChunkZ = JavaArithmetic::intSub(JavaArithmetic::intShr(k, 4), chunkZ);
	if (localChunkX < 0 || localChunkX >= chunkArrayWidth ||
		localChunkZ < 0 || localChunkZ >= chunkArrayDepth)
		return type->defaultLightValue;

	Chunk *chunk = chunkArray[cellIndex(localChunkX, localChunkZ)];
	return chunk != nullptr
		? chunk->getSavedLightValue(type, i & 15, j, k & 15)
		: type->defaultLightValue;
}


float ChunkCache::getBrightness(int_t i, int_t j, int_t k, int_t l)
{
#if PLATFORM_CONSOLE_LOW
#if PLATFORM_FORCE_FULLBRIGHT_TERRAIN
	// The per-face color multiplier (non-AO path) reads this for every cube face.
	// Under fullbright terrain the real skylight is unused and is often still 0 on
	// lazily generated chunks, which multiplies the face to black. Match getLightBrightness().
	(void)i; (void)j; (void)k; (void)l;
	return 1.0f;
#endif
#endif
	int_t i1 = getLightValue(i, j, k);
	if (i1 < l) i1 = l;
	return worldObj->worldProvider->lightBrightnessTable[i1];
}

float ChunkCache::getLightBrightness(int_t i, int_t j, int_t k)
{
#if PLATFORM_CONSOLE_LOW
#if PLATFORM_FORCE_FULLBRIGHT_TERRAIN
	(void)i; (void)j; (void)k;
	return 1.0f;
#endif
#endif
	return worldObj->worldProvider->lightBrightnessTable[getLightValue(i, j, k)];
}

int_t ChunkCache::getLightValue(int_t i, int_t j, int_t k)
{
	return getLightValueExt(i, j, k, true);
}

int_t ChunkCache::getLightValueExt(int_t i, int_t j, int_t k, bool flag)
{
	// bounds check using the Java hex literals (matching original)
	if (i < -30000000 || k < -30000000 || i >= 30000000 || k > 30000000)
		return 15;
	if (flag)
	{
		int_t l = getBlockId(i, j, k);
		if (l == Block::stairSingle->blockID
			|| l == Block::tilledField->blockID
			|| l == Block::stairCompactPlanks->blockID
			|| l == Block::stairCompactCobblestone->blockID)
		{
			int_t k1 = getLightValueExt(i, j + 1, k, false);
			int_t i2 = getLightValueExt(i + 1, j, k, false);
			int_t j2 = getLightValueExt(i - 1, j, k, false);
			int_t k2 = getLightValueExt(i, j, k + 1, false);
			int_t l2 = getLightValueExt(i, j, k - 1, false);
			if (i2 > k1) k1 = i2;
			if (j2 > k1) k1 = j2;
			if (k2 > k1) k1 = k2;
			if (l2 > k1) k1 = l2;
			return k1;
		}
	}
	if (j < 0) return 0;
	if (j >= WorldHeight::HEIGHT)
	{
		int_t i1 = 15 - worldObj->skylightSubtracted;
		if (i1 < 0) i1 = 0;
		return i1;
	}
	int_t j1 = JavaArithmetic::intSub(JavaArithmetic::intShr(i, 4), chunkX);
	int_t l1 = JavaArithmetic::intSub(JavaArithmetic::intShr(k, 4), chunkZ);
	if (j1 < 0 || j1 >= chunkArrayWidth || l1 < 0 || l1 >= chunkArrayDepth)
		return 0;
	const size_t cell = cellIndex(j1, l1);
#if PLATFORM_FAST_CHUNK_BLOCK_READS
	if (fastBasesValid)
	{
		const ExtendedBlockStorage *section = sectionBase[cell * WorldHeight::SECTION_COUNT + (j >> 4)];
		if (section == nullptr)
		{
			const int_t sky = worldObj->worldProvider->hasNoSky ? 0 : 15 - worldObj->skylightSubtracted;
			return sky > 0 ? sky : 0;
		}
		int_t light = worldObj->worldProvider->hasNoSky ? 0 : section->getExtSkylightValue(i & 0xf, j & 0xf, k & 0xf);
		if (light > 0)
			Chunk::isLit = true;
		light -= worldObj->skylightSubtracted;
		const int_t emitted = section->getExtBlocklightValue(i & 0xf, j & 0xf, k & 0xf);
		return emitted > light ? emitted : light;
	}
#endif
	Chunk *chunk = chunkArray[cell];
	if (chunk == nullptr) return 0;
	return chunk->getBlockLightValue(i & 0xf, j, k & 0xf, worldObj->skylightSubtracted);
}

int_t ChunkCache::getBlockMetadata(int_t i, int_t j, int_t k)
{
	if (j < 0) return 0;
	if (j >= WorldHeight::HEIGHT) return 0;
	int_t l  = JavaArithmetic::intSub(JavaArithmetic::intShr(i, 4), chunkX);
	int_t i1 = JavaArithmetic::intSub(JavaArithmetic::intShr(k, 4), chunkZ);
	if (l < 0 || l >= chunkArrayWidth || i1 < 0 || i1 >= chunkArrayDepth)
		return 0;
	const size_t cell = cellIndex(l, i1);
#if PLATFORM_FAST_CHUNK_BLOCK_READS
	if (fastBasesValid)
	{
		const ExtendedBlockStorage *section = sectionBase[cell * WorldHeight::SECTION_COUNT + (j >> 4)];
		return section != nullptr ? section->getExtBlockMetadata(i & 0xf, j & 0xf, k & 0xf) : 0;
	}
#endif
	Chunk *chunk = chunkArray[cell];
	if (chunk == nullptr) return 0;
	return chunk->getBlockMetadata(i & 0xf, j, k & 0xf);
}

Material *ChunkCache::getBlockMaterial(int_t i, int_t j, int_t k)
{
	int_t l = getBlockId(i, j, k);
	if (l == 0)
		return Material::air;
	return Block::blocksList[l]->blockMaterial;
}

WorldChunkManager *ChunkCache::getWorldChunkManager()
{
	return worldObj->getWorldChunkManager();
}

bool ChunkCache::isBlockOpaqueCube(int_t i, int_t j, int_t k)
{
#if PLATFORM_CONSOLE_LOW
	// A neighbour lying in an un-generated (missing) chunk must occlude the face
	// pointing at it. Otherwise the loaded-area boundary renders a "wall" of water
	// or terrain hanging against the sky (the ocean cross-section at the world
	// edge). Treat missing chunks within the world height as opaque so those edge
	// faces are culled -- there is nothing real to see behind them yet. j outside
	// 0..255 stays air so the sky above and the void below are untouched, and
	// collision still reads air via getBlockId() so no invisible wall is created.
	if (j >= WorldHeight::MIN_Y && j < WorldHeight::HEIGHT)
	{
		int_t l  = JavaArithmetic::intSub(JavaArithmetic::intShr(i, 4), chunkX);
		int_t i1 = JavaArithmetic::intSub(JavaArithmetic::intShr(k, 4), chunkZ);
		if (l < 0 || l >= chunkArrayWidth || i1 < 0 || i1 >= chunkArrayDepth ||
			chunkArray[cellIndex(l, i1)] == nullptr)
			return true;
	}
#endif
	Block *block = Block::blocksList[getBlockId(i, j, k)];
	if (block == nullptr) return false;
	return block->isOpaqueCube();
}

bool ChunkCache::isBlockNormalCube(int_t i, int_t j, int_t k)
{
	Block *block = Block::blocksList[getBlockId(i, j, k)];
	if (block == nullptr) return false;
	return block->blockMaterial->getIsSolid() && block->renderAsNormalBlock();
}


bool ChunkCache::isAirBlock(int_t i, int_t j, int_t k)
{
	const int_t blockId = getBlockId(i, j, k);
	return blockId <= 0 || blockId >= Block::BLOCK_REGISTRY_SIZE || Block::blocksList[blockId] == nullptr;
}

BiomeGenBase *ChunkCache::getBiomeGenForCoords(int_t i, int_t k)
{
	return worldObj != nullptr ? worldObj->getBiomeGenForCoords(i, k) : nullptr;
}

int_t ChunkCache::getHeight()
{
	return WorldHeight::HEIGHT;
}

bool ChunkCache::func_48452_a()
{
	return levelsEmpty;
}
