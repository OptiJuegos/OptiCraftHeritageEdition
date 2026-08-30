#include "ConnectedTextures.h"

#include <algorithm>
#include <map>

#include "Block.h"
#include "Config.h"
#include "IBlockAccess.h"
#include "OptiFineResource.h"
#include "RenderEngine.h"

RenderEngine *ConnectedTextures::renderEngine = nullptr;
std::vector<std::vector<ConnectedProperties>> ConnectedTextures::blockProperties;
std::vector<std::vector<ConnectedProperties>> ConnectedTextures::terrainProperties;
bool ConnectedTextures::matchingCtmPng = false;
bool ConnectedTextures::hasBlockProperties = false;
bool ConnectedTextures::hasTerrainProperties = false;

ConnectedProperties ConnectedTextures::makeDefault(const std::string &method, RenderEngine *engine)
{
	std::map<std::string, std::string> props;
	props["method"] = method;
	props["source"] = "/ctm.png";
	ConnectedProperties cp(props);
	cp.connect = ConnectedProperties::CONNECT_BLOCK;
	cp.isValid("/ctm.png");
	cp.textureId = engine != nullptr ? engine->getTexture(cp.source) : -1;
	return cp;
}

void ConnectedTextures::readConnectedProperties(const std::string &prefix, std::vector<std::vector<ConnectedProperties>> &out, int_t defaultConnect)
{
	static const char *suffixes[] = {"","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z"};
	out.assign(256, std::vector<ConnectedProperties>());
	for (int_t index = 0; index < 256; ++index)
	{
		for (const char *suffix : suffixes)
		{
			const std::string path = prefix + std::to_string(index) + suffix + ".properties";
			if (!OptiFineResource::exists(renderEngine, path))
				break;
			auto props = OptiFineResource::readProperties(renderEngine, path);
			ConnectedProperties cp(props);
			if (cp.connect == ConnectedProperties::CONNECT_NONE)
				cp.connect = defaultConnect;
			if (!cp.isValid(path))
				continue;
			if (!cp.source.empty() && cp.source[0] != '/')
				cp.source.insert(cp.source.begin(), '/');
			cp.textureId = renderEngine->getTexture(cp.source);
			out[static_cast<size_t>(index)].push_back(cp);
		}
	}
}

void ConnectedTextures::update(RenderEngine *engine)
{
	renderEngine = engine;
	blockProperties.clear();
	terrainProperties.clear();
	hasBlockProperties = false;
	hasTerrainProperties = false;
	matchingCtmPng = false;
	if (!Config::isConnectedTextures() || engine == nullptr)
		return;

	readConnectedProperties("/ctm/block", blockProperties, ConnectedProperties::CONNECT_BLOCK);
	readConnectedProperties("/ctm/terrain", terrainProperties, ConnectedProperties::CONNECT_TILE);
	for (const auto &entry : blockProperties) if (!entry.empty()) { hasBlockProperties = true; break; }
	for (const auto &entry : terrainProperties) if (!entry.empty()) { hasTerrainProperties = true; break; }
	if (engine->hasResource("/ctm.png"))
	{
		int_t ctmWidth = 0, ctmHeight = 0, terrainWidth = 0, terrainHeight = 0;
		const int_t ctmTexture = engine->getTexture("/ctm.png");
		const int_t terrainTexture = engine->getTexture("/terrain.png");
		matchingCtmPng = engine->getTextureDimensions(ctmTexture, &ctmWidth, &ctmHeight)
			&& engine->getTextureDimensions(terrainTexture, &terrainWidth, &terrainHeight)
			&& ctmWidth == terrainWidth && ctmHeight == terrainHeight;
	}

	if (!hasBlockProperties && !hasTerrainProperties && matchingCtmPng)
	{
		blockProperties.assign(256, std::vector<ConnectedProperties>());
		terrainProperties.assign(256, std::vector<ConnectedProperties>());
		if (Block::glass != nullptr)
			blockProperties[static_cast<size_t>(Block::glass->blockID)].push_back(makeDefault("ctm", engine));
		if (Block::bookShelf != nullptr)
			blockProperties[static_cast<size_t>(Block::bookShelf->blockID)].push_back(makeDefault("horizontal", engine));
		if (Block::sandStone != nullptr)
		{
			ConnectedProperties top = makeDefault("top", engine);
			top.connect = ConnectedProperties::CONNECT_TILE;
			terrainProperties[static_cast<size_t>(Block::sandStone->blockIndexInTexture)].push_back(top);
		}
		hasBlockProperties = true;
		hasTerrainProperties = true;
	}
}

