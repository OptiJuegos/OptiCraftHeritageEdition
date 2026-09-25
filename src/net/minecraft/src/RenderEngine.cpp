#include "platform/WorkProfiler.h"
#include "platform/ExtendedProfiler.h"
#include "RenderEngine.h"
#include "platform/Log.h"
#include "platform/PlatformConfig.h"
#include "platform/TextureResidencyPolicy.h"
#include "legacy/LegacyPanoramaUpload.h"
#include "java/Arithmetic.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "Block.h"
#include "BlockFire.h"
#include "BlockPortal.h"
#include "GameSettings.h"
#include "Config.h"
#include "CustomAnimation.h"
#if PLATFORM_OPTIFINE_RANDOM_MOBS
#include "RandomMobs.h"
#endif
#include "NaturalTextures.h"
#include "OptiFineResource.h"
#include "ConnectedTextures.h"
#include "CustomColorizer.h"
#include "GLAllocation.h"
#include "ImageBuffer.h"
#include "TextureFX.h"
#include "TexturePackBase.h"
#include "TexturePackList.h"
#include "ThreadDownloadImageData.h"
#include "platform/RenderAPI.h"
#if PLATFORM_PS2
#include "platform/storage/AssetPak.h"
#include "ps2/storage/assets/Ps2AsyncAssetLoader.h"
#include "ps2/storage/assets/Ps2Assets.h"
#endif



bool RenderEngine::useMipmaps = false;

static std::string normalizedTexturePath(const std::string &name)
{
	std::string path = name;
	if (path.rfind("%clamp%", 0) == 0)
		path.erase(0, 7);
	else if (path.rfind("%blur%", 0) == 0)
		path.erase(0, 6);
	else if (path.rfind("##", 0) == 0)
		path.erase(0, 2);
	if (path.find(':') != std::string::npos || path.rfind("./", 0) == 0)
		return path;
	if (!path.empty() && path[0] != '/')
		path.insert(path.begin(), '/');
	return path;
}

static bool isTileAtlasResource(const std::string &name)
{
	const std::string path = normalizedTexturePath(name);
	return path == "/terrain.png" || path == "/gui/items.png";
}

static bool shouldCacheDecodedTexturePixels(const std::string &name)
{
#if PLATFORM_BOUNDED_DECODED_TEXTURE_CACHE
	// PS2 only benefits from one concrete back-to-back reuse: compass and watch
	// both decode items.png during TextureFX construction. Colormaps, fonts and
	// custom color tables are one-shot reads, so caching them only doubles their
	// peak RAM while they are copied into their real owner.
	return normalizedTexturePath(name) == "/gui/items.png";
#else
	(void)name;
	return true;
#endif
}

// OptiFine's isTerrainTexture(): only terrain.png and its matching ctm.png
// atlas get the transparent-texel color fix below. gui/items.png does not.
static bool isTerrainAlphaFixResource(const std::string &name)
{
	const std::string path = normalizedTexturePath(name);
	return path == "/terrain.png" || path == "/ctm.png";
}

// OptiFine's getAverageOpaqueColor(), computed once per tile of a 16x16 atlas
// grid. Returns packed 0x00RRGGBB per tile, or 0 if the tile has no opaque
// pixel at all. Used below to recolor fully transparent texels before they
// feed the mip chain, so a cutout tile (leaves, glass, vines...) does not
// bleed its (usually black) transparent RGB into the visible edge once
// mipmapping blends it with opaque neighbours.
static std::vector<int_t> computeTileAverageOpaqueColors(const unsigned char *raw, int_t width, int_t height)
{
	std::vector<int_t> tileColors(256, 0);
	const int_t tileWidth = width / 16;
	const int_t tileHeight = height / 16;
	if (tileWidth <= 0 || tileHeight <= 0)
		return tileColors;

	for (int_t ty = 0; ty < 16; ++ty)
	{
		for (int_t tx = 0; tx < 16; ++tx)
		{
			long long redSum = 0, greenSum = 0, blueSum = 0, count = 0;
			for (int_t py = 0; py < tileHeight; ++py)
			{
				const std::size_t rowBase = (static_cast<std::size_t>(ty * tileHeight + py) * static_cast<std::size_t>(width) +
					static_cast<std::size_t>(tx * tileWidth)) * 4u;
				for (int_t px = 0; px < tileWidth; ++px)
				{
					const std::size_t idx = rowBase + static_cast<std::size_t>(px) * 4u;
					if (raw[idx + 3] == 0)
						continue;
					redSum += raw[idx + 0];
					greenSum += raw[idx + 1];
					blueSum += raw[idx + 2];
					++count;
				}
			}
			if (count > 0)
			{
				const int_t r = static_cast<int_t>(redSum / count);
				const int_t g = static_cast<int_t>(greenSum / count);
				const int_t b = static_cast<int_t>(blueSum / count);
				tileColors[static_cast<size_t>(ty * 16 + tx)] = (r << 16) | (g << 8) | b;
			}
		}
	}
	return tileColors;
}

static int_t clampMipmapLevelForSize(int_t requestedLevel, int_t width, int_t height)
{
	int_t level = 0;
	while (level < requestedLevel && width > 1 && height > 1)
	{
		width /= 2;
		height /= 2;
		++level;
	}
	return level;
}

// OptiFine's getMaxMipmapLevel(size): floor(log2(size)).
static int_t maxMipmapLevelForDimension(int_t size)
{
	int_t level = 0;
	while (size > 0)
	{
		size /= 2;
		++level;
	}
	return level - 1;
}

// Config value 4 ("Max") is a sentinel, not a literal level count: OptiFine
// replaces it with getMaxMipmapLevel(minDim) - 4 so the deepest sampled level
// never shrinks past a single atlas tile (avoids mipmap bleeding between
// unrelated terrain.png tiles). Levels 0-3 are used literally.
static int_t resolveMipmapLevel(int_t configuredLevel, int_t width, int_t height)
{
	if (configuredLevel >= 4)
	{
		configuredLevel = maxMipmapLevelForDimension(std::min(width, height)) - 4;
		if (configuredLevel < 0)
			configuredLevel = 0;
	}
	return clampMipmapLevelForSize(configuredLevel, width, height);
}

void RenderEngine::getTextureMemoryStats(std::size_t *textureIds,
	                                  std::size_t *pixelCacheBytes,
	                                  std::size_t *retainedImageBytes,
	                                  std::size_t *textureFx,
	                                  std::size_t *downloadImages) const
{
	std::size_t cachedPixels = 0;
	for (const auto &entry : field_28151_c)
		cachedPixels += entry.second.capacity() * sizeof(int_t);

	std::size_t retainedImages = 0;
	for (const auto &entry : textureNameToImageMap)
	{
		const BufferedImage *image = entry.second.get();
		if (image != nullptr)
			retainedImages += (std::size_t)image->getWidth() *
			                  (std::size_t)image->getHeight() * 4u;
	}

	if (textureIds) *textureIds = textureMap.size();
	if (pixelCacheBytes) *pixelCacheBytes = cachedPixels;
	if (retainedImageBytes) *retainedImageBytes = retainedImages;
	if (textureFx) *textureFx = textureList.size();
	if (downloadImages) *downloadImages = urlToImageDataMap.size();
}

RenderEngine::RenderEngine(TexturePackList *texturepacklist, GameSettings *gamesettings)
	: options(gamesettings)
	, dynamicTexturesUpdated(false)
	, dynamicTextureTickCounter(0)
	, defaultTerrainFxTileWidth(0)
	, customAnimationTerrainTileWidth(0)
	, customAnimationItemTileWidth(0)
	, clampTexture(false)
	, blurTexture(false)
	, backgroundTextureLoadingEnabled(false)
	, texturePack(texturepacklist)
	, missingTextureImage(createMissingTexture())
{
	loadCustomAnimations();
}

RenderEngine::~RenderEngine()
{
#if PLATFORM_PS2
	Ps2AsyncAssetLoader::cancelAll();
	Ps2AsyncAssetLoader::shutdown();
#endif
	for (TextureFX *fx : ownedTextureFx)
		delete fx;
	ownedTextureFx.clear();
	textureList.clear();

	for (auto &entry : urlToImageDataMap)
		delete entry.second;
	urlToImageDataMap.clear();
}

std::vector<int_t> RenderEngine::readTextureImageData(const std::string &s)
{
	auto it = field_28151_c.find(s);
	if (it != field_28151_c.end())
	{
#if PLATFORM_BOUNDED_DECODED_TEXTURE_CACHE
		// The bounded PS2 entry is a hand-off cache, not a permanent owner. The
		// second consumer takes the vector and removes the map node immediately.
		std::vector<int_t> pixels = std::move(it->second);
		field_28151_c.erase(it);
		return pixels;
#else
		return it->second;
#endif
	}

	try
	{
		std::unique_ptr<BufferedImage> image;
		TexturePackBase *texturepackbase = texturePack != nullptr ? texturePack->getSelectedTexturePack() : nullptr;
		if (s.rfind("##", 0) == 0)
		{
			image = unwrapImageByColumns(readTextureImage(texturepackbase->getResourceAsStream(s.substr(2))).get());
		}
		else if (s.rfind("%clamp%", 0) == 0)
		{
			clampTexture = true;
			image = readTextureImage(texturepackbase->getResourceAsStream(s.substr(7)));
			clampTexture = false;
		}
		else if (s.rfind("%blur%", 0) == 0)
		{
			blurTexture = true;
			image = readTextureImage(texturepackbase->getResourceAsStream(s.substr(6)));
			blurTexture = false;
		}
		else if (texturepackbase != nullptr)
		{
			image = readTextureImage(texturepackbase->getResourceAsStream(s));
		}

		// Only a successful decode goes in the cache. Caching the checkerboard
		// here is worse than in getTexture(): these pixels feed TextureFX and the
		// terrain compositing, so one transient allocation failure would bake the
		// missing-texture pattern into animated water/lava/fire for the rest of
		// the session with no bind to retry on.
		std::vector<int_t> pixels = getImagePixelsARGB(image ? image.get() : missingTextureImage.get());
		if (image && shouldCacheDecodedTexturePixels(s))
		{
#if PLATFORM_BOUNDED_DECODED_TEXTURE_CACHE
			field_28151_c.clear();
#endif
			field_28151_c[s] = pixels;
		}
		return pixels;
	}
	catch (...)
	{
		return getImagePixelsARGB(missingTextureImage.get());
	}
}

