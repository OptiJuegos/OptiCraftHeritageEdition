#include "RenderEngine.h"
#include "platform/Log.h"
#include "platform/PlatformConfig.h"
#include "platform/TextureResidencyPolicy.h"
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
#include "RandomMobs.h"
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
	, clampTexture(false)
	, blurTexture(false)
	, texturePack(texturepacklist)
	, missingTextureImage(createMissingTexture())
{
	loadCustomAnimations();
}

RenderEngine::~RenderEngine()
{
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
	bool loaded = false;
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
		}
		else if (s.rfind("%blur%", 0) == 0)
		{
			blurTexture = true;
			image = readTextureImage(texturepackbase->getResourceAsStream(s.substr(6)));
		}
		else if (texturepackbase != nullptr)
		{
			image = readTextureImage(texturepackbase->getResourceAsStream(s));
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
		const std::string normalizedPath = normalizedTexturePath(s);
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

int_t RenderEngine::getTexture(const std::string &s)
{
	auto it = textureMap.find(s);
	if (it != textureMap.end())
	{
		// A texture whose load failed is bound to the black/white checkerboard
		// from createMissingTexture(). Java had no way to reach that state at
		// runtime; here it is almost always a *transient* allocation failure on
		// a fragmented heap, because the sheets that fail are the ones loaded
		// mid-game rather than on a loading screen -- /environment/rain.png and
		// /environment/snow.png the first time it rains, /gui/furnace.png the
		// first time a furnace is opened. Caching that id as final is what made
		// the rain stay a checkerboard for the whole session, so retry it into
		// the SAME name on a countdown.
		auto failedIt = failedTextures.find(s);
		if (failedIt != failedTextures.end() && --failedIt->second <= 0)
		{
			if (loadTextureInto(s, it->second))
				failedTextures.erase(failedIt);
			else
				failedIt->second = TEXTURE_RETRY_INTERVAL;
		}
		return it->second;
	}

	std::vector<int_t> single(1);
	GLAllocation::generateTextureNames(single);
	int_t texture = single[0];

	bool loaded = loadTextureInto(s, texture);

	// Some backends can reject an upload because native texture memory is exhausted.
	// Do not cache a broken texture id; retry the whole load on the next bind.
	if (!renderTextureIsValid(texture))
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

int_t RenderEngine::allocateAndSetupTexture(BufferedImage *bufferedimage)
{
	std::vector<int_t> single(1);
	GLAllocation::generateTextureNames(single);
	int_t texture = single[0];
	setupTexture(bufferedimage != nullptr ? bufferedimage : missingTextureImage.get(), texture);
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

void RenderEngine::setupTexture(BufferedImage *bufferedimage, int_t texture, bool tileAtlas, bool terrainAlphaFix)
{
	if (bufferedimage == nullptr)
		bufferedimage = missingTextureImage.get();

	const int_t mipmapLevel = Config::getMipmapLevel();
	useMipmaps = mipmapLevel > 0;
	renderBindTexture(texture);

	int_t width = bufferedimage->getWidth();
	int_t height = bufferedimage->getHeight();
	textureDimensions[texture] = std::make_pair(width, height);
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
	                                                 blurTexture, clampTexture, tileAtlas);
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
		if (!animation || !animation->isValid() || !categoryEnabled(animation->category) || !animation->nextFrame())
			continue;
		const int_t texture = getTexture(animation->destTexture);
		const byte_t *frame = animation->getActiveFrameData();
		if (texture >= 0 && frame != nullptr)
			updateTextureSubImageRegion(texture, animation->destX, animation->destY,
			                            animation->frameWidth, animation->frameHeight, frame);
	}
}

void RenderEngine::loadCustomAnimations()
{
	textureAnimations.clear();

	auto normalizePath = [](std::string path) -> std::string
	{
		path = OptiFineResource::trim(path);
		if (!path.empty() && path.front() != '/')
			path.insert(path.begin(), '/');
		return path;
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

		std::unique_ptr<BufferedImage> image = readTextureImage(getResourceAsStream(source));
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
				std::memcpy(&data[dst], &raw[src], 4u);
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
			if (!hasResource(source))
				continue;
			const std::string propertyPath = source.substr(0, source.size() - 4) + ".properties";
			std::map<std::string, std::string> properties = OptiFineResource::readProperties(this, propertyPath);
			if (properties.empty())
				properties = OptiFineResource::readProperties(this, "/anim" + propertyPath);
			std::unique_ptr<CustomAnimation> animation = makeAnimation(
				properties, source, destTexture,
				(icon % 16) * iconWidth, (icon / 16) * iconWidth, iconWidth, iconWidth, category);
			if (animation)
				textureAnimations.push_back(std::move(animation));
		}
	};

	auto registerNamedTerrainAnimation = [&](const char *source, int_t icon, int_t copies, CustomAnimationCategory category)
	{
		const int_t iconWidth = Config::getIconWidthTerrain();
		if (iconWidth <= 0 || icon < 0 || !hasResource(source))
			return;
		const std::string sourcePath(source);
		const std::string propertyPath = sourcePath.substr(0, sourcePath.size() - 4) + ".properties";
		std::map<std::string, std::string> properties = OptiFineResource::readProperties(this, propertyPath);
		if (properties.empty())
			properties = OptiFineResource::readProperties(this, "/anim" + propertyPath);

		for (int_t copy = 0; copy < copies; ++copy)
		{
			const int_t dx = copy % 2;
			const int_t dy = copy / 2;
			std::unique_ptr<CustomAnimation> animation = makeAnimation(
				properties, sourcePath, "/terrain.png",
				(icon % 16 + dx) * iconWidth, (icon / 16 + dy) * iconWidth, iconWidth, iconWidth, category);
			if (animation)
				textureAnimations.push_back(std::move(animation));
		}
	};

	if (Block::waterMoving != nullptr)
	{
		registerNamedTerrainAnimation("/custom_water_still.png", Block::waterMoving->blockIndexInTexture, 1, CustomAnimationCategory::Water);
		registerNamedTerrainAnimation("/custom_water_flowing.png", Block::waterMoving->blockIndexInTexture + 1, 4, CustomAnimationCategory::Water);
	}
	if (Block::lavaMoving != nullptr)
	{
		registerNamedTerrainAnimation("/custom_lava_still.png", Block::lavaMoving->blockIndexInTexture, 1, CustomAnimationCategory::Lava);
		registerNamedTerrainAnimation("/custom_lava_flowing.png", Block::lavaMoving->blockIndexInTexture + 1, 4, CustomAnimationCategory::Lava);
	}
	if (Block::portal != nullptr)
		registerNamedTerrainAnimation("/custom_portal.png", Block::portal->blockIndexInTexture, 1, CustomAnimationCategory::Portal);
	if (Block::fire != nullptr)
	{
		registerNamedTerrainAnimation("/custom_fire_n_s.png", Block::fire->blockIndexInTexture, 1, CustomAnimationCategory::Fire);
		registerNamedTerrainAnimation("/custom_fire_e_w.png", Block::fire->blockIndexInTexture + 16, 1, CustomAnimationCategory::Fire);
	}

	registerLegacyAtlasAnimations("/custom_terrain_", "/terrain.png", Config::getIconWidthTerrain(), CustomAnimationCategory::Terrain);
	registerLegacyAtlasAnimations("/custom_item_", "/gui/items.png", Config::getIconWidthItems(), CustomAnimationCategory::Items);
}

