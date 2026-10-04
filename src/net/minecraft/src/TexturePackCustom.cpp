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

// Zip-backed texture packs, over the vendored zlib's minizip (unzip.h).
// Every entry is inflated into memory and handed back as a stringstream,
// which is the shape the rest of the resource chain already consumes and
// the same whole-entry posture the previous libzip code had. The pack file
// itself is read directly off disk (minizip's default POSIX iofuncs), not
// slurped first: packs are scanned on desktop and 3DS only (TexturePackList
// compiles the scan out on PS2/Wii), and both of those have working seekable
// stdio on the pack directory.

namespace
{
// unzGetCurrentFileInfo wants a caller-provided name buffer. Pack entry names
// are short ("/terrain.png", "/mob/char.png", "/custom/lava_still.png" ...),
// but a hostile or zip-bombed pack can carry arbitrarily long ones; minizip
// simply truncates to whatever fits, so give it a comfortably large buffer.
constexpr unsigned long kEntryNameBufferBytes = 1024;
} // namespace

TexturePackCustom::TexturePackCustom(const std::string &file) :
	texturePackZipFile(nullptr),
	texturePackName(-1),
	texturePackThumbnail(nullptr),
	texturePackFile(file)
{
	texturePackFileName = file;
}

TexturePackCustom::~TexturePackCustom()
{
	closeTexturePackFile();
}

std::string TexturePackCustom::truncateString(const std::string &s)
{
	const jstring value(s);
	return String::utf16Length(value) > 34 ? String::substringUtf16(value, 0, 34) : value;
}

std::vector<char> TexturePackCustom::readEntry(unzFile zip, const char *entryName)
{
	std::vector<char> data;
	if (zip == nullptr || entryName == nullptr)
		return data;

	// Case-insensitive lookup: a superset of vanilla Java's exact-match
	// ZipFile behavior -- anything vanilla ever resolved still resolves, and
	// hand-made packs with one wrong case stop silently falling back to the
	// default assets.
	if (unzLocateFile(zip, entryName, 1) != UNZ_OK)
		return data;

	unz_file_info info;
	char nameBuffer[kEntryNameBufferBytes];
	if (unzGetCurrentFileInfo(zip, &info, nameBuffer, sizeof(nameBuffer), nullptr, 0, nullptr, 0) != UNZ_OK)
		return data;

	if (info.uncompressed_size == 0)
		return data;

	if (unzOpenCurrentFile(zip) != UNZ_OK)
		return data;

	// Inflate in chunks straight into the destination, with the entry size
	// reserved up front: one buffer, no doubling reallocations and no
	// second copy like the old vector + istringstream pair made.
	data.resize(info.uncompressed_size);
	unsigned long offset = 0;
	while (offset < data.size())
	{
		const int count = unzReadCurrentFile(zip, data.data() + offset,
		                                     static_cast<unsigned int>(data.size() - offset));
		if (count <= 0)
			break;
		offset += static_cast<unsigned long>(count);
	}
	unzCloseCurrentFile(zip);

	if (offset != data.size())
		data.resize(offset);
	return data;
}

void TexturePackCustom::getTexturePackFolder(Minecraft *minecraft)
{
	(void)minecraft;
	unzFile zipfile = unzOpen(texturePackFile.c_str());
	if (zipfile == nullptr)
	{
		MC_LOG_ERROR("resources", "Failed to open texture pack: %s\n", texturePackFile.c_str());
		return;
	}

	// Read pack.txt: first two lines become the list's description lines.
	std::vector<char> packTxt = readEntry(zipfile, "pack.txt");
	if (!packTxt.empty())
	{
		std::istringstream iss(std::string(packTxt.data(), packTxt.size()));
		std::string line;
		if (std::getline(iss, line))
			firstDescriptionLine = truncateString(line);
		if (std::getline(iss, line))
			secondDescriptionLine = truncateString(line);
	}

	// Read pack.png through the same image decoder used by the rest of Minecraft.
	texturePackThumbnail.reset();
	std::vector<char> packPng = readEntry(zipfile, "pack.png");
	if (!packPng.empty())
	{
		try
		{
			std::istringstream imageStream(std::string(packPng.data(), packPng.size()),
			                               std::ios::in | std::ios::binary);
			texturePackThumbnail.reset(new BufferedImage(BufferedImage::ImageIO_read(imageStream)));
		}
		catch (...)
		{
			texturePackThumbnail.reset();
		}
	}

	unzClose(zipfile);
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

	if (texturePackThumbnail != nullptr && texturePackName < 0)
		texturePackName = minecraft->renderEngine->allocateAndSetupTexture(texturePackThumbnail.get());
	if (texturePackThumbnail != nullptr && texturePackName >= 0)
	{
		minecraft->renderEngine->bindTexture(texturePackName);
		return;
	}

	renderBindTexture(minecraft->renderEngine->getTexture("/gui/unknown_pack.png"));
}

void TexturePackCustom::loadTexturePack()
{
	closeTexturePackFile();
	texturePackZipFile = unzOpen(texturePackFile.c_str());
	if (texturePackZipFile == nullptr)
	{
		MC_LOG_ERROR("resources", "Failed to open texture pack: %s\n", texturePackFile.c_str());
		return;
	}
}

void TexturePackCustom::closeTexturePackFile()
{
	if (texturePackZipFile != nullptr)
	{
		unzClose(texturePackZipFile);
		texturePackZipFile = nullptr;
	}
}

std::istream* TexturePackCustom::getResourceAsStream(const std::string &s)
{
	if (texturePackZipFile != nullptr)
	{
		std::string entryName = s;
		if (!entryName.empty() && entryName[0] == '/')
			entryName = entryName.substr(1); // remove leading /
		std::vector<char> buf = readEntry(texturePackZipFile, entryName.c_str());
		if (!buf.empty())
			return new std::istringstream(std::string(buf.data(), buf.size()));
	}
	return TexturePackBase::getResourceAsStream(s);
}

std::vector<std::string> TexturePackCustom::listResources(const std::string &prefix, const std::string &suffix)
{
	std::vector<std::string> result;
	if (texturePackZipFile == nullptr)
		return result;

	std::string normalizedPrefix = prefix;
	while (!normalizedPrefix.empty() && normalizedPrefix.front() == '/')
		normalizedPrefix.erase(normalizedPrefix.begin());

	unz_global_info globalInfo;
	if (unzGetGlobalInfo(texturePackZipFile, &globalInfo) != UNZ_OK)
		return result;

	char nameBuffer[kEntryNameBufferBytes];
	if (unzGoToFirstFile(texturePackZipFile) != UNZ_OK)
		return result;

	for (uLong i = 0; i < globalInfo.number_entry; ++i)
	{
		if (unzGetCurrentFileInfo(texturePackZipFile, nullptr, nameBuffer, sizeof(nameBuffer),
		                          nullptr, 0, nullptr, 0) != UNZ_OK)
		{
			if (unzGoToNextFile(texturePackZipFile) != UNZ_OK)
				break;
			continue;
		}
		const std::string name(nameBuffer);
		if (name.rfind(normalizedPrefix, 0) == 0)
		{
			if (suffix.empty() || (name.size() >= suffix.size() &&
			                       name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0))
			{
				result.push_back('/' + name);
			}
		}
		if (unzGoToNextFile(texturePackZipFile) != UNZ_OK)
			break;
	}

	std::sort(result.begin(), result.end());
	return result;
}
