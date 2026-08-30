#include "platform/Log.h"
#include "Config.h"

#include <iostream>
#include "GameSettings.h"
#include "IBlockAccess.h"
#include "platform/PlatformCompat.h"
#include "platform/PlatformTuning.h"
#include "platform/RenderAPI.h"
#include "java/Arithmetic.h"

GameSettings *Config::gameSettings = nullptr;
Minecraft *Config::minecraft = nullptr;
float *Config::lightLevels = nullptr;
int_t Config::iconWidthTerrain = 16;
int_t Config::iconWidthItems = 16;

void Config::checkOpenGlCaps()
{
	log("");
	log(std::string("OpenGL: ") + (const char *)renderGetString(RenderStringQuery::Renderer) +
		" version " + (const char *)renderGetString(RenderStringQuery::Version) + ", " + (const char *)renderGetString(RenderStringQuery::Vendor));
	if (!isFancyFogAvailable())
		log("OpenGL Fancy fog: Not available (GL_NV_fog_distance)");
	if (!isOcclusionAvailable())
		log("OpenGL Occlussion culling: Not available (GL_ARB_occlusion_query)");
}

bool Config::isFancyFogAvailable()
{
	return renderSupportsFeature(RenderFeature::FancyFogDistance);
}

bool Config::isOcclusionAvailable()
{
	return renderSupportsFeature(RenderFeature::OcclusionQuery);
}

void Config::setGameSettings(GameSettings *options)
{
	if (gameSettings == nullptr)
		checkOpenGlCaps();
	gameSettings = options;
}

bool Config::isFogFancy()
{
	if (!isFancyFogAvailable())
		return false;
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofFogFancy;
}

bool Config::isFogOff()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofFogOff;
}

float Config::getFogStart()
{
	if (gameSettings == nullptr)
		return DEF_FOG_START;
	return gameSettings->ofFogStart;
}

bool Config::isUseAlphaFunc()
{
	return getAlphaFuncLevel() > DEF_ALPHA_FUNC_LEVEL + 1.0e-5f;
}

float Config::getAlphaFuncLevel()
{
	return DEF_ALPHA_FUNC_LEVEL;
}

bool Config::isOcclusionEnabled()
{
	if (gameSettings == nullptr)
		return DEF_OCCLUSION_ENABLED;
	return gameSettings->advancedOpengl; // campo 'h'
}

bool Config::isOcclusionFancy()
{
	if (!isOcclusionEnabled())
		return false;
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofOcclusionFancy;
}

bool Config::isLoadChunksFar()
{
	if (gameSettings == nullptr)
		return DEF_LOAD_CHUNKS_FAR;
	return gameSettings->ofLoadFar;
}

int_t Config::getPreloadedChunks()
{
	if (gameSettings == nullptr)
		return DEF_PRELOADED_CHUNKS;
	return gameSettings->ofPreloadedChunks;
}

int_t Config::getUpdatesPerFrame()
{
	if (gameSettings != nullptr)
		return gameSettings->ofChunkUpdates;
	return 1;
}

bool Config::isDynamicUpdates()
{
	if (gameSettings != nullptr)
		return gameSettings->ofChunkUpdatesDynamic;
	return true;
}

bool Config::isSmoothFps()
{
	return gameSettings != nullptr && gameSettings->ofSmoothFps;
}

bool Config::isSmoothInput()
{
	return gameSettings != nullptr && gameSettings->ofSmoothInput;
}

bool Config::isBackgroundChunkLoading()
{
	return true;
}

bool Config::isFarView()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofFarView;
}

bool Config::isUseMipmaps()
{
	return getMipmapLevel() > 0;
}

int_t Config::getMipmapLevel()
{
	if (gameSettings == nullptr || !renderSupportsFeature(RenderFeature::Mipmaps))
		return 0;
	return limit(gameSettings->ofMipmapLevel, 0, 4);
}

bool Config::isMipmapLinear()
{
	return gameSettings != nullptr && gameSettings->ofMipmapLinear;
}

int_t Config::getAntialiasingLevel()
{
	if (gameSettings == nullptr || !renderSupportsFeature(RenderFeature::MultisampleAntialiasing))
		return 0;
	const int_t maxSamples = renderGetMaxSamples();
	return maxSamples > 0 ? limit(gameSettings->ofAaLevel, 0, maxSamples) : 0;
}

int_t Config::getAnisotropicFilterLevel()
{
	if (gameSettings == nullptr || !renderSupportsFeature(RenderFeature::AnisotropicFiltering))
		return 1;
	const int_t maxAnisotropy = renderGetMaxAnisotropy();
	return limit(gameSettings->ofAfLevel, 1, maxAnisotropy > 1 ? maxAnisotropy : 1);
}

bool Config::isCustomFonts()
{
	return gameSettings == nullptr || gameSettings->ofCustomFonts;
}

