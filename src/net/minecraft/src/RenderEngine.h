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

enum class SpecialTextureFxResult
{
	NotHandled,
	HandledNoUpload,
	HandledUpload
};

// net.minecraft.src.RenderEngine
class RenderEngine
{
public:
	RenderEngine(TexturePackList *texturepacklist, GameSettings *gamesettings);
	~RenderEngine();

	std::vector<int_t> readTextureImageData(const std::string &s);
	int_t getTexture(const std::string &s);
	int_t allocateAndSetupTexture(BufferedImage *bufferedimage, bool highPrecision = false);
	void setupTexture(BufferedImage *bufferedimage, int_t i, bool tileAtlas = false, bool terrainAlphaFix = false, bool highPrecision = false);
	void updateTextureSubImage(const std::vector<int_t> &ai, int_t i, int_t j, int_t k);
	void deleteTexture(int_t i);
	void releaseTexture(const std::string &s);
	void clearDecodedTextureCache();
	int_t getTextureForDownloadableImage(const std::string &s, const std::string &s1);
	ThreadDownloadImageData *obtainImageData(const std::string &s, ImageBuffer *imagebuffer);
	void releaseImageData(const std::string &s);
	void registerTextureFX(TextureFX *texturefx, bool takeOwnership = true);
	void updateDynamicTextures();
	// Advance PS2 background texture jobs independently of getTexture() calls.
	// This lets cached texture IDs recover after a transient USB/read/decode failure.
	void updateBackgroundTextureLoads();
	void refreshTextures();
	void bindTexture(int_t i);
	void setBackgroundTextureLoadingEnabled(bool enabled);

	// Selected texture-pack resource access used by OptiFine-compatible visual features.
	// The caller owns the returned stream, matching TexturePackBase::getResourceAsStream.
	std::istream *getResourceAsStream(const std::string &path) const;
	bool hasResource(const std::string &path) const;
	bool isDefaultTexturePack() const;
	std::vector<std::string> listResources(const std::string &prefix, const std::string &suffix) const;
	bool getTextureDimensions(int_t texture, int_t *width, int_t *height) const;

	// Deterministic texture state queries to prevent visual corruption / checkerboard fallback.
	// Used by UI widgets and rendering components to check if an asset is already loaded and valid,
	// currently in flight (async), or has failed, allowing clean procedural fallbacks.
	bool isTextureLoaded(const std::string &s) const;
	bool isTextureFailed(const std::string &s) const;
	bool isTexturePending(const std::string &s) const;

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
	bool loadTextureStreamInto(const std::string &s, int_t texture, std::istream *inputstream, bool applyResidencyPolicy = true);
	bool shouldLoadTextureAsync(const std::string &s) const;
	bool processAsyncTextureLoad(const std::string &s, int_t texture, bool advanceRetryCountdown);
	bool isDynamicTextureResource(const std::string &s) const;

	// getTexture() calls to wait before retrying a texture whose load failed.
	// Small enough that a texture recovers within a couple of seconds once
	// memory frees up, large enough that a permanently missing file does not put
	// an SD read and a PNG decode inside every frame that binds it.
	static constexpr int_t TEXTURE_RETRY_INTERVAL = 120;
	// A rejected async request normally means the four PS2 I/O slots are busy.
	// Retry that queue admission quickly; actual I/O/decode failures use the longer interval above.
	static constexpr int_t TEXTURE_QUEUE_RETRY_INTERVAL = 4;
	int_t averageColor(int_t i, int_t j);
	int_t weightedAverageColor(int_t i, int_t j);
	void updateTextureSubImageRegion(int_t texture, int_t x, int_t y, int_t width, int_t height, const byte_t *pixels);
	void updateCustomAnimations();
	void loadCustomAnimations();
	void syncCustomAnimationTileWidths();
	SpecialTextureFxResult updateSpecialTextureFx(TextureFX *texturefx, int_t texture, int_t tileWidth);
	SpecialTextureFxResult updateDefaultTerrainTextureFx(TextureFX *texturefx, int_t texture, int_t tileWidth);
	bool updateStaticProceduralTextureFx(TextureFX *texturefx);
	void uploadTextureFxTile(TextureFX *texturefx, int_t texture, int_t tileWidth);
	void rebuildDefaultTerrainFxTiles();
	const std::vector<byte_t> *getDefaultTerrainFxTile(int_t iconIndex) const;

	std::map<std::string, int_t> textureMap;
	// Keys in textureMap whose image failed to load, mapped to the countdown
	// until the next retry attempt. See TEXTURE_RETRY_INTERVAL.
	std::map<std::string, int_t> failedTextures;
	std::map<std::string, std::vector<int_t>> field_28151_c;
	std::map<int_t, std::shared_ptr<BufferedImage>> textureNameToImageMap;
	std::map<int_t, bool> textureHighPrecision;
	std::map<int_t, std::pair<int_t, int_t>> textureDimensions;
	std::vector<TextureFX *> textureList;
	std::vector<std::unique_ptr<CustomAnimation>> textureAnimations;
	std::map<int_t, std::unique_ptr<CustomAnimation>> specialTextureAnimations;
	std::map<int_t, std::unique_ptr<CustomAnimation>> terrainTextureFxAnimations;
	std::map<int_t, std::unique_ptr<CustomAnimation>> itemTextureFxAnimations;
	std::map<int_t, std::vector<byte_t>> defaultTerrainFxTiles;
	std::map<TextureFX *, bool> textureFxHasFrame;
	std::map<TextureFX *, bool> textureFxFrameAnaglyph;
	std::vector<TextureFX *> ownedTextureFx;
	std::map<std::string, ThreadDownloadImageData *> urlToImageDataMap;
	GameSettings *options;
	bool dynamicTexturesUpdated;
	int_t dynamicTextureTickCounter;
	int_t defaultTerrainFxTileWidth;
	int_t customAnimationTerrainTileWidth;
	int_t customAnimationItemTileWidth;
	bool clampTexture;
	bool blurTexture;
	bool backgroundTextureLoadingEnabled;
	std::map<std::string, bool> asyncTextureLoads;
	TexturePackList *texturePack;
	std::unique_ptr<BufferedImage> missingTextureImage;
};
