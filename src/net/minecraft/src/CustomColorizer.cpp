#include "CustomColorizer.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "BiomeGenBase.h"
#include "Block.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "BlockLilyPad.h"
#include "BlockStem.h"
#include "BlockTallGrass.h"
#include "BlockVine.h"
#include "Config.h"
#include "EntityFX.h"
#include "IBlockAccess.h"
#include "Material.h"
#include "OptiFineResource.h"
#include "RenderEngine.h"

std::vector<int_t> CustomColorizer::grassColors;
std::vector<int_t> CustomColorizer::waterColors;
std::vector<int_t> CustomColorizer::foliageColors;
std::vector<int_t> CustomColorizer::foliagePineColors;
std::vector<int_t> CustomColorizer::foliageBirchColors;
std::vector<int_t> CustomColorizer::swampGrassColors;
std::vector<int_t> CustomColorizer::swampFoliageColors;
std::vector<int_t> CustomColorizer::redstoneColors;
std::vector<int_t> CustomColorizer::stemColors;
std::vector<int_t> CustomColorizer::myceliumParticleColors;
std::vector<std::vector<int_t>> CustomColorizer::paletteColors;
std::array<std::array<int_t, 16>, 256> CustomColorizer::blockPalettes;
bool CustomColorizer::hasBlockPalettes = false;
int_t CustomColorizer::particleWaterColor = -1;
int_t CustomColorizer::particlePortalColor = -1;
int_t CustomColorizer::lilyPadColor = -1;

std::vector<int_t> CustomColorizer::loadColors(RenderEngine *engine, const std::string &path, int_t expectedLength)
{
	if (engine == nullptr || !engine->hasResource(path))
		return {};
	std::vector<int_t> colors = engine->readTextureImageData(path);
	if (expectedLength > 0 && colors.size() != static_cast<size_t>(expectedLength))
		return {};
	for (int_t &color : colors) color &= 0xffffff;
	return colors;
}

void CustomColorizer::update(RenderEngine *engine)
{
	grassColors.clear(); waterColors.clear(); foliageColors.clear(); foliagePineColors.clear(); foliageBirchColors.clear();
	swampGrassColors.clear(); swampFoliageColors.clear(); redstoneColors.clear(); stemColors.clear(); myceliumParticleColors.clear();
	paletteColors.clear(); hasBlockPalettes = false; particleWaterColor = particlePortalColor = lilyPadColor = -1;
	for (auto &entry : blockPalettes) entry.fill(-1);
	if (engine == nullptr) return;

	// These three maps are honored even with Custom Colors off, matching C6's
	// biome colormap replacement behavior.
	grassColors = loadColors(engine, "/misc/grasscolor.png", 65536);
	foliageColors = loadColors(engine, "/misc/foliagecolor.png", 65536);
	waterColors = loadColors(engine, "/misc/watercolorX.png", 65536);
	if (!Config::isCustomColors()) return;

	foliagePineColors = loadColors(engine, "/misc/pinecolor.png", 65536);
	foliageBirchColors = loadColors(engine, "/misc/birchcolor.png", 65536);
	swampGrassColors = loadColors(engine, "/misc/swampgrasscolor.png", 65536);
	swampFoliageColors = loadColors(engine, "/misc/swampfoliagecolor.png", 65536);
	redstoneColors = loadColors(engine, "/misc/redstonecolor.png", 16);
	stemColors = loadColors(engine, "/misc/stemcolor.png", 8);
	myceliumParticleColors = loadColors(engine, "/misc/myceliumparticlecolor.png", -1);
	readColorProperties(engine);
}

int_t CustomColorizer::getCustomColor(const std::vector<int_t> &colors, IBlockAccess *access, int_t x, int_t z)
{
	if (colors.size() != 65536 || access == nullptr) return -1;
	BiomeGenBase *biome = access->getBiomeGenForCoords(x, z);
	if (biome == nullptr) return -1;
	const float temperature = std::max(0.0f, std::min(1.0f, biome->getFloatTemperature()));
	const float rainfall = std::max(0.0f, std::min(1.0f, biome->getFloatRainfall())) * temperature;
	const int_t cx = static_cast<int_t>((1.0f - temperature) * 255.0f);
	const int_t cy = static_cast<int_t>((1.0f - rainfall) * 255.0f);
	return colors[static_cast<size_t>((cy << 8) | cx)] & 0xffffff;
}

