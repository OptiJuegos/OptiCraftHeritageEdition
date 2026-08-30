#include "TexturePackDefault.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <memory>
#include "java/BufferedImage.h"
#include "GameResources.h"
#include "Minecraft.h"
#include "RenderEngine.h"
#include "platform/RenderAPI.h"

#ifdef PS2_PLATFORM
#include "java/Resource.h"
#endif

TexturePackDefault::TexturePackDefault() :
	texturePackName(-1),
	texturePackThumbnail(nullptr)
{
	texturePackFileName = "Default";
	firstDescriptionLine = "The default look of Minecraft";
	try
	{
		std::unique_ptr<std::istream> input(getResourceAsStream("/pack.png"));
		if (input != nullptr && *input)
			texturePackThumbnail = new BufferedImage(BufferedImage::ImageIO_read(*input));
	}
	catch (...)
	{
		texturePackThumbnail = nullptr;
	}
}

TexturePackDefault::~TexturePackDefault()
{
	delete texturePackThumbnail;
	texturePackThumbnail = nullptr;
}

void TexturePackDefault::getResourceAsStream(Minecraft *minecraft)
{
	if (texturePackThumbnail != nullptr && texturePackName >= 0)
	{
		minecraft->renderEngine->deleteTexture(texturePackName);
		texturePackName = -1;
	}
}

void TexturePackDefault::bindThumbnailTexture(Minecraft *minecraft)
{
	if (texturePackThumbnail != nullptr && texturePackName < 0)
		texturePackName = minecraft->renderEngine->allocateAndSetupTexture(texturePackThumbnail);
	if (texturePackThumbnail != nullptr)
		minecraft->renderEngine->bindTexture(texturePackName);
	else
		renderBindTexture(minecraft->renderEngine->getTexture("/gui/unknown_pack.png"));
}

std::istream* TexturePackDefault::getResourceAsStream(const std::string &s)
{
#ifdef PS2_PLATFORM
	try
	{
		return Resource::getResource(s);
	}
	catch (...)
	{
		return nullptr;
	}
#else
	std::string resolved = GameResources::resolve(s);
	if (resolved.empty())
		return nullptr;
	std::ifstream *input = new std::ifstream(resolved, std::ios::binary);
	if (*input)
		return input;
	delete input;
	return nullptr;
#endif
}