std::vector<int_t> RenderEngine::getImagePixelsARGB(BufferedImage *bufferedimage)
{
	if (bufferedimage == nullptr)
		bufferedimage = missingTextureImage.get();

	int_t width = bufferedimage->getWidth();
	int_t height = bufferedimage->getHeight();
	const std::size_t pixelCount = BufferedImage::checkedPixelCount(width, height);
	std::vector<int_t> pixels(pixelCount);
	const unsigned char *raw = bufferedimage->getRawPixels();
	for (std::size_t i = 0; i < pixelCount; i++)
	{
		int_t r = raw[i * 4u + 0] & 0xff;
		int_t g = raw[i * 4u + 1] & 0xff;
		int_t b = raw[i * 4u + 2] & 0xff;
		int_t a = raw[i * 4u + 3] & 0xff;
		pixels[i] = JavaArithmetic::intFromBits((static_cast<uint_t>(a) << 24) | (static_cast<uint_t>(r) << 16) | (static_cast<uint_t>(g) << 8) | static_cast<uint_t>(b));
	}
	return pixels;
}

void RenderEngine::copyImagePixelsARGB(BufferedImage *bufferedimage, std::vector<int_t> &ai)
{
	ai = getImagePixelsARGB(bufferedimage);
}

bool RenderEngine::isDynamicTextureResource(const std::string &s) const
{
	const std::string path = normalizedTexturePath(s);
	if (path == "/terrain.png" || path == "/gui/items.png")
		return true;

	for (const std::unique_ptr<CustomAnimation> &animation : textureAnimations)
	{
		if (animation && normalizedTexturePath(animation->destTexture) == path)
			return true;
	}
	return false;
}

bool RenderEngine::loadTextureInto(const std::string &s, int_t texture, bool applyResidencyPolicy)
{
	TexturePackBase *texturepackbase = texturePack != nullptr ? texturePack->getSelectedTexturePack() : nullptr;
	std::istream *inputstream = texturepackbase != nullptr
		? texturepackbase->getResourceAsStream(normalizedTexturePath(s))
		: nullptr;
	return loadTextureStreamInto(s, texture, inputstream, applyResidencyPolicy);
}

bool RenderEngine::loadTextureStreamInto(const std::string &s, int_t texture, std::istream *inputstream, bool applyResidencyPolicy)
{
	PlatformLoadWorkScope textureWork(PlatformLoadWork::TextureLoad);
	bool loaded = false;
	try
	{
		std::unique_ptr<BufferedImage> image;
		if (s.rfind("##", 0) == 0)
		{
			image = unwrapImageByColumns(readTextureImage(inputstream).get());
		}
		else if (s.rfind("%clamp%", 0) == 0)
		{
			clampTexture = true;
			image = readTextureImage(inputstream);
		}
		else if (s.rfind("%blur%", 0) == 0)
		{
			blurTexture = true;
			image = readTextureImage(inputstream);
		}
		else
		{
			image = readTextureImage(inputstream);
		}

		const std::string normalizedPath = normalizedTexturePath(s);
		image = legacyPreparePanoramaForUpload(normalizedPath, std::move(image));

		// Minecraft 1.2.5 player/biped models use 64x32 texture format.
		// If a modern 64x64 skin texture is loaded, automatically convert it to 64x32
		// with second-layer overlay compositing and opaque base regions to prevent
		// texture stretching/distortion on the model.
		if (image && image->getWidth() == 64 && image->getHeight() == 64)
		{
			std::string lowerPath = normalizedPath;
			for (char &c : lowerPath)
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			if (lowerPath.find("skin") != std::string::npos ||
			    lowerPath.find("char") != std::string::npos ||
			    lowerPath.find("player") != std::string::npos ||
			    lowerPath.find("/mob/") != std::string::npos)
			{
				std::vector<unsigned char> srcRgba(BufferedImage::checkedRgbaByteCount(64, 64));
				image->getRGB(0, 0, 64, 64, srcRgba.data());
				std::vector<unsigned char> dstRgba(BufferedImage::checkedRgbaByteCount(64, 32), 0);
				std::memcpy(dstRgba.data(), srcRgba.data(), 64 * 32 * 4);

				// Alpha composite 64x64 2nd-layer overlays:
				auto blendRect = [&](int sx, int sy, int rw, int rh, int dx, int dy) {
					for (int y = 0; y < rh; ++y)
					{
						int srcY = sy + y;
						int dstY = dy + y;
						for (int x = 0; x < rw; ++x)
						{
							int srcX = sx + x;
							int dstX = dx + x;
							const unsigned char *sp = &srcRgba[(srcY * 64 + srcX) * 4];
							unsigned char *dp = &dstRgba[(dstY * 64 + dstX) * 4];
							float a = sp[3] / 255.0f;
							if (a > 0.01f)
							{
								dp[0] = static_cast<unsigned char>(sp[0] * a + dp[0] * (1.0f - a));
								dp[1] = static_cast<unsigned char>(sp[1] * a + dp[1] * (1.0f - a));
								dp[2] = static_cast<unsigned char>(sp[2] * a + dp[2] * (1.0f - a));
								dp[3] = 255;
							}
						}
					}
				};

				blendRect(16, 32, 24, 16, 16, 16); // Torso overlay
				blendRect(40, 32, 16, 16, 40, 16); // Right Arm overlay
				blendRect(0, 32, 16, 16, 0, 16);   // Right Leg overlay

				// Make base skin regions opaque (head, torso, limbs)
				for (int y = 0; y < 16; ++y)
					for (int x = 0; x < 32; ++x)
						dstRgba[(x + y * 64) * 4 + 3] = 255;
				for (int y = 16; y < 32; ++y)
					for (int x = 0; x < 64; ++x)
						dstRgba[(x + y * 64) * 4 + 3] = 255;

				auto retro = std::make_unique<BufferedImage>(64, 32);
				retro->setRGB(0, 0, 64, 32, dstRgba.data());
				image = std::move(retro);
			}
		}
#ifdef WII_PLATFORM
		// Bring-up diagnostic. getTexture swallows every failure into
		// missingTextureImage via the catch below, so a texture that silently
		// fails to load is indistinguishable from one that renders wrong -- and
		// that ambiguity has already cost several debugging rounds.
		//
		// The FAILURE is what has diagnostic value and it fires almost never, so
		// it stays on at the default level. The success line fires once per
		// texture, and a texture load is not always on a loading screen -- open a
		// GUI that pulls in a sheet it has not needed yet and the success lines
		// land inside a frame, each one an SD open/write/close. That half is
		// level 2.
		if (MC_LOG_LEVEL >= 2 || !image)
		{
			MC_LOG_DEBUG("render", "texture path='%s' status=%s size=%dx%d id=%d\n", s.c_str(),
			       image ? "ok" : "NOT-LOADED",
			       image ? (int)image->getWidth() : 0,
			       image ? (int)image->getHeight() : 0,
			       (int)texture);
		}
#endif
		loaded = (bool)image;
		// setupTexture() reads clampTexture/blurTexture to pick GL_CLAMP/GL_LINEAR, so it MUST run
		// while the flags are still set; reset them AFTER (as Java does), not inside the branch above.
		BufferedImage *uploadImage = image ? image.get() : missingTextureImage.get();
		if (image && normalizedPath == "/terrain.png")
			Config::setIconWidthTerrain(image->getWidth() / 16);
		else if (image && normalizedPath == "/gui/items.png")
			Config::setIconWidthItems(image->getWidth() / 16);
		setupTexture(uploadImage, texture, isTileAtlasResource(s), isTerrainAlphaFixResource(s));
	}
	catch (...)
	{
#ifdef WII_PLATFORM
		MC_LOG_ERROR("render", "texture path='%s' exception; using missing texture\n",
		       s.c_str());
#endif
		setupTexture(missingTextureImage.get(), texture, isTileAtlasResource(s), isTerrainAlphaFixResource(s));
		loaded = false;
	}

	if (applyResidencyPolicy)
		TextureResidencyPolicy::afterNamedTextureUpload(texture, isDynamicTextureResource(s));

	clampTexture = false;
	blurTexture = false;
	return loaded;
}

bool RenderEngine::shouldLoadTextureAsync(const std::string &s) const
{
#if PLATFORM_PS2
	if (s.find(':') != std::string::npos || s.rfind("./", 0) == 0)
		return false;
	if (!backgroundTextureLoadingEnabled || Ps2Assets::source() != Ps2Assets::Source::UsbMass)
		return false;

	const std::string path = normalizedTexturePath(s);
	if (path == "/terrain.png" || path == "/gui/items.png" || path == "/ctm.png")
		return false;
	if (path.rfind("/font/", 0) == 0)
		return false;

	// [Issue #3 Fix]: All GUI, HUD, title, legacy UI and cursor elements must load synchronously
	// on first use so interactive menus, containers, and HUD overlays never display the 16x16
	// checkerboard error fallback (missingTextureImage) while waiting on the background thread.
	if (path.rfind("/gui/", 0) == 0 ||
	    path.rfind("/title/", 0) == 0 ||
	    path.rfind("/legacy/", 0) == 0 ||
	    path.rfind("/achievement/", 0) == 0 ||
	    path == "/cursor.png")
		return false;

	// [Issue #3 Fix]: Essential in-world blocks, tile entities, particles, and items.
	// Loading these synchronously guarantees world elements and particles do not glitch
	// with checkerboard patterns during chunk rendering or particle effects.
	if (path.rfind("/item/", 0) == 0 ||
	    path.rfind("/misc/", 0) == 0 ||
	    path.rfind("/environment/", 0) == 0 ||
	    path.rfind("/art/", 0) == 0 ||
	    path == "/particles.png")
		return false;

	// [Issue #3 Fix]: Mobs and armor should also load synchronously to prevent checkerboard
	// glitches on living entities (e.g., sheep, zombies) as they enter view.
	if (path.rfind("/mob/", 0) == 0 ||
	    path.rfind("/armor/", 0) == 0)
		return false;

	return true;
#else
	(void)s;
	return false;
#endif
}

