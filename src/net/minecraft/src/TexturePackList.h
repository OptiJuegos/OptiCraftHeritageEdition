#pragma once

#include <vector>
#include <map>
#include <string>
#include "java/String.h"

class Minecraft;
class TexturePackBase;
class TexturePackDefault;

// net.minecraft.src.TexturePackList
class TexturePackList
{
public:
	TexturePackList(Minecraft *minecraft, const std::string &file);
	~TexturePackList();

	bool setTexturePack(TexturePackBase *texturepackbase);
	void updateAvailableTexturePacks();
	const std::vector<TexturePackBase*> &getAvailableTexturePacks() const;
	TexturePackBase *getSelectedTexturePack() const;

private:
	std::vector<TexturePackBase*> availableTexturePacks;
	TexturePackDefault *defaultTexturePack;
	std::map<jstring, TexturePackBase*> soundPool; // texturePackCache
	Minecraft *mc;
	std::string texturePackDir;
	jstring currentTexturePack;
	TexturePackBase *selectedTexturePack;
};
