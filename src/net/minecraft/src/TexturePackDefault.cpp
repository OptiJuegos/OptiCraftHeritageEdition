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
#include "legacy/LegacyCraftingIconsPng.h"
#include <sstream>
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
	if (s.find(':') != std::string::npos || s.rfind("./", 0) == 0)
	{
		auto st = GameResources::open(s);
		if (st)
			return st.release();
	}
#ifdef PS2_PLATFORM
	try
	{
		std::istream *stream = Resource::getResource(s);
		if (stream != nullptr)
			return stream;
	}
	catch (...)
	{
	}
	// Console crafting menu tab icons, compiled in like the Xbox build.
	if (s.find("crafting_icons.png") != std::string::npos)
		return new std::istringstream(std::string(reinterpret_cast<const char*>(s_legacyCraftingIconsPngData), LEGACY_CRAFTING_ICONS_PNG_SIZE));

	return nullptr;
#else
	return GameResources::open(s).release();
#endif
}