int_t ConnectedTextures::getTerrainTextureId()
{
	return renderEngine != nullptr ? renderEngine->getTexture("/terrain.png") : 0;
}

int_t ConnectedTextures::getCtmTextureId()
{
	return renderEngine != nullptr ? renderEngine->getTexture("/ctm.png") : -1;
}

int_t ConnectedTextures::getReverseGlassPaneTexture(int_t texture)
{
	const int_t column = texture & 15;
	if (column == 1)
		return texture + 2;
	if (column == 3)
		return texture - 2;
	return texture;
}

int_t ConnectedTextures::getGlassPaneTexture(bool linkP, bool linkN, bool linkYp, bool linkYn)
{
	// C6's /ctm.png reserves the first 4x4 pane tiles for the horizontal
	// connection state and three additional rows for the vertical variants.
	if (linkN && linkP)
	{
		if (linkYp) return linkYn ? 34 : 50;
		return linkYn ? 18 : 2;
	}
	if (linkN)
	{
		if (linkYp) return linkYn ? 35 : 51;
		return linkYn ? 19 : 3;
	}
	if (linkP)
	{
		if (linkYp) return linkYn ? 33 : 49;
		return linkYn ? 17 : 1;
	}
	if (linkYp) return linkYn ? 32 : 48;
	return linkYn ? 16 : 0;
}

bool ConnectedTextures::metadataMatches(const ConnectedProperties &cp, int_t metadata)
{
	return cp.metadata.empty() || std::find(cp.metadata.begin(), cp.metadata.end(), metadata) != cp.metadata.end();
}

int_t ConnectedTextures::getConnectedTexture(IBlockAccess *access, Block *block, int_t x, int_t y, int_t z, int_t side, int_t tileNum)
{
	if (!Config::isConnectedTextures() || access == nullptr || block == nullptr)
		return -1;
	if (hasTerrainProperties && tileNum >= 0 && tileNum < static_cast<int_t>(terrainProperties.size()))
	{
		for (const ConnectedProperties &cp : terrainProperties[static_cast<size_t>(tileNum)])
		{
			const int_t result = getConnectedTexture(cp, access, block, x, y, z, side, tileNum);
			if (result >= 0) return result;
		}
	}
	if (hasBlockProperties && block->blockID >= 0 && block->blockID < static_cast<int_t>(blockProperties.size()))
	{
		for (const ConnectedProperties &cp : blockProperties[static_cast<size_t>(block->blockID)])
		{
			const int_t result = getConnectedTexture(cp, access, block, x, y, z, side, tileNum);
			if (result >= 0) return result;
		}
	}
	return -1;
}

int_t ConnectedTextures::getConnectedTexture(const ConnectedProperties &cp, IBlockAccess *access, Block *block, int_t x, int_t y, int_t z, int_t side, int_t tileNum)
{
	if (side >= 0 && (cp.faces & (1 << side)) == 0)
		return -1;
	if (!metadataMatches(cp, access->getBlockMetadata(x, y, z)))
		return -1;
	switch (cp.method)
	{
	case ConnectedProperties::METHOD_CTM: return getCtm(cp, access, block, x, y, z, side, tileNum);
	case ConnectedProperties::METHOD_HORIZONTAL: return getHorizontal(cp, access, block, x, y, z, side, tileNum);
	case ConnectedProperties::METHOD_VERTICAL: return getVertical(cp, access, block, x, y, z, side, tileNum);
	case ConnectedProperties::METHOD_TOP: return getTop(cp, access, block, x, y, z, side, tileNum);
	case ConnectedProperties::METHOD_RANDOM: return getRandom(cp, x, y, z, side);
	case ConnectedProperties::METHOD_REPEAT: return getRepeat(cp, x, y, z, side);
	default: return -1;
	}
}