bool RenderEngine::processAsyncTextureLoad(const std::string &s, int_t texture, bool advanceRetryCountdown)
{
#if PLATFORM_PS2
	if (asyncTextureLoads.find(s) == asyncTextureLoads.end())
		return false;

	const std::string resourcePath = normalizedTexturePath(s);
	Ps2AsyncAssetLoader::State asyncState = Ps2AsyncAssetLoader::state(resourcePath);
	if (asyncState == Ps2AsyncAssetLoader::State::Ready)
	{
		std::unique_ptr<std::istream> input = Ps2AsyncAssetLoader::takeStream(resourcePath);
		const bool loaded = input != nullptr && loadTextureStreamInto(s, texture, input.release());
		const bool textureValid = renderTextureIsValid(texture);
#if MC_LOG_LEVEL >= 2
		MC_LOG_DEBUG("assets.async",
			"[PS2][async-texture] decode path=%s loaded=%d valid=%d id=%d\n",
			resourcePath.c_str(), loaded ? 1 : 0, textureValid ? 1 : 0, (int)texture);
#endif
		if (loaded && textureValid)
		{
			asyncTextureLoads.erase(s);
			failedTextures.erase(s);
		}
		else
		{
			// takeStream() releases the loader slot. Keep this texture in the async
			// set so the global pump can request the same resource again later.
			failedTextures[s] = TEXTURE_RETRY_INTERVAL;
		}
		return true; // PNG decode/upload is the expensive part; cap it to one/tick.
	}

	if (asyncState == Ps2AsyncAssetLoader::State::Failed)
	{
#if MC_LOG_LEVEL >= 2
		MC_LOG_DEBUG("assets.async",
			"[PS2][async-texture] loader-failed path=%s retryTicks=%d\n",
			resourcePath.c_str(), (int)TEXTURE_RETRY_INTERVAL);
#endif
		// [Issue #3 Fix]: Release loader slot and clear from asyncTextureLoads so we do not
		// enter an infinite loop of attempting to decode an invalid or missing resource, which
		// would lock the renderer into perpetually displaying missingTextureImage.
		Ps2AsyncAssetLoader::release(resourcePath);
		asyncTextureLoads.erase(s);
		failedTextures[s] = TEXTURE_RETRY_INTERVAL;
		return false;
	}

	if (asyncState == Ps2AsyncAssetLoader::State::Missing)
	{
		auto failedIt = failedTextures.find(s);
		bool retryNow = failedIt == failedTextures.end();
		if (!retryNow && advanceRetryCountdown && --failedIt->second <= 0)
			retryNow = true;

		if (retryNow)
		{
			if (Ps2AsyncAssetLoader::request(resourcePath))
				failedTextures.erase(s);
			else
				failedTextures[s] = TEXTURE_QUEUE_RETRY_INTERVAL;
		}
	}
#else
	(void)s;
	(void)texture;
	(void)advanceRetryCountdown;
#endif
	return false;
}

void RenderEngine::updateBackgroundTextureLoads()
{
#if PLATFORM_PS2
	if (!backgroundTextureLoadingEnabled || asyncTextureLoads.empty())
		return;

	// Poll every outstanding resource so retry timers are independent of whether
	// a renderer cached its texture ID. Decode ready textures without stalling.
	int decodedCount = 0;
	for (auto it = asyncTextureLoads.begin(); it != asyncTextureLoads.end(); )
	{
		const std::string path = it->first;
		++it; // processAsyncTextureLoad() may erase `path`.

		auto textureIt = textureMap.find(path);
		if (textureIt == textureMap.end())
		{
			Ps2AsyncAssetLoader::release(normalizedTexturePath(path));
			asyncTextureLoads.erase(path);
			failedTextures.erase(path);
			continue;
		}

		if (processAsyncTextureLoad(path, textureIt->second, true))
		{
			if (++decodedCount >= 2)
				break;
		}
	}
#endif
}

int_t RenderEngine::getTexture(const std::string &s)
{
	auto it = textureMap.find(s);
	if (it != textureMap.end())
	{
#if PLATFORM_PS2
		// Harvest a completed job immediately when this texture is actively used,
		// but do not age retry timers here: getTexture() frequency varies wildly
		// between resources. updateBackgroundTextureLoads() advances them once/tick.
		if (asyncTextureLoads.find(s) != asyncTextureLoads.end())
		{
			processAsyncTextureLoad(s, it->second, false);
			return it->second;
		}
#endif

		// A texture whose synchronous load failed is bound to the checkerboard
		// and retried into the same texture name on a countdown.
		auto failedIt = failedTextures.find(s);
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
		bool retriedLoad = false;
#endif
		if (failedIt != failedTextures.end() && --failedIt->second <= 0)
		{
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
			retriedLoad = true;
			const std::uint32_t textureRetryStart = platformProfileRenderPhaseBegin();
#endif
			const bool retryLoaded = loadTextureInto(s, it->second);
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
			platformProfileTextureLookup(s.c_str(), false, retryLoaded && renderTextureIsValid(it->second),
				platformProfileRenderPhaseBegin() - textureRetryStart);
#endif
			if (retryLoaded)
				failedTextures.erase(failedIt);
			else
				failedIt->second = TEXTURE_RETRY_INTERVAL;
		}
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
		if (!retriedLoad)
			platformProfileTextureLookup(s.c_str(), true, true, 0);
#endif
		return it->second;
	}

	std::vector<int_t> single(1);
	GLAllocation::generateTextureNames(single);
	int_t texture = single[0];

#if PLATFORM_PS2
	if (shouldLoadTextureAsync(s))
	{
		const std::string normalized = normalizedTexturePath(s);
		// [Issue #3 Fix]: If the background loader was already requested or completed earlier,
		// directly consume the stream synchronously. This avoids an unnecessary upload of the
		// 16x16 checkerboard fallback when the asset is already available in memory.
		if (Ps2AsyncAssetLoader::request(normalized))
		{
			if (Ps2AsyncAssetLoader::state(normalized) == Ps2AsyncAssetLoader::State::Ready)
			{
				std::unique_ptr<std::istream> input = Ps2AsyncAssetLoader::takeStream(normalized);
				if (input != nullptr && loadTextureStreamInto(s, texture, input.release()))
				{
					if (renderTextureIsValid(texture))
					{
						textureMap[s] = texture;
						return texture;
					}
				}
			}
		}

		setupTexture(missingTextureImage.get(), texture, isTileAtlasResource(s), isTerrainAlphaFixResource(s));
		if (!renderTextureIsValid(texture))
		{
			int_t name = texture;
			renderDeleteTextures(1, &name);
			return texture;
		}

		textureMap[s] = texture;
		asyncTextureLoads[s] = true;
		TextureResidencyPolicy::afterNamedTextureUpload(texture, isDynamicTextureResource(s));
		// [Issue #3 Fix]: Handle immediate failures gracefully instead of getting stuck
		// in asyncTextureLoads.
		if (Ps2AsyncAssetLoader::state(normalized) == Ps2AsyncAssetLoader::State::Failed)
		{
			Ps2AsyncAssetLoader::release(normalized);
			asyncTextureLoads.erase(s);
			failedTextures[s] = TEXTURE_RETRY_INTERVAL;
		}
		else if (Ps2AsyncAssetLoader::state(normalized) == Ps2AsyncAssetLoader::State::Missing)
		{
			failedTextures[s] = TEXTURE_QUEUE_RETRY_INTERVAL;
		}
		return texture;
	}
#endif

#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
	const std::uint32_t textureLoadStart = platformProfileRenderPhaseBegin();
#endif
	bool loaded = loadTextureInto(s, texture);
	const bool textureValid = renderTextureIsValid(texture);
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
	platformProfileTextureLookup(s.c_str(), false, loaded && textureValid,
		platformProfileRenderPhaseBegin() - textureLoadStart);
#endif

	// Some backends can reject an upload because native texture memory is exhausted.
	// Do not cache a broken texture id; retry the whole load on the next bind.
	if (!textureValid)
	{
		MC_LOG_DEBUG("render", "getTexture('%s'): upload failed, will retry next bind\n", s.c_str());
		int_t name = texture;
		renderDeleteTextures(1, &name);
		return texture;
	}
	textureMap[s] = texture;
	if (!loaded)
		failedTextures[s] = TEXTURE_RETRY_INTERVAL;
	return texture;
}

std::unique_ptr<BufferedImage> RenderEngine::unwrapImageByColumns(BufferedImage *bufferedimage)
{
	if (bufferedimage == nullptr)
		return createMissingTexture();

	const int_t sourceWidth = bufferedimage->getWidth();
	const int_t sourceHeight = bufferedimage->getHeight();
	const int_t columns = sourceWidth / 16;
	const int_t outputHeight = JavaArithmetic::intMul(sourceHeight, columns);
	std::unique_ptr<BufferedImage> image(new BufferedImage(16, outputHeight));
	const unsigned char *src = bufferedimage->getRawPixels();
	std::vector<unsigned char> column(BufferedImage::checkedRgbaByteCount(16, sourceHeight));
	for (int_t col = 0; col < columns; col++)
	{
		for (int_t y = 0; y < sourceHeight; y++)
		{
			for (int_t x = 0; x < 16; x++)
			{
				const std::size_t srcIndex =
					(static_cast<std::size_t>(y) * static_cast<std::size_t>(sourceWidth) +
					 static_cast<std::size_t>(col) * 16u + static_cast<std::size_t>(x)) * 4u;
				const std::size_t dstIndex =
					(static_cast<std::size_t>(y) * 16u + static_cast<std::size_t>(x)) * 4u;
				std::memcpy(&column[dstIndex], &src[srcIndex], 4u);
			}
		}
		image->setRGB(0, JavaArithmetic::intMul(col, sourceHeight), 16, sourceHeight, column.data());
	}
	return image;
}

int_t RenderEngine::allocateAndSetupTexture(BufferedImage *bufferedimage, bool highPrecision)
{
	std::vector<int_t> single(1);
	GLAllocation::generateTextureNames(single);
	int_t texture = single[0];
	setupTexture(bufferedimage != nullptr ? bufferedimage : missingTextureImage.get(), texture, false, false, highPrecision);
	if (bufferedimage != nullptr)
	{
		int_t width = bufferedimage->getWidth();
		int_t height = bufferedimage->getHeight();
		const std::size_t byteCount = BufferedImage::checkedRgbaByteCount(width, height);
		std::unique_ptr<unsigned char[]> copy(new unsigned char[byteCount]);
		std::memcpy(copy.get(), bufferedimage->getRawPixels(), byteCount);
		textureNameToImageMap[texture] = std::shared_ptr<BufferedImage>(new BufferedImage(width, height, std::move(copy)));
	}
	return texture;
}

