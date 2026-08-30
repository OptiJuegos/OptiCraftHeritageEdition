#pragma once

#include "java/Type.h"

class TileEntity;
class Material;
class WorldChunkManager;
class BiomeGenBase;

// net.minecraft.src.IBlockAccess
class IBlockAccess
{
public:
	virtual ~IBlockAccess() = default;

	virtual int_t getBlockId(int_t i, int_t j, int_t k) = 0;
	virtual TileEntity *getBlockTileEntity(int_t i, int_t j, int_t k) = 0;
	virtual int_t getLightBrightnessForSkyBlocks(int_t i, int_t j, int_t k, int_t minimumBlockLight) = 0;
	virtual float getBrightness(int_t i, int_t j, int_t k, int_t l) = 0;
	virtual float getLightBrightness(int_t i, int_t j, int_t k) = 0;
	virtual int_t getBlockMetadata(int_t i, int_t j, int_t k) = 0;
	virtual Material *getBlockMaterial(int_t i, int_t j, int_t k) = 0;
	virtual bool isBlockOpaqueCube(int_t i, int_t j, int_t k) = 0;
	virtual bool isBlockNormalCube(int_t i, int_t j, int_t k) = 0;
	virtual bool isAirBlock(int_t i, int_t j, int_t k) = 0;
	virtual BiomeGenBase *getBiomeGenForCoords(int_t i, int_t k) = 0;
	virtual int_t getHeight() = 0;
	virtual bool func_48452_a() = 0;
	virtual WorldChunkManager *getWorldChunkManager() = 0;
};