int_t Config::getMaxRenderDistanceFine()
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
	// PLATFORM_VISIBLE_CHUNK_RADIUS is authoritative on memory-constrained backends.
	return limit(PLATFORM_VISIBLE_CHUNK_RADIUS * 16, 32, 256);
#else
	return 256;
#endif
}

int_t Config::getRenderDistanceFine()
{
	return gameSettings != nullptr
		? limit(gameSettings->ofRenderDistanceFine, 32, getMaxRenderDistanceFine())
		: 128;
}

int_t Config::getIconWidthTerrain() { return iconWidthTerrain; }
int_t Config::getIconWidthItems() { return iconWidthItems; }
void Config::setIconWidthTerrain(int_t width) { iconWidthTerrain = limit(width, 1, getMaxDynamicTileWidth()); }
void Config::setIconWidthItems(int_t width) { iconWidthItems = limit(width, 1, getMaxDynamicTileWidth()); }
int_t Config::getMaxDynamicTileWidth() { return 64; }

bool Config::isRainFancy()
{
	if (gameSettings->ofRain == 0)
		return gameSettings->fancyGraphics; // 'j'
	return gameSettings->ofRain == 2;
}

bool Config::isRainOff()
{
	return gameSettings->ofRain == 3;
}

bool Config::isWaterFancy()
{
	if (gameSettings->ofWater == 0)
		return gameSettings->fancyGraphics;
	return gameSettings->ofWater == 2;
}

bool Config::isCloudsFancy()
{
	if (gameSettings->ofClouds == 0)
		return gameSettings->fancyGraphics;
	return gameSettings->ofClouds == 2;
}

bool Config::isCloudsOff()
{
	return gameSettings->ofClouds == 3;
}

bool Config::isTreesFancy()
{
	if (gameSettings->ofTrees == 0)
		return gameSettings->fancyGraphics;
	return gameSettings->ofTrees == 2;
}

bool Config::isGrassFancy()
{
	if (gameSettings->ofGrass == 0)
		return gameSettings->fancyGraphics;
	return gameSettings->ofGrass == 2;
}

bool Config::isBetterGrass()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofBetterGrass != 3;
}

bool Config::isBetterGrassFancy()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofBetterGrass == 2;
}

int_t Config::getSideGrassTexture(IBlockAccess *blockAccess, int_t x, int_t y, int_t z, int_t side, int_t tileNum)
{
	if (!isBetterGrass())
		return tileNum;

	int_t fullTileNum = 0;
	int_t destBlockId = 2;
	if (tileNum == 77)
	{
		fullTileNum = 78;
		destBlockId = 110;
	}

	if (isBetterGrassFancy())
	{
		y--;
		switch (side)
		{
		case 2: z--; break;
		case 3: z++; break;
		case 4: x--; break;
		case 5: x++; break;
		}
		if (blockAccess->getBlockId(x, y, z) != destBlockId)
			return tileNum;
	}
	return fullTileNum;
}

int_t Config::getSideSnowGrassTexture(IBlockAccess *blockAccess, int_t x, int_t y, int_t z, int_t side)
{
	if (!isBetterGrass())
		return 68;
	if (isBetterGrassFancy())
	{
		switch (side)
		{
		case 2: z--; break;
		case 3: z++; break;
		case 4: x--; break;
		case 5: x++; break;
		}
		int_t blockId = blockAccess->getBlockId(x, y, z);
		if (blockId != 78 && blockId != 80)
			return 68;
	}
	return 66;
}

bool Config::isAnimatedWater()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedWater != 2;
	return true;
}

bool Config::isGeneratedWater()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedWater == 1;
	return true;
}

bool Config::isAnimatedLava()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedLava != 2;
	return true;
}

bool Config::isGeneratedLava()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedLava == 1;
	return true;
}

bool Config::isAnimatedPortal()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedPortal;
	return true;
}

bool Config::isAnimatedFire()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedFire;
	return true;
}

bool Config::isAnimatedRedstone()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedRedstone;
	return true;
}

bool Config::isAnimatedExplosion()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedExplosion;
	return true;
}

bool Config::isAnimatedFlame()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedFlame;
	return true;
}

bool Config::isAnimatedSmoke()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedSmoke;
	return true;
}

bool Config::isVoidParticles()
{
	return gameSettings == nullptr || gameSettings->ofVoidParticles;
}

bool Config::isWaterParticles()
{
	return gameSettings == nullptr || gameSettings->ofWaterParticles;
}

bool Config::isRainSplash()
{
	return gameSettings == nullptr || gameSettings->ofRainSplash;
}

bool Config::isPortalParticles()
{
	return gameSettings == nullptr || gameSettings->ofPortalParticles;
}

bool Config::isDrippingWaterLava()
{
	return gameSettings == nullptr || gameSettings->ofDrippingWaterLava;
}