void RenderEngine::setupTexture(BufferedImage *bufferedimage, int_t texture, bool tileAtlas, bool terrainAlphaFix, bool highPrecision)
{
	if (bufferedimage == nullptr)
		bufferedimage = missingTextureImage.get();

	const int_t mipmapLevel = Config::getMipmapLevel();
	useMipmaps = mipmapLevel > 0;
	renderBindTexture(texture);

	int_t width = bufferedimage->getWidth();
	int_t height = bufferedimage->getHeight();
	textureDimensions[texture] = std::make_pair(width, height);
	if (highPrecision)
		textureHighPrecision[texture] = true;
	else
		textureHighPrecision.erase(texture);
	const std::size_t pixelCount = BufferedImage::checkedPixelCount(width, height);
	std::vector<unsigned char> pixels(BufferedImage::checkedRgbaByteCount(width, height));
	const unsigned char *raw = bufferedimage->getRawPixels();

	// Only computed for terrain.png/ctm.png with mipmapping on -- this is the
	// per-tile background color a fully transparent texel gets recolored to
	// below, so the mip chain never blends its RGB.
	const std::vector<int_t> tileAverageColors = (terrainAlphaFix && useMipmaps)
		? computeTileAverageOpaqueColors(raw, width, height) : std::vector<int_t>();
	const int_t tileWidth = width / 16;
	const int_t tileHeight = height / 16;

	for (std::size_t i = 0; i < pixelCount; i++)
	{
		int_t r = raw[i * 4u + 0] & 0xff;
		int_t g = raw[i * 4u + 1] & 0xff;
		int_t b = raw[i * 4u + 2] & 0xff;
		int_t a = raw[i * 4u + 3] & 0xff;
		if (options != nullptr && options->anaglyph)
		{
			int_t nr = (r * 30 + g * 59 + b * 11) / 100;
			int_t ng = (r * 30 + g * 70) / 100;
			int_t nb = (r * 30 + b * 70) / 100;
			r = nr;
			g = ng;
			b = nb;
		}
		// A fully transparent texel keeps whatever RGB its source PNG happened to
		// store there (often black), which bleeds into the visible edge once GL
		// filtering or mip downsampling blends it with opaque neighbours. Match
		// OptiFine: normalize it to white on the terrain/ctm atlas (or its tile's
		// own average opaque color, once mips are on) and to black elsewhere.
		if (a == 0)
		{
			if (terrainAlphaFix)
			{
				r = 255;
				g = 255;
				b = 255;
				if (!tileAverageColors.empty() && tileWidth > 0 && tileHeight > 0)
				{
					const int_t px = static_cast<int_t>(i % static_cast<std::size_t>(width));
					const int_t py = static_cast<int_t>(i / static_cast<std::size_t>(width));
					const int_t tx = px / tileWidth;
					const int_t ty = py / tileHeight;
					const int_t bgColor = tileAverageColors[static_cast<size_t>(ty * 16 + tx)];
					if (bgColor != 0)
					{
						r = (bgColor >> 16) & 0xff;
						g = (bgColor >> 8) & 0xff;
						b = bgColor & 0xff;
					}
				}
			}
			else
			{
				r = 0;
				g = 0;
				b = 0;
			}
		}
		pixels[i * 4u + 0] = (unsigned char)r;
		pixels[i * 4u + 1] = (unsigned char)g;
		pixels[i * 4u + 2] = (unsigned char)b;
		pixels[i * 4u + 3] = (unsigned char)a;
	}

	// The INTERNAL format -- the third argument -- is the one that decides what
	// the texture costs once it is uploaded; the source data stays RGBA8 either
	// way. Vanilla asks for GL_RGBA, which a desktop driver stores as 32 bits per
	// texel and so does wiigx (GX_TF_RGBA8).
	//
	// On the Wii that is worth changing. GX_TF_RGB5A3 is 16 bits per texel, so it
	// halves both the RAM every texture occupies and the bandwidth the GP spends
	// fetching texels -- and at 640x480 with Minecraft's overdraw, texel fetch is
	// a real part of the frame. RGB5A3 picks its layout per texel: 5 bits per
	// channel where the texel is opaque, 4 bits plus 3 bits of alpha where it is
	// not. Terrain is almost entirely the former, and the cutouts (leaves, glass,
	// grass) only ever use alpha 0 or 255, which both layouts represent exactly.
	//
	// Not CMPR/DXT1, which would be 4 bits per texel: its 4x4 colour blocks sit
	// across a 16x16 tile atlas and the artefacts are obvious at Minecraft's
	// scale. This is the format that costs half the memory and almost no
	// appearance, not the one that costs a quarter and looks it.
	const int_t nativeMaxLevel = useMipmaps ? resolveMipmapLevel(mipmapLevel, width, height) : 0;
	bool nativeTextureReady = renderTextureBeginUpload(texture, width, height, nativeMaxLevel,
	                                                 blurTexture, clampTexture, tileAtlas, highPrecision);
	if (nativeTextureReady)
	{
		// GX recreates its native texture object in renderTextureBeginUpload(), so
		// quality state applied before this point would be reset. Apply the common
		// sampling policy only after the backend has created/rebound its object.
		renderTextureParameters(blurTexture, nativeMaxLevel > 0, clampTexture);
#if PLATFORM_TEXTURE_QUALITY_CONTROLS
		renderApplyTextureQuality(blurTexture, nativeMaxLevel, Config::isMipmapLinear(), Config::getAnisotropicFilterLevel());
#endif
		renderTextureImageRgba(0, width, height, pixels.data());
	}

	if (useMipmaps)
	{
		std::vector<byte_t> previous(pixels.begin(), pixels.end());
		int_t previousWidth = width;
		int_t previousHeight = height;
		// Bound by the resolved level (nativeMaxLevel), not the raw config value:
		// mipmapLevel==4 is OptiFine's "Max" sentinel, not a literal level count.
		// GL_TEXTURE_MAX_LEVEL above is already set to nativeMaxLevel, so looping
		// on the raw sentinel here would either upload levels nobody asked for
		// (vanilla-size atlases, where the two happen to coincide) or, for HD
		// texture packs where nativeMaxLevel > 4, leave the chain incomplete --
		// GL then treats the texture as mipmap-incomplete once sampling reaches
		// the undefined levels, which reads as textures going black at distance.
		for (int_t level = 1; level <= nativeMaxLevel && previousWidth > 1 && previousHeight > 1; ++level)
		{
			const int_t mipWidth = previousWidth / 2;
			const int_t mipHeight = previousHeight / 2;
			std::vector<byte_t> current((size_t)mipWidth * (size_t)mipHeight * 4u);
			for (int_t y = 0; y < mipHeight; ++y)
			{
				for (int_t x = 0; x < mipWidth; ++x)
				{
					auto packed = [&](int_t sx, int_t sy) -> int_t
					{
						const size_t idx = ((size_t)sy * previousWidth + sx) * 4u;
						return JavaArithmetic::intFromBits((static_cast<uint_t>(previous[idx + 3]) << 24) | (static_cast<uint_t>(previous[idx + 0]) << 16) |
						       (static_cast<uint_t>(previous[idx + 1]) << 8) | static_cast<uint_t>(previous[idx + 2]));
					};
					const int_t c0 = packed(x * 2, y * 2);
					const int_t c1 = packed(x * 2 + 1, y * 2);
					const int_t c2 = packed(x * 2 + 1, y * 2 + 1);
					const int_t c3 = packed(x * 2, y * 2 + 1);
					const int_t c = weightedAverageColor(weightedAverageColor(c0, c1), weightedAverageColor(c2, c3));
					const size_t out = ((size_t)y * mipWidth + x) * 4u;
					current[out + 0] = (byte_t)((c >> 16) & 0xff);
					current[out + 1] = (byte_t)((c >> 8) & 0xff);
					current[out + 2] = (byte_t)(c & 0xff);
					current[out + 3] = (byte_t)((c >> 24) & 0xff);
				}
			}
			if (nativeTextureReady)
				renderTextureImageRgba(level, mipWidth, mipHeight, current.data());
			previous.swap(current);
			previousWidth = mipWidth;
			previousHeight = mipHeight;
		}
	}
}

void RenderEngine::updateTextureSubImage(const std::vector<int_t> &ai, int_t width, int_t height, int_t texture)
{
	const int_t mipmapLevel = resolveMipmapLevel(Config::getMipmapLevel(), width, height);
	useMipmaps = mipmapLevel > 0;
	renderBindTexture(texture);
	renderTextureParameters(blurTexture, mipmapLevel > 0, clampTexture);
#if PLATFORM_TEXTURE_QUALITY_CONTROLS
	renderApplyTextureQuality(blurTexture, mipmapLevel, Config::isMipmapLinear(), Config::getAnisotropicFilterLevel());
#endif

	const std::size_t pixelCount = BufferedImage::checkedPixelCount(width, height);
	if (ai.size() > pixelCount)
		throw std::out_of_range("RenderEngine::updateTextureSubImage: source exceeds target dimensions");
	std::vector<unsigned char> pixels(BufferedImage::checkedRgbaByteCount(width, height));
	for (std::size_t i = 0; i < ai.size(); i++)
	{
		int_t a = (ai[i] >> 24) & 0xff;
		int_t r = (ai[i] >> 16) & 0xff;
		int_t g = (ai[i] >> 8) & 0xff;
		int_t b = ai[i] & 0xff;
		if (options != nullptr && options->anaglyph)
		{
			int_t nr = (r * 30 + g * 59 + b * 11) / 100;
			int_t ng = (r * 30 + g * 70) / 100;
			int_t nb = (r * 30 + b * 70) / 100;
			r = nr;
			g = ng;
			b = nb;
		}
		pixels[i * 4u + 0] = (unsigned char)r;
		pixels[i * 4u + 1] = (unsigned char)g;
		pixels[i * 4u + 2] = (unsigned char)b;
		pixels[i * 4u + 3] = (unsigned char)a;
	}
	renderTextureSubImageRgba(0, 0, 0, width, height, pixels.data());
}

