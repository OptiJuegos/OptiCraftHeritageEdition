#pragma once

#include <string>
#include <vector>
#include "java/String.h"

class Minecraft;

// net.minecraft.src.TexturePackBase
class TexturePackBase
{
public:
	TexturePackBase();
	virtual ~TexturePackBase() = default;

	virtual void loadTexturePack(); // openTexturePackFile
	virtual void closeTexturePackFile();
	virtual void getTexturePackFolder(Minecraft *minecraft); // loadTexturePack
	virtual void getResourceAsStream(Minecraft *minecraft); // unloadTexturePack
	virtual void bindThumbnailTexture(Minecraft *minecraft);
	virtual std::istream* getResourceAsStream(const std::string &s);
	virtual std::vector<std::string> listResources(const std::string &prefix, const std::string &suffix);

	jstring texturePackFileName;
	jstring firstDescriptionLine;
	jstring secondDescriptionLine;
	jstring texturePackFolder; // texturePackId / cacheKey
};