bool Config::isAnimatedTerrain()
{
	return gameSettings == nullptr || gameSettings->ofAnimatedTerrain;
}

bool Config::isAnimatedItems()
{
	return gameSettings == nullptr || gameSettings->ofAnimatedItems;
}

bool Config::isAnimatedTextures()
{
	return gameSettings == nullptr || gameSettings->ofAnimatedTextures;
}

float Config::getAmbientOcclusionLevel()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAoLevel;
	return 0.0f;
}

float Config::fixAoLight(float light, float defLight)
{
	if (lightLevels == nullptr)
		return light;
	float level_0 = lightLevels[0];
	float level_1 = lightLevels[1];
	if (light > level_0)
		return light;
	if (defLight <= level_1)
		return light;
	float mul = 1.0f - getAmbientOcclusionLevel();
	return light + (defLight - light) * mul;
}

void Config::setLightLevels(float *levels)
{
	lightLevels = levels;
}

bool Config::isWeatherEnabled()
{
	if (gameSettings == nullptr)
		return true;
	return gameSettings->ofWeather;
}

bool Config::isSkyEnabled()
{
	if (gameSettings == nullptr)
		return true;
	return gameSettings->ofSky;
}

bool Config::isStarsEnabled()
{
	if (gameSettings == nullptr)
		return true;
	return gameSettings->ofStars;
}

bool Config::isSunMoonEnabled()
{
	return gameSettings == nullptr || gameSettings->ofSunMoon;
}

bool Config::isDepthFog()
{
	return gameSettings == nullptr || gameSettings->ofDepthFog;
}

bool Config::isProfilerEnabled()
{
	return gameSettings != nullptr && gameSettings->ofProfiler;
}

bool Config::isBetterSnow()
{
	return gameSettings != nullptr && gameSettings->ofBetterSnow;
}

bool Config::isSwampColors()
{
	return gameSettings == nullptr || gameSettings->ofSwampColors;
}

bool Config::isSmoothBiomes()
{
	return gameSettings == nullptr || gameSettings->ofSmoothBiomes;
}

bool Config::isRandomMobs()
{
	return gameSettings == nullptr || gameSettings->ofRandomMobs;
}

bool Config::isCustomColors()
{
	return gameSettings == nullptr || gameSettings->ofCustomColors;
}

bool Config::isConnectedTextures()
{
	return gameSettings != nullptr && gameSettings->ofConnectedTextures != 3;
}

bool Config::isConnectedTexturesFancy()
{
	return gameSettings != nullptr && gameSettings->ofConnectedTextures == 2;
}

bool Config::isNaturalTextures()
{
	return gameSettings != nullptr && gameSettings->ofNaturalTextures;
}

bool Config::isTimeDayOnly()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofTime == 1;
}

bool Config::isTimeNightOnly()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofTime == 2;
}

bool Config::isClearWater()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofClearWater;
}

int_t Config::limit(int_t val, int_t min, int_t max)
{
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

float Config::limit(float val, float min, float max)
{
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

int_t Config::intHash(int_t x)
{
	x = JavaArithmetic::intFromBits(static_cast<uint_t>(x) ^ static_cast<uint_t>(0x3d) ^
		static_cast<uint_t>(JavaArithmetic::intShr(x, 16)));
	x = JavaArithmetic::intAdd(x, JavaArithmetic::intShl(x, 3));
	x = JavaArithmetic::intFromBits(static_cast<uint_t>(x) ^ static_cast<uint_t>(JavaArithmetic::intShr(x, 4)));
	x = JavaArithmetic::intMul(x, 668265261);
	x = JavaArithmetic::intFromBits(static_cast<uint_t>(x) ^ static_cast<uint_t>(JavaArithmetic::intShr(x, 15)));
	return x;
}

int_t Config::getRandom(int_t x, int_t y, int_t z, int_t face)
{
	int_t value = intHash(JavaArithmetic::intAdd(face, 37));
	value = intHash(JavaArithmetic::intAdd(value, x));
	value = intHash(JavaArithmetic::intAdd(value, z));
	value = intHash(JavaArithmetic::intAdd(value, y));
	return value;
}

void Config::setMinecraft(Minecraft *mc) { minecraft = mc; }
Minecraft *Config::getMinecraft() { return minecraft; }

void Config::dbg(const std::string &s) { MC_LOG_DEBUG("config", "%s\n", s.c_str()); }
void Config::log(const std::string &s) { dbg(s); }

void Config::sleep(long ms)
{
#if defined(PS2_PLATFORM)
	(void)ms;
#elif defined(WII_PLATFORM)
	// Real sleep on this console: PlatformCompat::delay yields to libogc's
	// scheduler, so audio and USB keep running while we wait.
	PlatformCompat::delay((uint32_t)ms);
#else
	SDL_Delay((Uint32)ms);
#endif
}
