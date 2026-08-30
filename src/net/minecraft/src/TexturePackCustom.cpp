#include "platform/Log.h"
#include "TexturePackCustom.h"
#include "java/String.h"
#include "java/BufferedImage.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "Minecraft.h"
#include "RenderEngine.h"
#include "platform/RenderAPI.h"

TexturePackCustom::TexturePackCustom(const std::string &file) :
	texturePackZipFile(nullptr),
	texturePackName(-1),
	texturePackThumbnail(nullptr),
	texturePackFile(file)
{
	texturePackFileName = file;
}

TexturePackCustom::~TexturePackCustom() = default;

std::string TexturePackCustom::truncateString(const std::string &s)
{
	const jstring value(s);
	return String::utf16Length(value) > 34 ? String::substringUtf16(value, 0, 34) : value;
}

void TexturePackCustom::getTexturePackFolder(Minecraft *minecraft)
{
	(void)minecraft;
#ifdef MCBETA_HAVE_LIBZIP
	int err = 0;
	zip_t *zipfile = zip_open(texturePackFile.c_str(), 0, &err);
	if (!zipfile)
	{
		MC_LOG_ERROR("resources", "Failed to open texture pack: %s\n", texturePackFile.c_str());
		return;
	}

	// Read pack.txt
	zip_stat_t stat;
	if (zip_stat(zipfile, "pack.txt", 0, &stat) == 0)
	{
		zip_file_t *zf = zip_fopen(zipfile, "pack.txt", 0);
		if (zf)
		{
			std::vector<char> buf(stat.size + 1);
			zip_fread(zf, buf.data(), stat.size);
			buf[stat.size] = '\0';
			zip_fclose(zf);

			std::istringstream iss(buf.data());
			std::string line;
			if (std::getline(iss, line))
				firstDescriptionLine = truncateString(line);
			if (std::getline(iss, line))
				secondDescriptionLine = truncateString(line);
		}
	}

	// Read pack.png through the same image decoder used by the rest of Minecraft.
	texturePackThumbnail.reset();
	if (zip_stat(zipfile, "pack.png", 0, &stat) == 0)
	{
		zip_file_t *zf = zip_fopen(zipfile, "pack.png", 0);
		if (zf)
		{
			std::vector<char> buf(stat.size);
			const zip_int64_t bytesRead = zip_fread(zf, buf.data(), stat.size);
			zip_fclose(zf);
			if (bytesRead == static_cast<zip_int64_t>(stat.size))
			{
				try
				{
					std::istringstream imageStream(std::string(buf.data(), buf.size()), std::ios::in | std::ios::binary);
					texturePackThumbnail.reset(new BufferedImage(BufferedImage::ImageIO_read(imageStream)));
				}
				catch (...)
				{
					texturePackThumbnail.reset();
				}
			}
		}
	}

	zip_close(zipfile);
#endif
}

void TexturePackCustom::getResourceAsStream(Minecraft *minecraft)
{
	if (minecraft != nullptr && minecraft->renderEngine != nullptr && texturePackName >= 0)
	{
		minecraft->renderEngine->deleteTexture(texturePackName);
		texturePackName = -1;
	}
	closeTexturePackFile();
}

void TexturePackCustom::bindThumbnailTexture(Minecraft *minecraft)
{
	if (minecraft == nullptr || minecraft->renderEngine == nullptr)
		return;

#ifdef MCBETA_HAVE_LIBZIP
	if (texturePackThumbnail != nullptr && texturePackName < 0)
		texturePackName = minecraft->renderEngine->allocateAndSetupTexture(texturePackThumbnail.get());
	if (texturePackThumbnail != nullptr && texturePackName >= 0)
	{
		minecraft->renderEngine->bindTexture(texturePackName);
		return;
	}
#endif

	renderBindTexture(minecraft->renderEngine->getTexture("/gui/unknown_pack.png"));
}

void TexturePackCustom::loadTexturePack()
{
#ifdef MCBETA_HAVE_LIBZIP
	int err = 0;
	texturePackZipFile = zip_open(texturePackFile.c_str(), 0, &err);
#endif
}

void TexturePackCustom::closeTexturePackFile()
{
#ifdef MCBETA_HAVE_LIBZIP
	if (texturePackZipFile)
	{
		zip_close(texturePackZipFile);
		texturePackZipFile = nullptr;
	}
#endif
}

std::istream* TexturePackCustom::getResourceAsStream(const std::string &s)
{
#ifdef MCBETA_HAVE_LIBZIP
	if (texturePackZipFile)
	{
		std::string entryName = s.substr(1); // remove leading /
		zip_stat_t stat;
		if (zip_stat(texturePackZipFile, entryName.c_str(), 0, &stat) == 0)
		{
			zip_file_t *zf = zip_fopen(texturePackZipFile, entryName.c_str(), 0);
			if (zf)
			{
				std::vector<char> buf(stat.size);
				zip_fread(zf, buf.data(), stat.size);
				zip_fclose(zf);
				return new std::istringstream(std::string(buf.data(), stat.size));
			}
		}
	}
#endif
	return TexturePackBase::getResourceAsStream(s);
}

std::vector<std::string> TexturePackCustom::listResources(const std::string &prefix, const std::string &suffix)
{
#ifdef MCBETA_HAVE_LIBZIP
	std::vector<std::string> result;
	if (texturePackZipFile == nullptr)
		return result;
	std::string normalizedPrefix = prefix;
	while (!normalizedPrefix.empty() && normalizedPrefix.front() == '/')
		normalizedPrefix.erase(normalizedPrefix.begin());
	const zip_int64_t count = zip_get_num_entries(texturePackZipFile, 0);
	for (zip_uint64_t i = 0; i < static_cast<zip_uint64_t>(count); ++i)
	{
		const char *rawName = zip_get_name(texturePackZipFile, i, 0);
		if (rawName == nullptr)
			continue;
		const std::string name(rawName);
		if (name.rfind(normalizedPrefix, 0) != 0)
			continue;
		if (!suffix.empty() && (name.size() < suffix.size() || name.compare(name.size() - suffix.size(), suffix.size(), suffix) != 0))
			continue;
		result.push_back('/' + name);
	}
	std::sort(result.begin(), result.end());
	return result;
#else
	return TexturePackBase::listResources(prefix, suffix);
#endif
}
