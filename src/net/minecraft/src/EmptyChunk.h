#pragma once

#include <vector>
#include <typeinfo>
#include "Chunk.h"
#include "java/Type.h"

class World;
class Entity;
class TileEntity;
class AxisAlignedBB;
class EnumSkyBlock;

// net.minecraft.src.EmptyChunk
class EmptyChunk : public Chunk
{
public:
	EmptyChunk(World *world, int_t i, int_t j);
	EmptyChunk(World *world, std::vector<byte_t> abyte0, int_t i, int_t j);

	bool isAtLocation(int_t i, int_t j) override;
	int_t getHeightValue(int_t i, int_t j) override;

	void initBlockLight() override;
	void generateHeightMap() override;
	void generateSkylightMap() override;
	void onChunkLoadData() override;

	int_t getBlockID(int_t i, int_t j, int_t k) override;
	int_t getBlockLightOpacity(int_t i, int_t j, int_t k) override;
	bool setBlockIDWithMetadata(int_t i, int_t j, int_t k, int_t l, int_t i1) override;
	bool setBlockID(int_t i, int_t j, int_t k, int_t l) override;
	int_t getBlockMetadata(int_t i, int_t j, int_t k) override;
	bool setBlockMetadata(int_t i, int_t j, int_t k, int_t l) override;

	int_t getSavedLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k) override;
	void  setLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l) override;
	int_t getBlockLightValue(int_t i, int_t j, int_t k, int_t l) override;

	void addEntity(Entity *entity) override;
	void removeEntity(Entity *entity) override;
	void removeEntityAtIndex(Entity *entity, int_t i) override;
	bool canBlockSeeTheSky(int_t i, int_t j, int_t k) override;

	TileEntity *getChunkBlockTileEntity(int_t i, int_t j, int_t k) override;
	void addTileEntity(TileEntity *tileentity) override;
	void setChunkBlockTileEntity(int_t i, int_t j, int_t k, TileEntity *tileentity) override;
	void removeChunkBlockTileEntity(int_t i, int_t j, int_t k) override;

	void onChunkLoad() override;
	void onChunkUnload() override;
	void setChunkModified() override;

	void getEntitiesWithinAABBForEntity(Entity *entity, AxisAlignedBB *axisalignedbb, std::vector<Entity *> &list) override;
	void getEntitiesOfTypeWithinAAAB(const std::type_info &type, AxisAlignedBB *axisalignedbb, std::vector<Entity *> &list) override;

	bool needsSaving(bool flag) override;
	int_t setChunkData(byte_t *abyte0, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1, int_t k1) override;

	bool isEmpty() override;
	bool isEmptyChunk() override;
	bool getAreLevelsEmpty(int_t minY, int_t maxY) const override;
};
