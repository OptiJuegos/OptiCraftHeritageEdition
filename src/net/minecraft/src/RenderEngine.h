#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "java/BufferedImage.h"
#include "java/Type.h"

class CustomAnimation;
class GameSettings;
class ImageBuffer;
class TextureFX;
class TexturePackList;
class ThreadDownloadImageData;

// net.minecraft.src.RenderEngine
class RenderEngine
{
public:
	RenderEngine(TexturePackList *texturepacklist, GameSettings *gamesettings);
	~RenderEngine();

	std::vector<int_t> readTextureImageData(const std::string &s);
	int_t getTexture(const std::string &s);
	int_t allocateAndSetupTexture(BufferedImage *bufferedimage);
	void setupTexture(BufferedImage *bufferedimage, int_t i, bool tileAtlas = false, bool terrainAlphaFix = false);
	void updateTextureSubImage(const std::vector<int_t> &ai, int_t i, int_t j, int_t k);
	void deleteTexture(int_t i);
	void releaseTexture(const std::string &s);
	void clearDecodedTextureCache();
	int_t getTextureForDownloadableImage(const std::string &s, const std::string &s1);
	ThreadDownloadImageData *obtainImageData(const std::string &s, ImageBuffer *imagebuffer);
	void releaseImageData(const std::string &s);
	void registerTextureFX(TextureFX *texturefx, bool takeOwnership = true);
	void updateDynamicTextures();
	void refreshTextures();
	void bindTexture(int_t i);

	// Selected texture-pack resource access used by OptiFine-compatible visual features.
	// The caller owns the returned stream, matching TexturePackBase::getResourceAsStream.
	std::istream *getResourceAsStream(const std::string &path) const;
	bool hasResource(const std::string &path) const;
	bool isDefaultTexturePack() const;
	std::vector<std::string> listResources(const std::string &prefix, const std::string &suffix) const;
	bool getTextureDimensions(int_t texture, int_t *width, int_t *height) const;

	// Allocation-free census for platform diagnostics. These containers
	// retain CPU-side image data and are otherwise invisible in allocator stats.
	void getTextureMemoryStats(std::size_t *textureIds,
	                        std::size_t *pixelCacheBytes,
	                        std::size_t *retainedImageBytes,
	                        std::size_t *textureFx,
	                        std::size_t *downloadImages) const;

	static bool useMipmaps;

private:
	std::vector<int_t> getImagePixelsARGB(BufferedImage *bufferedimage);
	void copyImagePixelsARGB(BufferedImage *bufferedimage, std::vector<int_t> &ai);
	std::unique_ptr<BufferedImage> readTextureImage(std::istream *inputstream);
	std::unique_ptr<BufferedImage> unwrapImageByColumns(BufferedImage *bufferedimage);
	std::unique_ptr<BufferedImage> createMissingTexture();

	// Decode `s` and upload it into an existing texture name. Returns false when
	// the image could not be produced, in which case the name has been filled
	// with missingTextureImage (the black/white checkerboard) instead.
	bool loadTextureInto(const std::string &s, int_t texture, bool applyResidencyPolicy = true);
	bool isDynamicTextureResource(const std::string &s) const;

	// getTexture() calls to wait before retrying a texture whose load failed.
	// Small enough that a texture recovers within a couple of seconds once
	// memory frees up, large enough that a permanently missing file does not put
	// an SD read and a PNG decode inside every frame that binds it.
	static constexpr int_t TEXTURE_RETRY_INTERVAL = 120;
	int_t averageColor(int_t i, int_t j);
	int_t weightedAverageColor(int_t i, int_t j);
	void updateTextureSubImageRegion(int_t texture, int_t x, int_t y, int_t width, int_t height, const byte_t *pixels);
	void updateCustomAnimations();
	void loadCustomAnimations();

	std::map<std::string, int_t> textureMap;
	// Keys in textureMap whose image failed to load, mapped to the countdown
	// until the next retry attempt. See TEXTURE_RETRY_INTERVAL.
	std::map<std::string, int_t> failedTextures;
	std::map<std::string, std::vector<int_t>> field_28151_c;
	std::map<int_t, std::shared_ptr<BufferedImage>> textureNameToImageMap;
	std::map<int_t, std::pair<int_t, int_t>> textureDimensions;
	std::vector<TextureFX *> textureList;
	std::vector<std::unique_ptr<CustomAnimation>> textureAnimations;
	std::vector<TextureFX *> ownedTextureFx;
	std::map<std::string, ThreadDownloadImageData *> urlToImageDataMap;
	GameSettings *options;
	bool clampTexture;
	bool blurTexture;
	TexturePackList *texturePack;
	std::unique_ptr<BufferedImage> missingTextureImage;
};
