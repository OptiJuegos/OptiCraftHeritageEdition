#pragma once

#include <map>
#include <string>
#include <vector>

class Entity;
class RenderEngine;

class RandomMobs
{
public:
	static std::string getTexture(Entity *entity, const std::string &texture, RenderEngine *engine);
	static void resetTextures();

private:
	static std::map<std::string, std::vector<std::string>> textureVariantsMap;
	static const std::vector<std::string> &getTextureVariants(const std::string &texture, RenderEngine *engine);
};