int_t CustomColorizer::getSmoothColor(const std::vector<int_t> &colors, IBlockAccess *access, int_t x, int_t z)
{
	int_t r=0,g=0,b=0,count=0;
	for (int_t dx=-1; dx<=1; ++dx) for (int_t dz=-1; dz<=1; ++dz)
	{
		const int_t color=getCustomColor(colors,access,x+dx,z+dz); if(color<0) continue;
		r+=(color>>16)&255; g+=(color>>8)&255; b+=color&255; ++count;
	}
	return count > 0 ? ((r/count)<<16)|((g/count)<<8)|(b/count) : -1;
}

int_t CustomColorizer::getSmoothBlockColor(Block *block, IBlockAccess *access, int_t x, int_t y, int_t z,
	const std::vector<int_t> *colors, const std::vector<int_t> *swampColors)
{
	int_t r=0,g=0,b=0;
	for(int_t dx=-1;dx<=1;++dx) for(int_t dz=-1;dz<=1;++dz)
	{
		const int_t sx=x+dx, sz=z+dz;
		const std::vector<int_t> *map=colors;
		if (Config::isSwampColors() && swampColors != nullptr && !swampColors->empty() && access->getBiomeGenForCoords(sx,sz)==BiomeGenBase::swampland)
			map=swampColors;
		int_t color = map != nullptr && !map->empty() ? getCustomColor(*map,access,sx,sz) : block->colorMultiplier(access,sx,y,sz);
		if(color<0) color=block->colorMultiplier(access,sx,y,sz);
		r+=(color>>16)&255; g+=(color>>8)&255; b+=color&255;
	}
	return (r/9<<16)|(g/9<<8)|(b/9);
}

int_t CustomColorizer::getColorMultiplier(Block *block, IBlockAccess *access, int_t x, int_t y, int_t z)
{
	if (block == nullptr || access == nullptr) return 0xffffff;
	if (hasBlockPalettes && block->blockID >= 0 && block->blockID < 256)
	{
		const int_t md = access->getBlockMetadata(x,y,z) & 15;
		const int_t palette = blockPalettes[static_cast<size_t>(block->blockID)][static_cast<size_t>(md)];
		if (palette >= 0 && palette < static_cast<int_t>(paletteColors.size()) && !paletteColors[static_cast<size_t>(palette)].empty())
		{
			const auto &colors=paletteColors[static_cast<size_t>(palette)];
			return Config::isSmoothBiomes() ? getSmoothColor(colors,access,x,z) : getCustomColor(colors,access,x,z);
		}
	}

	const std::vector<int_t> *colors=nullptr, *swamp=nullptr;
	if (block == Block::grass || block == Block::tallGrass)
	{
		colors=&grassColors; swamp=&swampGrassColors;
	}
	else if (block == Block::leaves || block == Block::vine)
	{
		const int_t md=access->getBlockMetadata(x,y,z)&3;
		if (block == Block::leaves && md==1 && !foliagePineColors.empty()) colors=&foliagePineColors;
		else if (block == Block::leaves && md==2 && !foliageBirchColors.empty()) colors=&foliageBirchColors;
		else { colors=&foliageColors; swamp=&swampFoliageColors; }
	}
	if (colors != nullptr && !colors->empty())
	{
		if (Config::isSmoothBiomes()) return getSmoothBlockColor(block,access,x,y,z,colors,swamp);
		if (Config::isSwampColors() && swamp != nullptr && !swamp->empty() && access->getBiomeGenForCoords(x,z)==BiomeGenBase::swampland)
			colors=swamp;
		const int_t custom=getCustomColor(*colors,access,x,z); if(custom>=0) return custom;
	}
	return block->colorMultiplier(access,x,y,z);
}

int_t CustomColorizer::getFluidColor(Block *block, IBlockAccess *access, int_t x, int_t y, int_t z)
{
	if (block == nullptr || access == nullptr) return 0xffffff;
	if (block->blockMaterial != Material::water) return block->colorMultiplier(access,x,y,z);
	if (!waterColors.empty())
	{
		const int_t color=Config::isSmoothBiomes()?getSmoothColor(waterColors,access,x,z):getCustomColor(waterColors,access,x,z);
		if(color>=0) return color;
	}
	if (!Config::isSwampColors()) return 0xffffff;
	return block->colorMultiplier(access,x,y,z);
}

void CustomColorizer::setParticleColor(EntityFX *fx, int_t color)
{
	if (fx == nullptr || color < 0) return;
	fx->setParticleColor(((color>>16)&255)/255.0f, ((color>>8)&255)/255.0f, (color&255)/255.0f);
}

void CustomColorizer::updatePortalFX(EntityFX *fx) { setParticleColor(fx, particlePortalColor); }

