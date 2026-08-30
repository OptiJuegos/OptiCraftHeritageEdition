#pragma once

#include <map>
#include <vector>

#include "java/Type.h"
#include "NaturalProperties.h"

class RenderEngine;

class NaturalTextures
{
public:
	static void update(RenderEngine *engine);
	static const NaturalProperties *getNaturalProperties(int_t textureId, int_t tileNum);

private:
	static RenderEngine *renderEngine;
	static std::map<int_t, std::vector<NaturalProperties>> propertiesByTexture;
	static std::map<int_t, std::vector<bool>> propertyMaskByTexture;
	static void makeDefaultProperties();
	static void setProperty(int_t textureId, int_t tile, const NaturalProperties &property);
};
