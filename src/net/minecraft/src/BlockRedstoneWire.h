#pragma once

#include "Block.h"
#include "ChunkPosition.h"
#include "java/HashSet.h"

// net.minecraft.src.BlockRedstoneWire
class BlockRedstoneWire : public Block
{
public:
	BlockRedstoneWire(int_t i, int_t j);

	int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	int_t getRenderType() override;
	int_t colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
	void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	int_t idDropped(int_t i, Random &random) override;
	bool isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l) override;
	bool isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
	bool canProvidePower() override;
	void randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random) override;

	static bool isPowerProviderOrWire(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l);

private:
	struct ChunkPositionHash
	{
		std::size_t operator()(const ChunkPosition &position) const
		{
			return static_cast<std::size_t>(static_cast<uint_t>(position.hashCode()));
		}
	};
	struct ChunkPositionEqual
	{
		bool operator()(const ChunkPosition &a, const ChunkPosition &b) const { return a.equals(b); }
	};

	void updateAndPropagateCurrentStrength(World *world, int_t i, int_t j, int_t k);
	void updateCurrentStrength(World *world, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1);
	void notifyWireNeighborsOfNeighborChange(World *world, int_t i, int_t j, int_t k);
	int_t getMaxCurrentStrength(World *world, int_t i, int_t j, int_t k, int_t l);

	bool wiresProvidePower;
	JavaHashSet<ChunkPosition, ChunkPositionHash, ChunkPositionEqual> pendingWireUpdates;
};
