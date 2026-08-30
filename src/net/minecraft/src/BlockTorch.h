#pragma once

#include "Block.h"

class AxisAlignedBB;
class Vec3D;
class MovingObjectPosition;

// net.minecraft.src.BlockTorch
class BlockTorch : public Block
{
public:
	BlockTorch(int_t i, int_t j);
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	int_t getRenderType() override;
	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	void onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t l) override;
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	MovingObjectPosition *collisionRayTrace(World *world, int_t i, int_t j, int_t k, Vec3D *vec3d, Vec3D *vec3d1) override;
	void randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random) override;

private:
	bool getTorchTexture(World *world, int_t i, int_t j, int_t k);
	bool dropTorchIfCantStay(World *world, int_t i, int_t j, int_t k);
};
