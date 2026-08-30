#pragma once

#include "TexturePackBase.h"
#include <memory>
#include <string>

#ifdef MCBETA_HAVE_LIBZIP
#include <zip.h>
#else
struct zip;
typedef struct zip zip_t;
#endif

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

	zip_t *texturePackZipFile;
	int_t texturePackName; // Renderer texture handle
	std::unique_ptr<BufferedImage> texturePackThumbnail;
	std::string texturePackFile;
};