void RenderEngine::updateTextureSubImageRegion(int_t texture, int_t x, int_t y, int_t width, int_t height, const byte_t *pixels)
{
	if (texture < 0 || pixels == nullptr || width <= 0 || height <= 0)
		return;
	int_t targetWidth = 0;
	int_t targetHeight = 0;
	if (!getTextureDimensions(texture, &targetWidth, &targetHeight) || x < 0 || y < 0 || x + width > targetWidth || y + height > targetHeight)
		return;

	const int_t mipmapLevel = resolveMipmapLevel(Config::getMipmapLevel(), targetWidth, targetHeight);
	renderBindTexture(texture);
	renderTextureParameters(blurTexture, mipmapLevel > 0, clampTexture);
#if PLATFORM_TEXTURE_QUALITY_CONTROLS
	renderApplyTextureQuality(blurTexture, mipmapLevel, Config::isMipmapLinear(), Config::getAnisotropicFilterLevel());
#endif
	renderTextureSubImageRgba(0, x, y, width, height, pixels);

	std::vector<byte_t> previous(pixels, pixels + static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4u);
	int_t previousWidth = width;
	int_t previousHeight = height;
	for (int_t level = 1; level <= mipmapLevel && previousWidth > 1 && previousHeight > 1; ++level)
	{
		const int_t mipWidth = previousWidth / 2;
		const int_t mipHeight = previousHeight / 2;
		if (mipWidth <= 0 || mipHeight <= 0)
			break;
		std::vector<byte_t> current(static_cast<std::size_t>(mipWidth) * static_cast<std::size_t>(mipHeight) * 4u);
		for (int_t py = 0; py < mipHeight; ++py)
		{
			for (int_t px = 0; px < mipWidth; ++px)
			{
				auto packed = [&](int_t sx, int_t sy) -> int_t
				{
					const std::size_t idx = (static_cast<std::size_t>(sy) * static_cast<std::size_t>(previousWidth) + static_cast<std::size_t>(sx)) * 4u;
					return JavaArithmetic::intFromBits((static_cast<uint_t>(previous[idx + 3]) << 24) |
					       (static_cast<uint_t>(previous[idx + 0]) << 16) |
					       (static_cast<uint_t>(previous[idx + 1]) << 8) |
					       static_cast<uint_t>(previous[idx + 2]));
				};
				const int_t c = weightedAverageColor(
					weightedAverageColor(packed(px * 2, py * 2), packed(px * 2 + 1, py * 2)),
					weightedAverageColor(packed(px * 2 + 1, py * 2 + 1), packed(px * 2, py * 2 + 1)));
				const std::size_t out = (static_cast<std::size_t>(py) * static_cast<std::size_t>(mipWidth) + static_cast<std::size_t>(px)) * 4u;
				current[out + 0] = static_cast<byte_t>((c >> 16) & 0xff);
				current[out + 1] = static_cast<byte_t>((c >> 8) & 0xff);
				current[out + 2] = static_cast<byte_t>(c & 0xff);
				current[out + 3] = static_cast<byte_t>((c >> 24) & 0xff);
			}
		}
		renderTextureSubImageRgba(level, x >> level, y >> level, mipWidth, mipHeight, current.data());
		previous.swap(current);
		previousWidth = mipWidth;
		previousHeight = mipHeight;
	}
}

void RenderEngine::updateCustomAnimations()
{
	auto categoryEnabled = [](CustomAnimationCategory category) -> bool
	{
		switch (category)
		{
		case CustomAnimationCategory::Water:
			return Config::isAnimatedWater() && !Config::isGeneratedWater();
		case CustomAnimationCategory::Lava:
			return Config::isAnimatedLava() && !Config::isGeneratedLava();
		case CustomAnimationCategory::Fire:
			return Config::isAnimatedFire();
		case CustomAnimationCategory::Portal:
			return Config::isAnimatedPortal();
		case CustomAnimationCategory::Terrain:
			return Config::isAnimatedTerrain();
		case CustomAnimationCategory::Items:
			return Config::isAnimatedItems();
		case CustomAnimationCategory::Generic:
		default:
			return Config::isAnimatedTextures();
		}
	};

	for (const std::unique_ptr<CustomAnimation> &animation : textureAnimations)
	{
		if (!animation || !animation->isValid())
			continue;

		const bool animated = categoryEnabled(animation->category);
		const byte_t *frame = nullptr;
		if (animated)
		{
			if (!animation->nextFrame())
				continue;
			frame = animation->getActiveFrameData();
		}
		else
		{
			if (dynamicTexturesUpdated)
				continue;
			frame = animation->getFirstFrameData();
		}

		const int_t texture = getTexture(animation->destTexture);
		if (texture >= 0 && frame != nullptr)
			updateTextureSubImageRegion(texture, animation->destX, animation->destY,
			                            animation->frameWidth, animation->frameHeight, frame);
	}
}

void RenderEngine::loadCustomAnimations()
{
	textureAnimations.clear();
	specialTextureAnimations.clear();
	terrainTextureFxAnimations.clear();
	itemTextureFxAnimations.clear();
	defaultTerrainFxTiles.clear();
	defaultTerrainFxTileWidth = 0;

#if !PLATFORM_OPTIFINE_CUSTOM_ANIMATIONS
	// No custom animation probing on this platform. The default tiles are
	// still needed: they are what the special TextureFX fall back to for a
	// static frame when a category is disabled (see updateDefaultTerrainTextureFx).
	rebuildDefaultTerrainFxTiles();
	customAnimationTerrainTileWidth = Config::getIconWidthTerrain();
	customAnimationItemTileWidth = Config::getIconWidthItems();
	return;
#endif

	auto normalizePath = [](std::string path) -> std::string
	{
		path = OptiFineResource::trim(path);
		if (!path.empty() && path.front() != '/')
			path.insert(path.begin(), '/');
		return path;
	};

	auto resolveAnimationSource = [&](const std::string &path) -> std::string
	{
		if (path.empty())
			return std::string();
		if (hasResource(path))
			return path;
		const std::string fallback = "/anim" + path;
		return hasResource(fallback) ? fallback : std::string();
	};

	auto sanitizeFixedAtlasProperties = [](std::map<std::string, std::string> &properties)
	{
		properties.erase("from");
		properties.erase("to");
		properties.erase("x");
		properties.erase("y");
		properties.erase("w");
		properties.erase("h");
	};

	auto makeAnimation = [&](const std::map<std::string, std::string> &properties,
	                         const std::string &fallbackSource,
	                         const std::string &fallbackDest,
	                         int_t fallbackX, int_t fallbackY,
	                         int_t fallbackWidth, int_t fallbackHeight,
	                         CustomAnimationCategory category) -> std::unique_ptr<CustomAnimation>
	{
		std::string source = fallbackSource;
		std::string dest = fallbackDest;
		auto fromIt = properties.find("from");
		if (fromIt != properties.end()) source = fromIt->second;
		auto toIt = properties.find("to");
		if (toIt != properties.end()) dest = toIt->second;
		source = normalizePath(source);
		dest = normalizePath(dest);
		if (source.empty() || dest.empty())
			return nullptr;

		int_t x = fallbackX;
		int_t y = fallbackY;
		int_t width = fallbackWidth;
		int_t height = fallbackHeight;
		auto readInt = [&](const char *key, int_t current) -> int_t
		{
			auto it = properties.find(key);
			return it == properties.end() ? current : OptiFineResource::parseInt(it->second, current);
		};
		x = readInt("x", x);
		y = readInt("y", y);
		width = readInt("w", width);
		height = readInt("h", height);
		if (x < 0 || y < 0 || width <= 0 || height <= 0 || width > Config::getMaxDynamicTileWidth() || height > Config::getMaxDynamicTileWidth())
			return nullptr;

		const std::string resolvedSource = resolveAnimationSource(source);
		if (resolvedSource.empty())
			return nullptr;
		std::unique_ptr<BufferedImage> image = readTextureImage(getResourceAsStream(resolvedSource));
		if (!image || image->getWidth() <= 0 || image->getHeight() <= 0)
			return nullptr;

		const int_t sourceWidth = image->getWidth();
		const int_t sourceHeight = image->getHeight();
		const int_t scaledHeight = sourceWidth == width ? sourceHeight : JavaArithmetic::intMul(sourceHeight, width) / sourceWidth;
		if (scaledHeight <= 0)
			return nullptr;
		std::vector<byte_t> data(static_cast<std::size_t>(width) * static_cast<std::size_t>(scaledHeight) * 4u);
		const unsigned char *raw = image->getRawPixels();
		for (int_t py = 0; py < scaledHeight; ++py)
		{
			const int_t sy = py * sourceHeight / scaledHeight;
			for (int_t px = 0; px < width; ++px)
			{
				const int_t sx = px * sourceWidth / width;
				const std::size_t src = (static_cast<std::size_t>(sy) * static_cast<std::size_t>(sourceWidth) + static_cast<std::size_t>(sx)) * 4u;
				const std::size_t dst = (static_cast<std::size_t>(py) * static_cast<std::size_t>(width) + static_cast<std::size_t>(px)) * 4u;
				int_t red = raw[src + 0] & 0xff;
				int_t green = raw[src + 1] & 0xff;
				int_t blue = raw[src + 2] & 0xff;
				if (options != nullptr && options->anaglyph)
				{
					const int_t convertedRed = (red * 30 + green * 59 + blue * 11) / 100;
					const int_t convertedGreen = (red * 30 + green * 70) / 100;
					const int_t convertedBlue = (red * 30 + blue * 70) / 100;
					red = convertedRed;
					green = convertedGreen;
					blue = convertedBlue;
				}
				data[dst + 0] = static_cast<byte_t>(red);
				data[dst + 1] = static_cast<byte_t>(green);
				data[dst + 2] = static_cast<byte_t>(blue);
				data[dst + 3] = raw[src + 3];
			}
		}

		std::unique_ptr<CustomAnimation> animation(new CustomAnimation(source, std::move(data), width, height, properties, 1));
		if (!animation->isValid())
			return nullptr;
		animation->destTexture = dest;
		animation->destX = x;
		animation->destY = y;
		animation->category = category;
		return animation;
	};

	// Generic OptiFine C6 animations live under /anim/*.properties. Files named
	// custom_* are legacy atlas replacements and are registered separately below.
	for (const std::string &propertyPath : listResources("/anim/", ".properties"))
	{
		const std::size_t slash = propertyPath.find_last_of('/');
		const std::string fileName = slash == std::string::npos ? propertyPath : propertyPath.substr(slash + 1);
		if (fileName.rfind("custom_", 0) == 0)
			continue;
		const std::map<std::string, std::string> properties = OptiFineResource::readProperties(this, propertyPath);
		std::unique_ptr<CustomAnimation> animation = makeAnimation(
			properties, "", "", -1, -1, -1, -1, CustomAnimationCategory::Generic);
		if (animation)
			textureAnimations.push_back(std::move(animation));
	}

	auto isSpecialTerrainIcon = [](int_t icon) -> bool
	{
		if (Block::waterMoving != nullptr && (icon == Block::waterMoving->blockIndexInTexture || icon == Block::waterMoving->blockIndexInTexture + 1))
			return true;
		if (Block::lavaMoving != nullptr && (icon == Block::lavaMoving->blockIndexInTexture || icon == Block::lavaMoving->blockIndexInTexture + 1))
			return true;
		if (Block::portal != nullptr && icon == Block::portal->blockIndexInTexture)
			return true;
		return Block::fire != nullptr && (icon == Block::fire->blockIndexInTexture || icon == Block::fire->blockIndexInTexture + 16);
	};

	auto registerLegacyAtlasAnimations = [&](const char *prefix, const char *destTexture, int_t iconWidth, CustomAnimationCategory category)
	{
		if (iconWidth <= 0)
			return;
		for (int_t icon = 0; icon < 256; ++icon)
		{
			if (category == CustomAnimationCategory::Terrain && isSpecialTerrainIcon(icon))
				continue;
			const std::string source = std::string(prefix) + std::to_string(icon) + ".png";
			if (resolveAnimationSource(source).empty())
				continue;
			const std::string propertyPath = source.substr(0, source.size() - 4) + ".properties";
			std::map<std::string, std::string> properties = OptiFineResource::readProperties(this, propertyPath);
			if (properties.empty())
				properties = OptiFineResource::readProperties(this, "/anim" + propertyPath);
			sanitizeFixedAtlasProperties(properties);
			std::unique_ptr<CustomAnimation> animation = makeAnimation(
				properties, source, destTexture,
				(icon % 16) * iconWidth, (icon / 16) * iconWidth, iconWidth, iconWidth, category);
			if (!animation)
				continue;
			if (category == CustomAnimationCategory::Terrain)
				terrainTextureFxAnimations[icon] = std::move(animation);
			else if (category == CustomAnimationCategory::Items)
				itemTextureFxAnimations[icon] = std::move(animation);
		}
	};

	auto registerSpecialTextureAnimation = [&](const char *source, int_t icon, CustomAnimationCategory category)
	{
		const int_t iconWidth = Config::getIconWidthTerrain();
		if (iconWidth <= 0 || icon < 0 || resolveAnimationSource(source).empty())
			return;

		const std::string sourcePath(source);
		const std::string propertyPath = sourcePath.substr(0, sourcePath.size() - 4) + ".properties";
		std::map<std::string, std::string> properties = OptiFineResource::readProperties(this, propertyPath);
		if (properties.empty())
			properties = OptiFineResource::readProperties(this, "/anim" + propertyPath);

		// C6's named water/lava/fire/portal animations use .properties only for
		// timeline metadata. Their source, destination and tile geometry are fixed
		// by updateCustomTexture(), unlike generic /anim/*.properties animations.
		sanitizeFixedAtlasProperties(properties);

		std::unique_ptr<CustomAnimation> animation = makeAnimation(
			properties, sourcePath, "/terrain.png",
			(icon % 16) * iconWidth, (icon / 16) * iconWidth, iconWidth, iconWidth, category);
		if (animation)
			specialTextureAnimations[icon] = std::move(animation);
	};

	if (Block::waterMoving != nullptr)
	{
		registerSpecialTextureAnimation("/custom_water_still.png", Block::waterMoving->blockIndexInTexture, CustomAnimationCategory::Water);
		registerSpecialTextureAnimation("/custom_water_flowing.png", Block::waterMoving->blockIndexInTexture + 1, CustomAnimationCategory::Water);
	}
	if (Block::lavaMoving != nullptr)
	{
		registerSpecialTextureAnimation("/custom_lava_still.png", Block::lavaMoving->blockIndexInTexture, CustomAnimationCategory::Lava);
		registerSpecialTextureAnimation("/custom_lava_flowing.png", Block::lavaMoving->blockIndexInTexture + 1, CustomAnimationCategory::Lava);
	}
	if (Block::portal != nullptr)
		registerSpecialTextureAnimation("/custom_portal.png", Block::portal->blockIndexInTexture, CustomAnimationCategory::Portal);
	if (Block::fire != nullptr)
	{
		registerSpecialTextureAnimation("/custom_fire_n_s.png", Block::fire->blockIndexInTexture, CustomAnimationCategory::Fire);
		registerSpecialTextureAnimation("/custom_fire_e_w.png", Block::fire->blockIndexInTexture + 16, CustomAnimationCategory::Fire);
	}

	rebuildDefaultTerrainFxTiles();

	registerLegacyAtlasAnimations("/custom_terrain_", "/terrain.png", Config::getIconWidthTerrain(), CustomAnimationCategory::Terrain);
	registerLegacyAtlasAnimations("/custom_item_", "/gui/items.png", Config::getIconWidthItems(), CustomAnimationCategory::Items);
	customAnimationTerrainTileWidth = Config::getIconWidthTerrain();
	customAnimationItemTileWidth = Config::getIconWidthItems();
}

