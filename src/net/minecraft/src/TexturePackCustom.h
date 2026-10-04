#pragma once

#include "TexturePackBase.h"
#include <memory>
#include <string>

// minizip's opaque zip handle ("typedef void* unzFile" in unzip.h). minizip is
// compiled into every target of this project -- desktop through the root
// CMakeLists (OPTICRAFT_MINIZIP_SOURCES), 3DS/PS2/Wii through their cmake/
// platform files, all for the mods system's OchPackReader -- so the texture
// pack path shares it instead of vendoring a second zip library. The zip code
// below used to sit behind MCBETA_HAVE_LIBZIP, a macro nothing in the tree
// ever defined: the class compiled to a no-op and every selected pack
// silently fell back to the default assets on every platform.
#include "unzip.h"

class BufferedImage;
class Minecraft;

// net.minecraft.src.TexturePackCustom
class TexturePackCustom : public TexturePackBase
{
public:
	TexturePackCustom(const std::string &file);
	~TexturePackCustom() override;

	void getTexturePackFolder(Minecraft *minecraft) override;
	void getResourceAsStream(Minecraft *minecraft) override;
	void bindThumbnailTexture(Minecraft *minecraft) override;
	void loadTexturePack() override;
	void closeTexturePackFile() override;
	std::istream* getResourceAsStream(const std::string &s) override;
	std::vector<std::string> listResources(const std::string &prefix, const std::string &suffix) override;

private:
	std::string truncateString(const std::string &s);
	// Read a whole entry into memory. Returns an empty vector when the entry
	// is missing or cannot be inflated; callers fall back to the default
	// assets, the same posture the previous zip code had.
	std::vector<char> readEntry(unzFile zip, const char *entryName);

	unzFile texturePackZipFile;
	int_t texturePackName; // Renderer texture handle
	std::unique_ptr<BufferedImage> texturePackThumbnail;
	std::string texturePackFile;
};
