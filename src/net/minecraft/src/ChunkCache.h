#pragma once

#include <cstddef>
#include <vector>
#include "IBlockAccess.h"
#include "java/Type.h"
#include "platform/PlatformTuning.h"

class World;
class Chunk;
class TileEntity;
class Material;
class ExtendedBlockStorage;
class WorldChunkManager;
class BiomeGenBase;
class EnumSkyBlock;

// net.minecraft.src.ChunkCache
class ChunkCache : public IBlockAccess
{
public:
	ChunkCache(World *world, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1);
	~ChunkCache();

	int_t getBlockId(int_t i, int_t j, int_t k) override;
	bool hasResidentChunkAtBlock(int_t i, int_t k) const;
	TileEntity *getBlockTileEntity(int_t i, int_t j, int_t k) override;
	int_t getLightBrightnessForSkyBlocks(int_t i, int_t j, int_t k, int_t minimumBlockLight) override;
	int_t getSkyBlockTypeBrightness(EnumSkyBlock *type, int_t i, int_t j, int_t k);
	int_t getSpecialBlockBrightness(EnumSkyBlock *type, int_t i, int_t j, int_t k);

	float getBrightness(int_t i, int_t j, int_t k, int_t l) override;
	float getLightBrightness(int_t i, int_t j, int_t k) override;
	int_t getLightValue(int_t i, int_t j, int_t k);
	int_t getLightValueExt(int_t i, int_t j, int_t k, bool flag);

	int_t getBlockMetadata(int_t i, int_t j, int_t k) override;
	Material *getBlockMaterial(int_t i, int_t j, int_t k) override;
	WorldChunkManager *getWorldChunkManager() override;

	bool isBlockOpaqueCube(int_t i, int_t j, int_t k) override;
	bool isBlockNormalCube(int_t i, int_t j, int_t k) override;
	bool isAirBlock(int_t i, int_t j, int_t k) override;
	BiomeGenBase *getBiomeGenForCoords(int_t i, int_t k) override;
	int_t getHeight() override;
	bool func_48452_a() override;

private:
	// Flat [x * chunkArrayDepth + z], not a vector of vectors.
	//
	// A ChunkCache is constructed on every chunk-mesh build step and on every
	// pathfind, and the old std::vector<std::vector<Chunk*>> cost four heap
	// allocations each time (the outer vector, the temporary inner one, and its
	// copies) on a 32MB heap that is already fragmentation-sensitive. Worse,
	// getBlockId is the hottest accessor in the port -- a section build issues
	// roughly 25k of them, since RenderBlocks probes six neighbours per block --
	// and every one of those paid a double indirection plus two bounds
	// computations to reach the pointer.
	//
	// The mesh case is 3x3 and the pathfinder's is at most 6x6, so an inline
	// array covers every real caller with no allocation at all. chunkHeap is a
	// correctness fallback only: it is used if some future caller asks for a
	// span wider than kInlineDim, and chunkArray always points at whichever
	// storage is live.
	static const int_t kInlineDim = 8;

#if PLATFORM_FAST_CHUNK_BLOCK_READS
	// Raw array bases of the resident chunks, one entry per chunkArray cell,
	// resolved once by the constructor so the accessors below can index them
	// without the Chunk vtable. See PLATFORM_FAST_CHUNK_BLOCK_READS.
	//
	// A base is null whenever the corresponding chunk is absent, an EmptyChunk
	// or not fully sized, which is precisely the set of chunks whose virtual
	// accessors already returned 0. chunkArray keeps the Chunk pointers
	// untouched, so isBlockOpaqueCube's missing-chunk rule is unaffected.
	//
	// resolveFastBases() reports false for a span wider than the inline array
	// (nothing in the game asks for one, but ChunkCache stays correct if
	// something ever does); the accessors then take the original virtual path.
	//
	// Lifetime: these alias storage a Chunk owns, so they are valid exactly as
	// long as the Chunk pointers in chunkArray already had to be -- a ChunkCache
	// is a short-lived local and chunks are only unloaded from the world tick.
	// The arrays themselves are allocated once at chunk construction and never
	// resized, so a block write cannot move them.
	bool resolveFastBases();

	bool fastBasesValid;
	// One cached section pointer per resident chunk/vertical section. This keeps
	// the low-console hot path allocation-free without retaining the obsolete
	// Beta 1.7.3 32 KB contiguous chunk layout.
	const ExtendedBlockStorage *sectionBase[kInlineDim * kInlineDim * 16];
#endif

	int_t chunkX;
	int_t chunkZ;
	int_t chunkArrayWidth;
	int_t chunkArrayDepth;
	size_t chunkCellCount;
	Chunk *chunkInline[kInlineDim * kInlineDim];
	std::vector<Chunk *> chunkHeap;
	size_t cellIndex(int_t localX, int_t localZ) const;

	Chunk **chunkArray;
	World *worldObj;
	bool levelsEmpty;
};