void RenderEngine::syncCustomAnimationTileWidths()
{
	const int_t terrainTileWidth = Config::getIconWidthTerrain();
	const int_t itemTileWidth = Config::getIconWidthItems();
	if (terrainTileWidth == customAnimationTerrainTileWidth && itemTileWidth == customAnimationItemTileWidth)
		return;

	loadCustomAnimations();
	dynamicTexturesUpdated = false;
}

void RenderEngine::rebuildDefaultTerrainFxTiles()
{
	defaultTerrainFxTiles.clear();
	defaultTerrainFxTileWidth = Config::getIconWidthTerrain();
	if (defaultTerrainFxTileWidth <= 0)
		return;

	std::unique_ptr<BufferedImage> terrain = readTextureImage(getResourceAsStream("/terrain.png"));
	if (!terrain || terrain->getWidth() <= 0 || terrain->getHeight() <= 0)
		return;

	std::vector<int_t> icons;
	if (Block::waterMoving != nullptr)
	{
		icons.push_back(Block::waterMoving->blockIndexInTexture);
		icons.push_back(Block::waterMoving->blockIndexInTexture + 1);
	}
	if (Block::lavaMoving != nullptr)
	{
		icons.push_back(Block::lavaMoving->blockIndexInTexture);
		icons.push_back(Block::lavaMoving->blockIndexInTexture + 1);
	}

	const int_t sourceWidth = terrain->getWidth();
	const int_t sourceHeight = terrain->getHeight();
	const int_t targetAtlasWidth = defaultTerrainFxTileWidth * 16;
	const int_t targetAtlasHeight = defaultTerrainFxTileWidth * 16;
	const unsigned char *raw = terrain->getRawPixels();
	for (int_t icon : icons)
	{
		if (icon < 0 || icon >= 256)
			continue;
		std::vector<byte_t> tile(static_cast<std::size_t>(defaultTerrainFxTileWidth) *
		                         static_cast<std::size_t>(defaultTerrainFxTileWidth) * 4u);
		const int_t tileX = icon % 16;
		const int_t tileY = icon / 16;
		for (int_t y = 0; y < defaultTerrainFxTileWidth; ++y)
		{
			const int_t targetY = tileY * defaultTerrainFxTileWidth + y;
			const int_t sourceY = targetY * sourceHeight / targetAtlasHeight;
			for (int_t x = 0; x < defaultTerrainFxTileWidth; ++x)
			{
				const int_t targetX = tileX * defaultTerrainFxTileWidth + x;
				const int_t sourceX = targetX * sourceWidth / targetAtlasWidth;
				const std::size_t src = (static_cast<std::size_t>(sourceY) * static_cast<std::size_t>(sourceWidth) +
				                         static_cast<std::size_t>(sourceX)) * 4u;
				const std::size_t dst = (static_cast<std::size_t>(y) * static_cast<std::size_t>(defaultTerrainFxTileWidth) +
				                         static_cast<std::size_t>(x)) * 4u;
				int_t red = raw[src + 0] & 0xff;
				int_t green = raw[src + 1] & 0xff;
				int_t blue = raw[src + 2] & 0xff;
				if (options != nullptr && options->anaglyph)
				{
					const int_t convertedRed = (red * 30 + green * 59 + blue * 11) / 100;
					const int_t convertedGreen = (red * 30 + green * 70) / 100;
					const int_t convertedBlue = (red * 30 + blue * 70) / 100;
					red = convertedRed;
					green = convertedGreen;
					blue = convertedBlue;
				}
				tile[dst + 0] = static_cast<byte_t>(red);
				tile[dst + 1] = static_cast<byte_t>(green);
				tile[dst + 2] = static_cast<byte_t>(blue);
				tile[dst + 3] = raw[src + 3];
			}
		}
		defaultTerrainFxTiles[icon] = std::move(tile);
	}
}

const std::vector<byte_t> *RenderEngine::getDefaultTerrainFxTile(int_t iconIndex) const
{
	auto it = defaultTerrainFxTiles.find(iconIndex);
	return it == defaultTerrainFxTiles.end() ? nullptr : &it->second;
}

void RenderEngine::uploadTextureFxTile(TextureFX *texturefx, int_t texture, int_t tileWidth)
{
	if (texturefx == nullptr || texture < 0 || tileWidth <= 0)
		return;

	tileWidth = Config::limit(tileWidth, 1, Config::getMaxDynamicTileWidth());
	std::vector<byte_t> scaled(static_cast<std::size_t>(tileWidth) * static_cast<std::size_t>(tileWidth) * 4u);
	for (int_t y = 0; y < tileWidth; ++y)
	{
		const int_t sourceY = y * 16 / tileWidth;
		for (int_t x = 0; x < tileWidth; ++x)
		{
			const int_t sourceX = x * 16 / tileWidth;
			const std::size_t src = (static_cast<std::size_t>(sourceY) * 16u + static_cast<std::size_t>(sourceX)) * 4u;
			const std::size_t dst = (static_cast<std::size_t>(y) * static_cast<std::size_t>(tileWidth) + static_cast<std::size_t>(x)) * 4u;
			std::memcpy(&scaled[dst], &texturefx->imageData[src], 4u);
		}
	}

	for (int_t tileX = 0; tileX < texturefx->tileSize; ++tileX)
	{
		for (int_t tileY = 0; tileY < texturefx->tileSize; ++tileY)
		{
			const int_t x = (texturefx->iconIndex % 16) * tileWidth + tileX * tileWidth;
			const int_t y = (texturefx->iconIndex / 16) * tileWidth + tileY * tileWidth;
			updateTextureSubImageRegion(texture, x, y, tileWidth, tileWidth, scaled.data());
		}
	}
}

