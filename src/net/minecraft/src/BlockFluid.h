#pragma once

#include "Block.h"

class Material;

// net.minecraft.src.BlockFluid
class BlockFluid : public Block
{
protected:
	BlockFluid(int_t i, Material *material);

public:
	int_t colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	static float getPercentAir(int_t i);
	static float getFluidHeightPercent(int_t i);
	int_t getBlockTextureFromSide(int_t i) override;

protected:
	int_t getFlowDecay(World *world, int_t i, int_t j, int_t k);
	int_t getEffectiveFlowDecay(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k);

public:
	bool renderAsNormalBlock() override;
	bool isOpaqueCube() override;
	bool canCollideCheck(int_t i, bool flag) override;
	bool getBlocksMovement(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	int_t getBlockColor() override;
	int_t getMixedBrightnessForBlock(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	bool isBlockSolid(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
	bool getIsBlockSolid(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
	bool shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
	bool usesDefaultFaceCulling() const override { return false; }
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	int_t getRenderType() override;
	int_t idDropped(int_t i, Random &random) override;
	int_t quantityDropped(Random &random) override;
	void velocityToAddToEntity(World *world, int_t i, int_t j, int_t k, Entity *entity, Vec3D *vec3d) override;
	int_t tickRate() override;
	float getBlockBrightness(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	int_t getRenderBlockPass() override;
	void randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	static double getFlowDirection(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, Material *material);
	static float getFlowDirectionFloat(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, Material *material);
	static double func_293_a(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, Material *material);
	void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;

protected:
	void triggerLavaMixEffects(World *world, int_t i, int_t j, int_t k);

private:
	static Vec3D *getFlowVectorForMaterial(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, Material *material);
	Vec3D *getFlowVector(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k);
	void checkForHarden(World *world, int_t i, int_t j, int_t k);
};