bool ConnectedTextures::isNeighbour(const ConnectedProperties &cp, IBlockAccess *access, int_t x, int_t y, int_t z, int_t blockId, int_t side, int_t tileNum)
{
	const int_t neighbourId = access->getBlockId(x, y, z);
	if (cp.connect == ConnectedProperties::CONNECT_TILE)
	{
		Block *neighbour = neighbourId >= 0 && neighbourId < 256 ? Block::blocksList[neighbourId] : nullptr;
		return neighbour != nullptr && neighbour->getBlockTexture(access, x, y, z, side) == tileNum;
	}
	return neighbourId == blockId;
}

int_t ConnectedTextures::getRandom(const ConnectedProperties &cp, int_t x, int_t y, int_t z, int_t side)
{
	const int_t face = side < 0 ? 0 : (side / cp.symmetry) * cp.symmetry;
	const int_t random = Config::getRandom(x, y, z, face) & 0x7fffffff;
	int_t index = 0;
	if (cp.weights.empty()) index = random % static_cast<int_t>(cp.tiles.size());
	else
	{
		const int_t weighted = random % cp.sumAllWeights;
		while (index + 1 < static_cast<int_t>(cp.sumWeights.size()) && weighted >= cp.sumWeights[static_cast<size_t>(index)]) ++index;
	}
	return cp.textureId * 256 + cp.tiles[static_cast<size_t>(index)];
}

int_t ConnectedTextures::getRepeat(const ConnectedProperties &cp, int_t x, int_t y, int_t z, int_t side)
{
	int_t nx = 0, ny = 0;
	switch (side) {
	case 0: case 1: nx=x; ny=z; break;
	case 2: nx=-x-1; ny=-y; break;
	case 3: nx=x; ny=-y; break;
	case 4: nx=z; ny=-y; break;
	case 5: nx=-z-1; ny=-y; break;
	default: nx=x; ny=z; break;
	}
	nx %= cp.width; ny %= cp.height;
	if (nx < 0) nx += cp.width; if (ny < 0) ny += cp.height;
	return cp.textureId * 256 + cp.tiles[static_cast<size_t>(ny * cp.width + nx)];
}

int_t ConnectedTextures::getHorizontal(const ConnectedProperties &cp, IBlockAccess *access, Block *block, int_t x, int_t y, int_t z, int_t side, int_t tile)
{
	if (side == 0 || side == 1) return -1;
	bool left=false, right=false; const int_t id=block->blockID;
	switch(side) {
	case 2: left=isNeighbour(cp,access,x+1,y,z,id,side,tile); right=isNeighbour(cp,access,x-1,y,z,id,side,tile); break;
	case 3: left=isNeighbour(cp,access,x-1,y,z,id,side,tile); right=isNeighbour(cp,access,x+1,y,z,id,side,tile); break;
	case 4: left=isNeighbour(cp,access,x,y,z-1,id,side,tile); right=isNeighbour(cp,access,x,y,z+1,id,side,tile); break;
	case 5: left=isNeighbour(cp,access,x,y,z+1,id,side,tile); right=isNeighbour(cp,access,x,y,z-1,id,side,tile); break;
	}
	const int_t index = left ? (right ? 1 : 2) : (right ? 0 : 3);
	return cp.textureId * 256 + cp.tiles[static_cast<size_t>(index)];
}

int_t ConnectedTextures::getVertical(const ConnectedProperties &cp, IBlockAccess *access, Block *block, int_t x, int_t y, int_t z, int_t side, int_t tile)
{
	if (side == 0 || side == 1) return -1;
	const int_t id=block->blockID;
	const bool bottom=isNeighbour(cp,access,x,y-1,z,id,side,tile), top=isNeighbour(cp,access,x,y+1,z,id,side,tile);
	const int_t index = bottom ? (top ? 1 : 2) : (top ? 0 : 3);
	return cp.textureId * 256 + cp.tiles[static_cast<size_t>(index)];
}

