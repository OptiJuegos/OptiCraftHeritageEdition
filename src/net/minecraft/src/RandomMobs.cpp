#include "RandomMobs.h"

#include <cstdlib>

#include "Config.h"
#include "Entity.h"
#include "java/Arithmetic.h"
#include "OptiFineResource.h"
#include "RenderEngine.h"

std::map<std::string, std::vector<std::string>> RandomMobs::textureVariantsMap;

void RandomMobs::resetTextures()
{
	textureVariantsMap.clear();
}

const std::vector<std::string> &RandomMobs::getTextureVariants(const std::string &texture, RenderEngine *engine)
{
	auto existing = textureVariantsMap.find(texture);
	if (existing != textureVariantsMap.end())
		return existing->second;

	std::vector<std::string> variants;
	variants.push_back(texture);
	const size_t dot = texture.rfind('.');
	if (engine != nullptr && dot != std::string::npos)
	{
		const std::string prefix = texture.substr(0, dot);
		const std::string suffix = texture.substr(dot);
		for (int index = 2; index < 1000; ++index)
		{
			const std::string candidate = prefix + std::to_string(index) + suffix;
			if (!OptiFineResource::exists(engine, candidate))
				break;
			variants.push_back(candidate);
		}
	}
	return textureVariantsMap.emplace(texture, std::move(variants)).first->second;
}

std::string RandomMobs::getTexture(Entity *entity, const std::string &texture, RenderEngine *engine)
{
	if (!Config::isRandomMobs() || entity == nullptr || entity->isPlayer() || texture.empty())
		return texture;
	const std::vector<std::string> &variants = getTextureVariants(texture, engine);
	if (variants.size() <= 1)
		return texture;
	// entityId is stable for the lifetime of the entity in this port. Hash it with
	// the base path so neighbouring entities do not line up across mob species.
	int_t hash = Config::intHash(entity->entityId);
	for (unsigned char c : texture)
		hash = Config::intHash(JavaArithmetic::intAdd(hash, static_cast<int_t>(c)));
	const uint_t index = static_cast<uint_t>(hash) % static_cast<uint_t>(variants.size());
	return variants[index];
}
