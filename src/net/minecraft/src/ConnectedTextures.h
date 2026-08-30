#pragma once

#include <vector>

#include "java/Type.h"
#include "ConnectedProperties.h"

class Block;
class IBlockAccess;
class RenderEngine;

class ConnectedTextures
{
public:
	static void update(RenderEngine *engine);
	static int_t getConnectedTexture(IBlockAccess *blockAccess, Block *block, int_t x, int_t y, int_t z, int_t side, int_t tileNum);
	static bool isConnectedGlassPanes();
	static int_t getTerrainTextureId();
	static int_t getCtmTextureId();
	static int_t getGlassPaneTexture(bool linkP, bool linkN, bool linkYp, bool linkYn);
	static int_t getReverseGlassPaneTexture(int_t texture);

private:
	static RenderEngine *renderEngine;
	static std::vector<std::vector<ConnectedProperties>> blockProperties;
	static std::vector<std::vector<ConnectedProperties>> terrainProperties;
	static bool matchingCtmPng;
	static bool hasBlockProperties;
	static bool hasTerrainProperties;

	static void readConnectedProperties(const std::string &prefix, std::vector<std::vector<ConnectedProperties>> &out, int_t defaultConnect);
	static int_t getConnectedTexture(const ConnectedProperties &cp, IBlockAccess *blockAccess, Block *block, int_t x, int_t y, int_t z, int_t side, int_t tileNum);
	static int_t getCtm(const ConnectedProperties &cp, IBlockAccess *access, Block *block, int_t x, int_t y, int_t z, int_t side, int_t tile);
	static int_t getHorizontal(const ConnectedProperties &cp, IBlockAccess *access, Block *block, int_t x, int_t y, int_t z, int_t side, int_t tile);
	static int_t getVertical(const ConnectedProperties &cp, IBlockAccess *access, Block *block, int_t x, int_t y, int_t z, int_t side, int_t tile);
	static int_t getTop(const ConnectedProperties &cp, IBlockAccess *access, Block *block, int_t x, int_t y, int_t z, int_t side, int_t tile);
	static int_t getRandom(const ConnectedProperties &cp, int_t x, int_t y, int_t z, int_t side);
	static int_t getRepeat(const ConnectedProperties &cp, int_t x, int_t y, int_t z, int_t side);
	static bool isNeighbour(const ConnectedProperties &cp, IBlockAccess *access, int_t x, int_t y, int_t z, int_t blockId, int_t side, int_t tileNum);
	static bool metadataMatches(const ConnectedProperties &cp, int_t metadata);
	static ConnectedProperties makeDefault(const std::string &method, RenderEngine *engine);
};