SpecialTextureFxResult RenderEngine::updateDefaultTerrainTextureFx(TextureFX *texturefx, int_t texture, int_t tileWidth)
{
	if (isDefaultTexturePack())
		return SpecialTextureFxResult::NotHandled;
	if (texturefx == nullptr || texture < 0 || tileWidth <= 0 || defaultTerrainFxTileWidth != tileWidth)
		return SpecialTextureFxResult::NotHandled;

	const int_t icon = texturefx->iconIndex;
	bool scrolling = false;
	int_t scrollDiv = 1;
	bool recognized = false;

	if (Block::waterMoving != nullptr && (icon == Block::waterMoving->blockIndexInTexture || icon == Block::waterMoving->blockIndexInTexture + 1))
	{
		if (Config::isGeneratedWater())
			return SpecialTextureFxResult::NotHandled;
		recognized = true;
		scrolling = icon == Block::waterMoving->blockIndexInTexture + 1 && Config::isAnimatedWater();
	}
	else if (Block::lavaMoving != nullptr && (icon == Block::lavaMoving->blockIndexInTexture || icon == Block::lavaMoving->blockIndexInTexture + 1))
	{
		if (Config::isGeneratedLava())
			return SpecialTextureFxResult::NotHandled;
		recognized = true;
		scrolling = icon == Block::lavaMoving->blockIndexInTexture + 1 && Config::isAnimatedLava();
		scrollDiv = 3;
	}

	if (!recognized)
		return SpecialTextureFxResult::NotHandled;
	if (!scrolling && dynamicTexturesUpdated)
		return SpecialTextureFxResult::HandledNoUpload;

	const std::vector<byte_t> *tile = getDefaultTerrainFxTile(icon);
	if (tile == nullptr || tile->size() != static_cast<std::size_t>(tileWidth) * static_cast<std::size_t>(tileWidth) * 4u)
		return SpecialTextureFxResult::NotHandled;

	const byte_t *pixels = tile->data();
	std::vector<byte_t> scrolled;
	if (scrolling)
	{
		const int_t moveRows = tileWidth - (dynamicTextureTickCounter / scrollDiv) % tileWidth;
		const std::size_t offset = static_cast<std::size_t>(moveRows) * static_cast<std::size_t>(tileWidth) * 4u;
		scrolled.resize(tile->size());
		const std::size_t tail = tile->size() - offset;
		if (tail > 0)
			std::memcpy(scrolled.data(), tile->data() + offset, tail);
		if (offset > 0)
			std::memcpy(scrolled.data() + tail, tile->data(), offset);
		pixels = scrolled.data();
	}

	for (int_t tileX = 0; tileX < texturefx->tileSize; ++tileX)
	{
		for (int_t tileY = 0; tileY < texturefx->tileSize; ++tileY)
		{
			const int_t x = (icon % 16) * tileWidth + tileX * tileWidth;
			const int_t y = (icon / 16) * tileWidth + tileY * tileWidth;
			updateTextureSubImageRegion(texture, x, y, tileWidth, tileWidth, pixels);
		}
	}
	return SpecialTextureFxResult::HandledUpload;
}

SpecialTextureFxResult RenderEngine::updateSpecialTextureFx(TextureFX *texturefx, int_t texture, int_t tileWidth)
{
	if (texturefx == nullptr)
		return SpecialTextureFxResult::NotHandled;

	auto updateAnimation = [&](CustomAnimation *animation, bool animated) -> SpecialTextureFxResult
	{
		if (animation == nullptr || !animation->isValid())
			return SpecialTextureFxResult::NotHandled;

		const byte_t *frame = nullptr;
		if (animated)
		{
			if (!animation->nextFrame())
				return SpecialTextureFxResult::HandledNoUpload;
			frame = animation->getActiveFrameData();
		}
		else
		{
			if (dynamicTexturesUpdated)
				return SpecialTextureFxResult::HandledNoUpload;
			frame = animation->getFirstFrameData();
		}
		if (frame == nullptr)
			return SpecialTextureFxResult::HandledNoUpload;

		for (int_t tileX = 0; tileX < texturefx->tileSize; ++tileX)
		{
			for (int_t tileY = 0; tileY < texturefx->tileSize; ++tileY)
			{
				const int_t x = (texturefx->iconIndex % 16) * tileWidth + tileX * tileWidth;
				const int_t y = (texturefx->iconIndex / 16) * tileWidth + tileY * tileWidth;
				updateTextureSubImageRegion(texture, x, y, animation->frameWidth, animation->frameHeight, frame);
			}
		}
		return SpecialTextureFxResult::HandledUpload;
	};

	const int_t icon = texturefx->iconIndex;
	if (texturefx->tileImage == 0)
	{
		bool generated = false;
		bool animated = true;
		bool special = false;
		if (Block::waterMoving != nullptr && (icon == Block::waterMoving->blockIndexInTexture || icon == Block::waterMoving->blockIndexInTexture + 1))
		{
			special = true;
			generated = Config::isGeneratedWater();
			animated = Config::isAnimatedWater();
		}
		else if (Block::lavaMoving != nullptr && (icon == Block::lavaMoving->blockIndexInTexture || icon == Block::lavaMoving->blockIndexInTexture + 1))
		{
			special = true;
			generated = Config::isGeneratedLava();
			animated = Config::isAnimatedLava();
		}
		else if (Block::portal != nullptr && icon == Block::portal->blockIndexInTexture)
		{
			special = true;
			animated = Config::isAnimatedPortal();
		}
		else if (Block::fire != nullptr && (icon == Block::fire->blockIndexInTexture || icon == Block::fire->blockIndexInTexture + 16))
		{
			special = true;
			animated = Config::isAnimatedFire();
		}

		if (special)
		{
			if (generated)
				return SpecialTextureFxResult::NotHandled;

			auto animationIt = specialTextureAnimations.find(icon);
			if (animationIt != specialTextureAnimations.end())
			{
				const SpecialTextureFxResult customResult = updateAnimation(animationIt->second.get(), animated);
				if (customResult != SpecialTextureFxResult::NotHandled)
					return customResult;
			}

			return updateDefaultTerrainTextureFx(texturefx, texture, tileWidth);
		}

		if (Config::isAnimatedTerrain())
		{
			auto animationIt = terrainTextureFxAnimations.find(icon);
			if (animationIt != terrainTextureFxAnimations.end())
				return updateAnimation(animationIt->second.get(), true);
		}
		return SpecialTextureFxResult::NotHandled;
	}

	if (texturefx->tileImage == 1 && Config::isAnimatedItems())
	{
		auto animationIt = itemTextureFxAnimations.find(icon);
		if (animationIt != itemTextureFxAnimations.end())
			return updateAnimation(animationIt->second.get(), true);
	}

	return SpecialTextureFxResult::NotHandled;
}

bool RenderEngine::updateStaticProceduralTextureFx(TextureFX *texturefx)
{
	if (texturefx == nullptr || texturefx->tileImage != 0 || dynamicTexturesUpdated || !isDefaultTexturePack())
		return false;

	const int_t icon = texturefx->iconIndex;
	const bool portal = Block::portal != nullptr && icon == Block::portal->blockIndexInTexture;
	const bool fire = Block::fire != nullptr && (icon == Block::fire->blockIndexInTexture || icon == Block::fire->blockIndexInTexture + 16);
	if (!portal && !fire)
		return false;

	const bool anaglyph = options != nullptr && options->anaglyph;
	texturefx->anaglyphEnabled = anaglyph;
	bool &hasFrame = textureFxHasFrame[texturefx];
	auto anaglyphIt = textureFxFrameAnaglyph.find(texturefx);
	if (hasFrame && anaglyphIt != textureFxFrameAnaglyph.end() && anaglyphIt->second == anaglyph)
		return true;

	if (fire && !hasFrame)
	{
		for (int_t i = 0; i < 20; ++i)
			texturefx->onTick();
	}
	else
	{
		texturefx->onTick();
	}
	hasFrame = true;
	textureFxFrameAnaglyph[texturefx] = anaglyph;
	return true;
}

void RenderEngine::deleteTexture(int_t i)
{
	textureNameToImageMap.erase(i);
	textureHighPrecision.erase(i);
	textureDimensions.erase(i);
	int texture = static_cast<int>(i);
	renderDeleteTextures(1, &texture);
}

void RenderEngine::releaseTexture(const std::string &s)
{
#if PLATFORM_PS2
	if (asyncTextureLoads.find(s) != asyncTextureLoads.end())
		Ps2AsyncAssetLoader::release(normalizedTexturePath(s));
	asyncTextureLoads.erase(s);
#endif
	auto it = textureMap.find(s);
	if (it == textureMap.end())
		return;

	const int_t texture = it->second;
	textureMap.erase(it);
	failedTextures.erase(s);
	field_28151_c.erase(s);
	deleteTexture(texture);
}

void RenderEngine::clearDecodedTextureCache()
{
	field_28151_c.clear();
}

int_t RenderEngine::getTextureForDownloadableImage(const std::string &s, const std::string &fallback)
{
	ThreadDownloadImageData *threaddownloadimagedata = nullptr;
	auto it = urlToImageDataMap.find(s);
	if (it != urlToImageDataMap.end())
		threaddownloadimagedata = it->second;
	BufferedImage *downloadedImage = threaddownloadimagedata != nullptr ? threaddownloadimagedata->image.load() : nullptr;
	if (threaddownloadimagedata != nullptr && downloadedImage != nullptr && !threaddownloadimagedata->textureSetupComplete)
	{
		if (threaddownloadimagedata->textureName < 0)
			threaddownloadimagedata->textureName = allocateAndSetupTexture(downloadedImage);
		else
			setupTexture(downloadedImage, threaddownloadimagedata->textureName);
		threaddownloadimagedata->textureSetupComplete = true;
	}
	if (threaddownloadimagedata == nullptr || threaddownloadimagedata->textureName < 0)
		return fallback.empty() ? -1 : getTexture(fallback);
	return threaddownloadimagedata->textureName;
}

ThreadDownloadImageData *RenderEngine::obtainImageData(const std::string &s, ImageBuffer *imagebuffer)
{
	auto it = urlToImageDataMap.find(s);
	if (it == urlToImageDataMap.end())
	{
		ThreadDownloadImageData *data = new ThreadDownloadImageData(s, imagebuffer);
		urlToImageDataMap[s] = data;
		return data;
	}
	// The existing download already owns its ImageBuffer. Java would let the
	// unused argument be collected; C++ must release the newly supplied one.
	delete imagebuffer;
	it->second->referenceCount++;
	return it->second;
}

void RenderEngine::releaseImageData(const std::string &s)
{
	auto it = urlToImageDataMap.find(s);
	if (it == urlToImageDataMap.end())
		return;
	ThreadDownloadImageData *data = it->second;
	data->referenceCount--;
	if (data->referenceCount == 0)
	{
		if (data->textureName >= 0)
			deleteTexture(data->textureName);
		delete data;
		urlToImageDataMap.erase(it);
	}
}

void RenderEngine::registerTextureFX(TextureFX *texturefx, bool takeOwnership)
{
	if (texturefx == nullptr)
		return;
	textureList.push_back(texturefx);
	if (takeOwnership && std::find(ownedTextureFx.begin(), ownedTextureFx.end(), texturefx) == ownedTextureFx.end())
		ownedTextureFx.push_back(texturefx);
	if (texturefx->isAnimationEnabled())
	{
		texturefx->anaglyphEnabled = options != nullptr && options->anaglyph;
		texturefx->onTick();
		textureFxHasFrame[texturefx] = true;
		textureFxFrameAnaglyph[texturefx] = texturefx->anaglyphEnabled;
	}
	else
	{
		textureFxHasFrame[texturefx] = false;
	}
	dynamicTexturesUpdated = false;
}

