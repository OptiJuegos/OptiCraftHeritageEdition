#pragma once
#include "Block.h"
#include <vector>

class EntityLiving;
class EntityPlayer;
class Entity;
class Vec3D;

// net.minecraft.src.BlockStairs
class BlockStairs : public Block
{
public:
    BlockStairs(int_t i, Block *block);
    void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
    AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
    bool isOpaqueCube() override;
    bool renderAsNormalBlock() override;
    int_t getRenderType() override;
    bool shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
    bool usesDefaultFaceCulling() const override { return false; }
    void getCollidingBoundingBoxes(World *world, int_t i, int_t j, int_t k, AxisAlignedBB *axisalignedbb, std::vector<AxisAlignedBB*> &arraylist) override;
    void randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
    void onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
    void onBlockDestroyedByPlayer(World *world, int_t i, int_t j, int_t k, int_t l) override;
    int_t getMixedBrightnessForBlock(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
    float getBlockBrightness(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
    float getExplosionResistance(Entity *entity) override;
    int_t getRenderBlockPass() override;
    int_t idDropped(int_t i, Random &random) override;
    int_t quantityDropped(Random &random) override;
    int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
    int_t getBlockTextureFromSide(int_t i) override;
    int_t getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
    int_t tickRate() override;
    AxisAlignedBB *getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
    void velocityToAddToEntity(World *world, int_t i, int_t j, int_t k, Entity *entity, Vec3D *vec3d) override;
    bool isCollidable() override;
    bool canCollideCheck(int_t i, bool flag) override;
    bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
    void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
    void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;
    void dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f) override;
    void onEntityWalking(World *world, int_t i, int_t j, int_t k, Entity *entity) override;
    void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
    bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
    void onBlockDestroyedByExplosion(World *world, int_t i, int_t j, int_t k) override;
    void onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving) override;
    void onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t side) override;

private:
    Block *modelBlock;
};
