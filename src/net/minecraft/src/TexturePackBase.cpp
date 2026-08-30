#include "TexturePackBase.h"
#include "GameResources.h"

#include <fstream>
#include <algorithm>
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
#include <filesystem>
#endif

TexturePackBase::TexturePackBase()
{
}

void TexturePackBase::loadTexturePack()
{
}

void TexturePackBase::closeTexturePackFile()
{
}

void TexturePackBase::getTexturePackFolder(Minecraft *minecraft)
{
}

void TexturePackBase::getResourceAsStream(Minecraft *minecraft)
{
}

void TexturePackBase::bindThumbnailTexture(Minecraft *minecraft)
{
}

std::istream* TexturePackBase::getResourceAsStream(const std::string &s)
{
	// Java getResourceAsStream fallback: resolve against the vanilla asset root.
	auto stream = GameResources::open(s);
	return stream.release();
}

std::vector<std::string> TexturePackBase::listResources(const std::string &prefix, const std::string &suffix)
{
	std::vector<std::string> result;
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
	namespace fs = std::filesystem;
	std::string normalizedPrefix = prefix;
	while (!normalizedPrefix.empty() && normalizedPrefix.front() == '/')
		normalizedPrefix.erase(normalizedPrefix.begin());
	const fs::path root = fs::path(GameResources::getAssetsDir()) / normalizedPrefix;
	if (!fs::exists(root) || !fs::is_directory(root))
		return result;
	for (const auto &entry : fs::recursive_directory_iterator(root))
	{
		if (!entry.is_regular_file())
			continue;
		const std::string name = entry.path().filename().string();
		if (!suffix.empty() && (name.size() < suffix.size() || name.compare(name.size() - suffix.size(), suffix.size(), suffix) != 0))
			continue;
		std::string relative = fs::relative(entry.path(), GameResources::getAssetsDir()).generic_string();
		result.push_back('/' + relative);
	}
	std::sort(result.begin(), result.end());
#else
	(void)prefix;
	(void)suffix;
#endif
	return result;
}