void CustomColorizer::updateReddustFX(EntityFX *fx, IBlockAccess *access, double x, double y, double z)
{
	if (access == nullptr) return;
	setParticleColor(fx, getRedstoneColor(access->getBlockMetadata((int_t)x,(int_t)y,(int_t)z)));
}

void CustomColorizer::updateWaterFX(EntityFX *fx, IBlockAccess *access)
{
	if (fx == nullptr || access == nullptr) return;
	int_t color=getFluidColor(Block::waterStill,access,(int_t)fx->posX,(int_t)fx->posY,(int_t)fx->posZ);
	float r=((color>>16)&255)/255.0f, g=((color>>8)&255)/255.0f, b=(color&255)/255.0f;
	if(particleWaterColor>=0){r*=((particleWaterColor>>16)&255)/255.0f;g*=((particleWaterColor>>8)&255)/255.0f;b*=(particleWaterColor&255)/255.0f;}
	fx->setParticleColor(r,g,b);
}

void CustomColorizer::updateMyceliumFX(EntityFX *fx)
{
	if (fx == nullptr || myceliumParticleColors.empty()) return;
	const uint_t index=static_cast<uint_t>(Config::intHash(fx->entityId))%static_cast<uint_t>(myceliumParticleColors.size());
	setParticleColor(fx,myceliumParticleColors[index]);
}

int_t CustomColorizer::getRedstoneColor(int_t level)
{
	return level>=0 && level<static_cast<int_t>(redstoneColors.size()) ? redstoneColors[static_cast<size_t>(level)]&0xffffff : -1;
}

int_t CustomColorizer::getStemColorMultiplier(BlockStem *stem, IBlockAccess *access, int_t x, int_t y, int_t z)
{
	if (stem == nullptr || access == nullptr) return 0xffffff;
	if (stemColors.empty()) return stem->colorMultiplier(access,x,y,z);
	int_t level=std::max<int_t>(0,std::min<int_t>(static_cast<int_t>(stemColors.size())-1,access->getBlockMetadata(x,y,z)));
	return stemColors[static_cast<size_t>(level)]&0xffffff;
}

int_t CustomColorizer::getLilypadColor()
{
	return lilyPadColor >= 0 ? lilyPadColor : (Block::waterlily != nullptr ? Block::waterlily->getBlockColor() : 0x208030);
}

void CustomColorizer::readCustomPalettes(RenderEngine *engine, const std::map<std::string, std::string> &properties)
{
	const std::string prefix="palette.block.";
	for(const auto &entry:properties)
	{
		if(entry.first.rfind(prefix,0)!=0) continue;
		std::string path=entry.first.substr(prefix.size()); if(path.empty()) continue; if(path[0]!='/')path.insert(path.begin(),'/');
		std::vector<int_t> colors=loadColors(engine,path,65536); if(colors.empty()) continue;
		const int_t paletteIndex=static_cast<int_t>(paletteColors.size()); paletteColors.push_back(std::move(colors));
		std::string normalized=entry.second; for(char &c:normalized) if(c==','||c==';') c=' ';
		std::istringstream input(normalized); std::string token;
		while(input>>token)
		{
			int_t metadata=-1; const size_t colon=token.find(':');
			if(colon!=std::string::npos){metadata=OptiFineResource::parseInt(token.substr(colon+1),-1);token=token.substr(0,colon);}
			const int_t blockId=OptiFineResource::parseInt(token,-1); if(blockId<0||blockId>255||metadata>15)continue;
			if(blockId==(Block::grass?Block::grass->blockID:-1)||blockId==(Block::tallGrass?Block::tallGrass->blockID:-1)||blockId==(Block::leaves?Block::leaves->blockID:-1)||blockId==(Block::vine?Block::vine->blockID:-1))continue;
			if(metadata>=0) blockPalettes[static_cast<size_t>(blockId)][static_cast<size_t>(metadata)]=paletteIndex;
			else blockPalettes[static_cast<size_t>(blockId)].fill(paletteIndex);
			hasBlockPalettes=true;
		}
	}
}

void CustomColorizer::readColorProperties(RenderEngine *engine)
{
	auto properties=OptiFineResource::readProperties(engine,"/color.properties");
	auto color=[&](const char *name)->int_t{auto it=properties.find(name);return it==properties.end()?-1:OptiFineResource::parseHexColor(it->second,-1);};
	lilyPadColor=color("lilypad"); particleWaterColor=color("particle.water"); if(particleWaterColor<0)particleWaterColor=color("drop.water"); particlePortalColor=color("particle.portal");
	readCustomPalettes(engine,properties);
}