void RenderEngine::deleteTexture(int_t i)
{
	textureNameToImageMap.erase(i);
	textureDimensions.erase(i);
	int texture = static_cast<int>(i);
	renderDeleteTextures(1, &texture);
}

void RenderEngine::releaseTexture(const std::string &s)
{
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
	texturefx->onTick();
}

void RenderEngine::updateDynamicTextures()
{
	const int_t mipmapLevel = Config::getMipmapLevel();
	useMipmaps = mipmapLevel > 0;

	auto uploadTile = [this, mipmapLevel](TextureFX *texturefx, int_t baseX, int_t baseY, int_t tileWidth)
	{
		tileWidth = Config::limit(tileWidth, 1, Config::getMaxDynamicTileWidth());
		std::vector<byte_t> scaled((size_t)tileWidth * (size_t)tileWidth * 4u);
		for (int_t y = 0; y < tileWidth; ++y)
		{
			const int_t sourceY = y * 16 / tileWidth;
			for (int_t x = 0; x < tileWidth; ++x)
			{
				const int_t sourceX = x * 16 / tileWidth;
				const size_t src = ((size_t)sourceY * 16u + (size_t)sourceX) * 4u;
				const size_t dst = ((size_t)y * (size_t)tileWidth + (size_t)x) * 4u;
				std::memcpy(&scaled[dst], &texturefx->imageData[src], 4u);
			}
		}

		renderTextureSubImageRgba(0, baseX, baseY, tileWidth, tileWidth, scaled.data());
		if (!useMipmaps)
			return;

		std::vector<byte_t> previous = scaled;
		int_t previousSize = tileWidth;
		for (int_t level = 1; level <= mipmapLevel && previousSize > 1; ++level)
		{
			const int_t size = previousSize / 2;
			if (size <= 0)
				break;
			std::vector<byte_t> current((size_t)size * (size_t)size * 4u);
			for (int_t y = 0; y < size; ++y)
			{
				for (int_t x = 0; x < size; ++x)
				{
					auto packed = [&](int_t sx, int_t sy) -> int_t
					{
						const size_t idx = ((size_t)sy * previousSize + sx) * 4u;
						return JavaArithmetic::intFromBits((static_cast<uint_t>(previous[idx + 3]) << 24) | (static_cast<uint_t>(previous[idx + 0]) << 16) |
						       (static_cast<uint_t>(previous[idx + 1]) << 8) | static_cast<uint_t>(previous[idx + 2]));
					};
					const int_t c0 = packed(x * 2, y * 2);
					const int_t c1 = packed(x * 2 + 1, y * 2);
					const int_t c2 = packed(x * 2 + 1, y * 2 + 1);
					const int_t c3 = packed(x * 2, y * 2 + 1);
					const int_t c = weightedAverageColor(weightedAverageColor(c0, c1), weightedAverageColor(c2, c3));
					const size_t out = ((size_t)y * size + x) * 4u;
					current[out + 0] = (byte_t)((c >> 16) & 0xff);
					current[out + 1] = (byte_t)((c >> 8) & 0xff);
					current[out + 2] = (byte_t)(c & 0xff);
					current[out + 3] = (byte_t)((c >> 24) & 0xff);
				}
			}
			renderTextureSubImageRgba(level, baseX >> level, baseY >> level, size, size, current.data());
			previous.swap(current);
			previousSize = size;
		}
	};

	for (TextureFX *texturefx : textureList)
	{
		if (!texturefx->isAnimationEnabled())
			continue;

		texturefx->anaglyphEnabled = options != nullptr && options->anaglyph;
		texturefx->onTick();
		texturefx->bindImage(this);
		const int_t tileWidth = texturefx->tileImage == 1 ? Config::getIconWidthItems() : Config::getIconWidthTerrain();
		for (int_t tileX = 0; tileX < texturefx->tileSize; ++tileX)
		{
			for (int_t tileY = 0; tileY < texturefx->tileSize; ++tileY)
			{
				const int_t x = (texturefx->iconIndex % 16) * tileWidth + tileX * tileWidth;
				const int_t y = (texturefx->iconIndex / 16) * tileWidth + tileY * tileWidth;
				uploadTile(texturefx, x, y, tileWidth);
			}
		}
	}

	// Java performs a second pass for TextureFX instances targeting their own
	// texture object instead of the terrain/items atlas.
	for (TextureFX *texturefx : textureList)
	{
		if (!texturefx->isAnimationEnabled() || texturefx->textureId <= 0)
			continue;
		renderBindTexture(texturefx->textureId);
		uploadTile(texturefx, 0, 0, 16);
	}

	updateCustomAnimations();
}

void RenderEngine::refreshTextures()
{
	for (auto &entry : urlToImageDataMap)
		if (entry.second != nullptr)
			entry.second->textureSetupComplete = false;

	for (auto &entry : textureNameToImageMap)
		setupTexture(entry.second.get(), entry.first);

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
	RandomMobs::resetTextures();
	loadCustomAnimations();

	for (const auto &entry : textureMap)
		TextureResidencyPolicy::afterNamedTextureUpload(
			entry.second, isDynamicTextureResource(entry.first));
}


std::istream *RenderEngine::getResourceAsStream(const std::string &path) const
{
	TexturePackBase *selected = texturePack != nullptr ? texturePack->getSelectedTexturePack() : nullptr;
	return selected != nullptr ? selected->getResourceAsStream(path) : nullptr;
}

bool RenderEngine::hasResource(const std::string &path) const
{
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
