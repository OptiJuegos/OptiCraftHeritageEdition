#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>

#include "java/Type.h"

class Block;
class BlockStem;
class EntityFX;
class IBlockAccess;
class RenderEngine;

class CustomColorizer
{
public:
	static void update(RenderEngine *engine);
	static int_t getColorMultiplier(Block *block, IBlockAccess *access, int_t x, int_t y, int_t z);
	static int_t getFluidColor(Block *block, IBlockAccess *access, int_t x, int_t y, int_t z);
	static int_t getRedstoneColor(int_t level);
	static int_t getStemColorMultiplier(BlockStem *stem, IBlockAccess *access, int_t x, int_t y, int_t z);
	static int_t getLilypadColor();
	static void updatePortalFX(EntityFX *fx);
	static void updateWaterFX(EntityFX *fx, IBlockAccess *access);
	static void updateReddustFX(EntityFX *fx, IBlockAccess *access, double x, double y, double z);
	static void updateMyceliumFX(EntityFX *fx);

private:
	static std::vector<int_t> grassColors;
	static std::vector<int_t> waterColors;
	static std::vector<int_t> foliageColors;
	static std::vector<int_t> foliagePineColors;
	static std::vector<int_t> foliageBirchColors;
	static std::vector<int_t> swampGrassColors;
	static std::vector<int_t> swampFoliageColors;
	static std::vector<int_t> redstoneColors;
	static std::vector<int_t> stemColors;
	static std::vector<int_t> myceliumParticleColors;
	static std::vector<std::vector<int_t>> paletteColors;
	static std::array<std::array<int_t, 16>, 256> blockPalettes;
	static bool hasBlockPalettes;
	static int_t particleWaterColor;
	static int_t particlePortalColor;
	static int_t lilyPadColor;

	static std::vector<int_t> loadColors(RenderEngine *engine, const std::string &path, int_t expectedLength);
	static int_t getCustomColor(const std::vector<int_t> &colors, IBlockAccess *access, int_t x, int_t z);
	static int_t getSmoothColor(const std::vector<int_t> &colors, IBlockAccess *access, int_t x, int_t z);
	static int_t getSmoothBlockColor(Block *block, IBlockAccess *access, int_t x, int_t y, int_t z, const std::vector<int_t> *colors, const std::vector<int_t> *swampColors);
	static void readColorProperties(RenderEngine *engine);
	static void readCustomPalettes(RenderEngine *engine, const std::map<std::string, std::string> &properties);
	static void setParticleColor(EntityFX *fx, int_t color);
};
