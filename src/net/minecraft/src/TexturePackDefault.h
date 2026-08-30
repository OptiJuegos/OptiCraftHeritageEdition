#pragma once

#include "TexturePackBase.h"

class Minecraft;
class BufferedImage;

// net.minecraft.src.TexturePackDefault
class TexturePackDefault : public TexturePackBase
{
public:
	TexturePackDefault();
	~TexturePackDefault() override;

	void getResourceAsStream(Minecraft *minecraft) override;
	void bindThumbnailTexture(Minecraft *minecraft) override;
	std::istream* getResourceAsStream(const std::string &s) override;

private:
	int_t texturePackName;
	BufferedImage *texturePackThumbnail;
};
