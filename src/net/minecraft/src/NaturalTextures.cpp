#include "NaturalTextures.h"

#include <sstream>

#include "Config.h"
#include "OptiFineResource.h"
#include "RenderEngine.h"

RenderEngine *NaturalTextures::renderEngine = nullptr;
std::map<int_t, std::vector<NaturalProperties>> NaturalTextures::propertiesByTexture;
std::map<int_t, std::vector<bool>> NaturalTextures::propertyMaskByTexture;

void NaturalTextures::setProperty(int_t textureId, int_t tile, const NaturalProperties &property)
{
	if (textureId < 0 || tile < 0 || tile > 255 || !property.isValid())
		return;
	auto &props = propertiesByTexture[textureId];
	auto &mask = propertyMaskByTexture[textureId];
	if (props.size() != 256) props.resize(256);
	if (mask.size() != 256) mask.assign(256, false);
	props[static_cast<size_t>(tile)] = property;
	mask[static_cast<size_t>(tile)] = true;
}

void NaturalTextures::makeDefaultProperties()
{
	if (renderEngine == nullptr || !renderEngine->isDefaultTexturePack())
		return;
	const int_t terrainTexture = renderEngine->getTexture("/terrain.png");
	struct Entry { int_t tile; const char *type; };
	static const Entry entries[] = {
		{0,"4F"},{1,"2F"},{2,"4F"},{3,"F"},{38,"F"},{6,"F"},{17,"2F"},{18,"4F"},
		{19,"4"},{20,"2F"},{21,"4F"},{32,"2F"},{33,"2F"},{34,"2F"},{50,"2F"},{51,"2F"},
		{160,"2F"},{37,"4F"},{52,"2F"},{53,"2F"},{196,"2"},{197,"2"},{66,"4F"},{68,"F"},
		{70,"2F"},{72,"4F"},{77,"F"},{78,"4F"},{86,"2F"},{87,"2F"},{103,"4F"},{104,"4F"},
		{105,"4"},{116,"2F"},{117,"F"},{132,"2F"},{133,"2F"},{153,"2F"},{175,"4"},{176,"4"},
		{208,"4F"},{211,"4F"},{212,"4F"}
	};
	for (const Entry &entry : entries)
		setProperty(terrainTexture, entry.tile, NaturalProperties(entry.type));
}

void NaturalTextures::update(RenderEngine *engine)
{
	renderEngine = engine;
	propertiesByTexture.clear();
	propertyMaskByTexture.clear();
	if (!Config::isNaturalTextures() || engine == nullptr)
		return;

	std::string text;
	if (!OptiFineResource::readText(engine, "/natural.properties", text))
	{
		makeDefaultProperties();
		return;
	}

	std::istringstream input(text);
	std::string line;
	while (std::getline(input, line))
	{
		line = OptiFineResource::trim(line);
		if (line.empty() || line[0] == '#')
			continue;
		const size_t eq = line.find('=');
		if (eq == std::string::npos)
			continue;
		const std::string key = OptiFineResource::trim(line.substr(0, eq));
		const std::string type = OptiFineResource::trim(line.substr(eq + 1));
		const size_t colon = key.rfind(':');
		if (colon == std::string::npos)
			continue;
		std::string texture = key.substr(0, colon);
		const int_t tile = OptiFineResource::parseInt(key.substr(colon + 1), -1);
		if (texture.empty() || tile < 0 || tile > 255)
			continue;
		if (texture[0] != '/') texture.insert(texture.begin(), '/');
		NaturalProperties property(type);
		if (property.isValid())
			setProperty(engine->getTexture(texture), tile, property);
	}
}

const NaturalProperties *NaturalTextures::getNaturalProperties(int_t textureId, int_t tileNum)
{
	if (renderEngine == nullptr || tileNum < 0 || tileNum > 255)
		return nullptr;
	if (textureId == 0)
		textureId = renderEngine->getTexture("/terrain.png");
	auto propsIt = propertiesByTexture.find(textureId);
	auto maskIt = propertyMaskByTexture.find(textureId);
	if (propsIt == propertiesByTexture.end() || maskIt == propertyMaskByTexture.end())
		return nullptr;
	if (!maskIt->second[static_cast<size_t>(tileNum)])
		return nullptr;
	return &propsIt->second[static_cast<size_t>(tileNum)];
}