int_t ConnectedTextures::getTop(const ConnectedProperties &cp, IBlockAccess *access, Block *block, int_t x, int_t y, int_t z, int_t side, int_t tile)
{
	if (side == 0 || side == 1) return -1;
	return isNeighbour(cp,access,x,y+1,z,block->blockID,side,tile) ? cp.textureId * 256 + cp.tiles[0] : -1;
}

int_t ConnectedTextures::getCtm(const ConnectedProperties &cp, IBlockAccess *access, Block *block, int_t x, int_t y, int_t z, int_t side, int_t tile)
{
	bool b[4] = {false,false,false,false}; const int_t id=block->blockID;
	switch(side) {
	case 0: case 1: b[0]=isNeighbour(cp,access,x-1,y,z,id,side,tile); b[1]=isNeighbour(cp,access,x+1,y,z,id,side,tile); b[2]=isNeighbour(cp,access,x,y,z+1,id,side,tile); b[3]=isNeighbour(cp,access,x,y,z-1,id,side,tile); break;
	case 2: b[0]=isNeighbour(cp,access,x+1,y,z,id,side,tile); b[1]=isNeighbour(cp,access,x-1,y,z,id,side,tile); b[2]=isNeighbour(cp,access,x,y-1,z,id,side,tile); b[3]=isNeighbour(cp,access,x,y+1,z,id,side,tile); break;
	case 3: b[0]=isNeighbour(cp,access,x-1,y,z,id,side,tile); b[1]=isNeighbour(cp,access,x+1,y,z,id,side,tile); b[2]=isNeighbour(cp,access,x,y-1,z,id,side,tile); b[3]=isNeighbour(cp,access,x,y+1,z,id,side,tile); break;
	case 4: b[0]=isNeighbour(cp,access,x,y,z-1,id,side,tile); b[1]=isNeighbour(cp,access,x,y,z+1,id,side,tile); b[2]=isNeighbour(cp,access,x,y-1,z,id,side,tile); b[3]=isNeighbour(cp,access,x,y+1,z,id,side,tile); break;
	case 5: b[0]=isNeighbour(cp,access,x,y,z+1,id,side,tile); b[1]=isNeighbour(cp,access,x,y,z-1,id,side,tile); b[2]=isNeighbour(cp,access,x,y-1,z,id,side,tile); b[3]=isNeighbour(cp,access,x,y+1,z,id,side,tile); break;
	}
	int_t index=0;
	if (b[0]&&!b[1]&&!b[2]&&!b[3]) index=3; else if(!b[0]&&b[1]&&!b[2]&&!b[3]) index=1;
	else if(!b[0]&&!b[1]&&b[2]&&!b[3]) index=12; else if(!b[0]&&!b[1]&&!b[2]&&b[3]) index=36;
	else if(b[0]&&b[1]&&!b[2]&&!b[3]) index=2; else if(!b[0]&&!b[1]&&b[2]&&b[3]) index=24;
	else if(b[0]&&!b[1]&&b[2]&&!b[3]) index=15; else if(b[0]&&!b[1]&&!b[2]&&b[3]) index=39;
	else if(!b[0]&&b[1]&&b[2]&&!b[3]) index=13; else if(!b[0]&&b[1]&&!b[2]&&b[3]) index=37;
	else if(!b[0]&&b[1]&&b[2]&&b[3]) index=25; else if(b[0]&&!b[1]&&b[2]&&b[3]) index=27;
	else if(b[0]&&b[1]&&!b[2]&&b[3]) index=38; else if(b[0]&&b[1]&&b[2]&&!b[3]) index=14;
	else if(b[0]&&b[1]&&b[2]&&b[3]) index=26;
	if (!Config::isConnectedTexturesFancy()) return cp.textureId*256+cp.tiles[static_cast<size_t>(index)];

	bool e[4] = {false,false,false,false};
	switch(side) {
	case 0: case 1: e[0]=!isNeighbour(cp,access,x+1,y,z+1,id,side,tile); e[1]=!isNeighbour(cp,access,x-1,y,z+1,id,side,tile); e[2]=!isNeighbour(cp,access,x+1,y,z-1,id,side,tile); e[3]=!isNeighbour(cp,access,x-1,y,z-1,id,side,tile); break;
	case 2: e[0]=!isNeighbour(cp,access,x-1,y-1,z,id,side,tile); e[1]=!isNeighbour(cp,access,x+1,y-1,z,id,side,tile); e[2]=!isNeighbour(cp,access,x-1,y+1,z,id,side,tile); e[3]=!isNeighbour(cp,access,x+1,y+1,z,id,side,tile); break;
	case 3: e[0]=!isNeighbour(cp,access,x+1,y-1,z,id,side,tile); e[1]=!isNeighbour(cp,access,x-1,y-1,z,id,side,tile); e[2]=!isNeighbour(cp,access,x+1,y+1,z,id,side,tile); e[3]=!isNeighbour(cp,access,x-1,y+1,z,id,side,tile); break;
	case 4: e[0]=!isNeighbour(cp,access,x,y-1,z+1,id,side,tile); e[1]=!isNeighbour(cp,access,x,y-1,z-1,id,side,tile); e[2]=!isNeighbour(cp,access,x,y+1,z+1,id,side,tile); e[3]=!isNeighbour(cp,access,x,y+1,z-1,id,side,tile); break;
	case 5: e[0]=!isNeighbour(cp,access,x,y-1,z-1,id,side,tile); e[1]=!isNeighbour(cp,access,x,y-1,z+1,id,side,tile); e[2]=!isNeighbour(cp,access,x,y+1,z-1,id,side,tile); e[3]=!isNeighbour(cp,access,x,y+1,z+1,id,side,tile); break;
	}
	if(index==13&&e[0])index=4; if(index==15&&e[1])index=5; if(index==37&&e[2])index=16; if(index==39&&e[3])index=17;
	if(index==14&&e[0]&&e[1])index=7; if(index==25&&e[0]&&e[2])index=6; if(index==27&&e[3]&&e[1])index=19; if(index==38&&e[3]&&e[2])index=18;
	if(index==14&&!e[0]&&e[1])index=31; if(index==25&&e[0]&&!e[2])index=30; if(index==27&&!e[3]&&e[1])index=41; if(index==38&&e[3]&&!e[2])index=40;
	if(index==14&&e[0]&&!e[1])index=29; if(index==25&&!e[0]&&e[2])index=28; if(index==27&&e[3]&&!e[1])index=43; if(index==38&&!e[3]&&e[2])index=42;
	if(index==26&&e[0]&&e[1]&&e[2]&&e[3])index=46;
	if(index==26&&!e[0]&&e[1]&&e[2]&&e[3])index=9; if(index==26&&e[0]&&!e[1]&&e[2]&&e[3])index=21;
	if(index==26&&e[0]&&e[1]&&!e[2]&&e[3])index=8; if(index==26&&e[0]&&e[1]&&e[2]&&!e[3])index=20;
	if(index==26&&e[0]&&e[1]&&!e[2]&&!e[3])index=11; if(index==26&&!e[0]&&!e[1]&&e[2]&&e[3])index=22;
	if(index==26&&!e[0]&&e[1]&&!e[2]&&e[3])index=23; if(index==26&&e[0]&&!e[1]&&e[2]&&!e[3])index=10;
	if(index==26&&e[0]&&!e[1]&&!e[2]&&e[3])index=34; if(index==26&&!e[0]&&e[1]&&e[2]&&!e[3])index=35;
	if(index==26&&e[0]&&!e[1]&&!e[2]&&!e[3])index=32; if(index==26&&!e[0]&&e[1]&&!e[2]&&!e[3])index=33;
	if(index==26&&!e[0]&&!e[1]&&e[2]&&!e[3])index=44; if(index==26&&!e[0]&&!e[1]&&!e[2]&&e[3])index=45;
	return cp.textureId*256+cp.tiles[static_cast<size_t>(index)];
}

bool ConnectedTextures::isConnectedGlassPanes()
{
	return Config::isConnectedTextures() && matchingCtmPng;
}