void RenderEngine::updateDynamicTextures()
{
	const int_t terrainTexture = getTexture("/terrain.png");
	const int_t itemsTexture = getTexture("/gui/items.png");
	syncCustomAnimationTileWidths();
	++dynamicTextureTickCounter;

	for (TextureFX *texturefx : textureList)
	{
		if (texturefx == nullptr)
			continue;

		texturefx->anaglyphEnabled = options != nullptr && options->anaglyph;
		const int_t texture = texturefx->tileImage == 1 ? itemsTexture : terrainTexture;
		const int_t tileWidth = texturefx->tileImage == 1 ? Config::getIconWidthItems() : Config::getIconWidthTerrain();
		const SpecialTextureFxResult specialResult = updateSpecialTextureFx(texturefx, texture, tileWidth);
		if (specialResult != SpecialTextureFxResult::NotHandled)
			continue;

		if (texturefx->isAnimationEnabled())
		{
			texturefx->onTick();
			textureFxHasFrame[texturefx] = true;
			textureFxFrameAnaglyph[texturefx] = texturefx->anaglyphEnabled;
		}
		else if (!updateStaticProceduralTextureFx(texturefx))
		{
			continue;
		}

		uploadTextureFxTile(texturefx, texture, tileWidth);
	}

	// Preserve the legacy second pass for TextureFX instances targeting their
	// own texture object instead of the terrain/items atlas. These targets are
	// not necessarily created through RenderEngine, so dimensions may be unknown.
	for (TextureFX *texturefx : textureList)
	{
		if (texturefx == nullptr || !texturefx->isAnimationEnabled() || texturefx->textureId <= 0)
			continue;

		int_t targetWidth = 0;
		int_t targetHeight = 0;
		if (getTextureDimensions(texturefx->textureId, &targetWidth, &targetHeight) && targetWidth >= 16 && targetHeight >= 16)
		{
			updateTextureSubImageRegion(texturefx->textureId, 0, 0, 16, 16, texturefx->imageData);
		}
		else
		{
			renderBindTexture(texturefx->textureId);
			renderTextureSubImageRgba(0, 0, 0, 16, 16, texturefx->imageData);
		}
	}

	updateCustomAnimations();
	dynamicTexturesUpdated = true;
}

void RenderEngine::refreshTextures()
{
#if PLATFORM_PS2
	Ps2AsyncAssetLoader::cancelAll();
	asyncTextureLoads.clear();
#endif
	dynamicTexturesUpdated = false;
	for (auto &entry : urlToImageDataMap)
		if (entry.second != nullptr)
			entry.second->textureSetupComplete = false;

	for (auto &entry : textureNameToImageMap)
	{
		const bool highPrecision = textureHighPrecision.find(entry.first) != textureHighPrecision.end();
		setupTexture(entry.second.get(), entry.first, false, false, highPrecision);
	}

	// Same decode-and-upload path as getTexture(), including the clamp/blur flag
	// handling and the retry bookkeeping. It used to be duplicated here, which is
	// how the two copies drifted; one of them has to own the failure state or a
	// texture pack switch would silently clear it.
	// Defer PS2 static-mirror eviction until the new texture pack's custom
	// animation table has been rebuilt. A generic /anim/*.properties file may
	// target an arbitrary named texture; classifying against the previous pack
	// could otherwise discard the mirror immediately before the new animation
	// starts issuing sub-image updates.
	for (auto &entry : textureMap)
	{
		if (loadTextureInto(entry.first, entry.second, false))
			failedTextures.erase(entry.first);
		else
			failedTextures[entry.first] = TEXTURE_RETRY_INTERVAL;
	}
	field_28151_c.clear();

	// Texture-pack dependent OptiFine caches must be rebuilt after the base
	// texture map has been refreshed so their texture IDs and color maps refer
	// to the newly selected pack.
	CustomColorizer::update(this);
	ConnectedTextures::update(this);
	NaturalTextures::update(this);
#if PLATFORM_OPTIFINE_RANDOM_MOBS
	RandomMobs::resetTextures();
#endif
	loadCustomAnimations();

	for (const auto &entry : textureMap)
		TextureResidencyPolicy::afterNamedTextureUpload(
			entry.second, isDynamicTextureResource(entry.first));

	// OptiFine C6 updates dynamic textures at the end of a texture-pack refresh.
	// This is required while the game is paused: terrain.png has just replaced
	// every dynamic tile with its atlas source and there may be no normal frame
	// update before the Options screen is drawn again.
	updateDynamicTextures();
}


std::istream *RenderEngine::getResourceAsStream(const std::string &path) const
{
	TexturePackBase *selected = texturePack != nullptr ? texturePack->getSelectedTexturePack() : nullptr;
	return selected != nullptr ? selected->getResourceAsStream(path) : nullptr;
}

bool RenderEngine::hasResource(const std::string &path) const
{
#if PLATFORM_PS2
	// [Issue #3 Fix]: Check AssetPak first, but if not found in the archive, do not prematurely
	// return false. Fall through to getResourceAsStream() so loose files on the disk/USB filesystem
	// (such as custom assets or unbundled files) can still be discovered and loaded properly.
	if (isDefaultTexturePack() && AssetPak::mounted())
	{
		const std::string normalized = normalizedTexturePath(path);
		if (!normalized.empty() && AssetPak::exists("assets" + normalized))
			return true;
	}
#endif
	std::unique_ptr<std::istream> input(getResourceAsStream(path));
	return input != nullptr && *input;
}

bool RenderEngine::isDefaultTexturePack() const
{
	TexturePackBase *selected = texturePack != nullptr ? texturePack->getSelectedTexturePack() : nullptr;
	return selected != nullptr && selected->texturePackFileName == "Default";
}

std::vector<std::string> RenderEngine::listResources(const std::string &prefix, const std::string &suffix) const
{
	TexturePackBase *selected = texturePack != nullptr ? texturePack->getSelectedTexturePack() : nullptr;
	return selected != nullptr ? selected->listResources(prefix, suffix) : std::vector<std::string>();
}

bool RenderEngine::getTextureDimensions(int_t texture, int_t *width, int_t *height) const
{
	auto it = textureDimensions.find(texture);
	if (it == textureDimensions.end())
		return false;
	if (width != nullptr) *width = it->second.first;
	if (height != nullptr) *height = it->second.second;
	return true;
}

// [Issue #3 Fix]: Deterministic texture status checking methods for future contributors.
// Used by UI components and renderers to know whether a texture is genuinely loaded,
// currently loading asynchronously, or has failed, avoiding binding missingTextureImage.
bool RenderEngine::isTextureLoaded(const std::string &s) const
{
	auto it = textureMap.find(s);
	if (it == textureMap.end())
		return false;
#if PLATFORM_PS2
	// If the texture is still in the pending async queue, it is currently holding
	// the placeholder missingTextureImage and is NOT genuinely loaded yet.
	if (asyncTextureLoads.find(s) != asyncTextureLoads.end())
		return false;
#endif
	if (failedTextures.find(s) != failedTextures.end())
		return false;
	return renderTextureIsValid(it->second);
}

bool RenderEngine::isTextureFailed(const std::string &s) const
{
	return failedTextures.find(s) != failedTextures.end();
}

bool RenderEngine::isTexturePending(const std::string &s) const
{
#if PLATFORM_PS2
	return asyncTextureLoads.find(s) != asyncTextureLoads.end();
#else
	(void)s;
	return false;
#endif
}

void RenderEngine::setBackgroundTextureLoadingEnabled(bool enabled)
{
	if (backgroundTextureLoadingEnabled == enabled)
		return;

	backgroundTextureLoadingEnabled = enabled;
#if PLATFORM_PS2
	if (!enabled)
	{
		Ps2AsyncAssetLoader::cancelAll();
		for (const auto &entry : asyncTextureLoads)
			failedTextures[entry.first] = 1;
		asyncTextureLoads.clear();
	}
#endif
}

void RenderEngine::bindTexture(int_t i)
{
	if (i < 0)
		return;
	renderBindTexture(i);
}

std::unique_ptr<BufferedImage> RenderEngine::readTextureImage(std::istream *inputstream)
{
	if (inputstream == nullptr || !*inputstream)
		return nullptr;
	std::unique_ptr<std::istream> guard(inputstream);
	return std::unique_ptr<BufferedImage>(new BufferedImage(BufferedImage::ImageIO_read(*inputstream)));
}

std::unique_ptr<BufferedImage> RenderEngine::createMissingTexture()
{
	std::unique_ptr<BufferedImage> image(new BufferedImage(64, 64));
	std::vector<unsigned char> pixels(64 * 64 * 4);
	for (int_t y = 0; y < 64; y++)
	{
		for (int_t x = 0; x < 64; x++)
		{
			bool black = ((x / 8) + (y / 8)) % 2 == 0;
			int_t i = (y * 64 + x) * 4;
			pixels[i + 0] = black ? 0 : 255;
			pixels[i + 1] = black ? 0 : 255;
			pixels[i + 2] = black ? 0 : 255;
			pixels[i + 3] = 255;
		}
	}
	image->setRGB(0, 0, 64, 64, pixels.data());
	return image;
}

int_t RenderEngine::averageColor(int_t i, int_t j)
{
	int_t k = (i & 0xff000000) >> 24 & 0xff;
	int_t l = (j & 0xff000000) >> 24 & 0xff;
	return JavaArithmetic::intAdd(JavaArithmetic::intShl(JavaArithmetic::intShr(JavaArithmetic::intAdd(k, l), 1), 24), JavaArithmetic::intShr(JavaArithmetic::intAdd(i & 0xfefefe, j & 0xfefefe), 1));
}

int_t RenderEngine::weightedAverageColor(int_t i, int_t j)
{
	int_t k = (i & 0xff000000) >> 24 & 0xff;
	int_t l = (j & 0xff000000) >> 24 & 0xff;
	// Output alpha is the real average of the two source alphas, not a fixed
	// 255 -- otherwise every mip level of a cutout texture (leaves, tall
	// grass, vines...) turns fully opaque instead of fading out.
	int_t c = (k + l) / 2;
	if (k == 0 && l == 0)
	{
		k = 1;
		l = 1;
	}
	else
	{
		if (k == 0)
		{
			i = j;
			c /= 2;
		}
		if (l == 0)
		{
			j = i;
			c /= 2;
		}
	}
	int_t i1 = (i >> 16 & 0xff) * k;
	int_t j1 = (i >> 8 & 0xff) * k;
	int_t k1 = (i & 0xff) * k;
	int_t l1 = (j >> 16 & 0xff) * l;
	int_t i2 = (j >> 8 & 0xff) * l;
	int_t j2 = (j & 0xff) * l;
	int_t k2 = (i1 + l1) / (k + l);
	int_t l2 = (j1 + i2) / (k + l);
	int_t i3 = (k1 + j2) / (k + l);
	return JavaArithmetic::intFromBits((static_cast<uint_t>(c) << 24) | (static_cast<uint_t>(k2) << 16) | (static_cast<uint_t>(l2) << 8) | static_cast<uint_t>(i3));
}
