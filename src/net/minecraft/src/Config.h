#pragma once

#include "java/Type.h"
#include <string>

class GameSettings;
class Minecraft;
class IBlockAccess;

// net.minecraft.src.Config (OptiFine)
// Port without Forge reflection or TextureHD classes. See OPTIFINE_PORT.md.
class Config
{
public:
	// Defaults (Config.DEF_*)
	static constexpr bool DEF_FOG_FANCY = true;
	static constexpr float DEF_FOG_START = 0.2f;
	static constexpr bool DEF_OPTIMIZE_RENDER_DISTANCE = false;
	static constexpr bool DEF_OCCLUSION_ENABLED = false;
	static constexpr float DEF_ALPHA_FUNC_LEVEL = 0.1f;
	static constexpr bool DEF_LOAD_CHUNKS_FAR = false;
	static constexpr int_t DEF_PRELOADED_CHUNKS = 0;
	static constexpr int_t DEF_CHUNKS_LIMIT = 25;
	static constexpr int_t DEF_UPDATES_PER_FRAME = 3;
	static constexpr bool DEF_DYNAMIC_UPDATES = false;

	static void setGameSettings(GameSettings *options);
	static void setMinecraft(Minecraft *mc);
	static Minecraft *getMinecraft();

	// Capacidades del backend grafico (via RenderAPI)
	static bool isFancyFogAvailable();
	static bool isOcclusionAvailable();

	// Niebla
	static bool isFogFancy();
	static bool isFogOff();
	static float getFogStart();

	// Alpha / smooth
	static bool isUseAlphaFunc();
	static float getAlphaFuncLevel();

	// Occlusion culling
	static bool isOcclusionEnabled();
	static bool isOcclusionFancy();

	// Carga de chunks
	static bool isLoadChunksFar();
	static int_t getPreloadedChunks();
	static int_t getUpdatesPerFrame();
	static bool isDynamicUpdates();
	static bool isSmoothFps();
	static bool isSmoothInput();
	static bool isBackgroundChunkLoading();
	static bool isFarView();

	// Texture quality / HD-compatible settings. Backends clamp unsupported features.
	static bool isUseMipmaps();
	static int_t getMipmapLevel();
	static bool isMipmapLinear();
	static int_t getAntialiasingLevel();
	static int_t getAnisotropicFilterLevel();
	static bool isCustomFonts();
	static int_t getMaxRenderDistanceFine();
	static int_t getRenderDistanceFine();
	static int_t getIconWidthTerrain();
	static int_t getIconWidthItems();
	static void setIconWidthTerrain(int_t width);
	static void setIconWidthItems(int_t width);
	static int_t getMaxDynamicTileWidth();

	// Detail
	static bool isRainFancy();
	static bool isRainOff();
	static bool isWaterFancy();
	static bool isCloudsFancy();
	static bool isCloudsOff();
	static bool isTreesFancy();
	static bool isGrassFancy();
	static bool isBetterGrass();
	static bool isBetterGrassFancy();
	static int_t getSideGrassTexture(IBlockAccess *blockAccess, int_t x, int_t y, int_t z, int_t side, int_t tileNum);
	static int_t getSideSnowGrassTexture(IBlockAccess *blockAccess, int_t x, int_t y, int_t z, int_t side);

	// Animaciones (toggles)
	static bool isAnimatedWater();
	static bool isGeneratedWater();
	static bool isAnimatedLava();
	static bool isGeneratedLava();
	static bool isAnimatedFire();
	static bool isAnimatedPortal();
	static bool isAnimatedRedstone();
	static bool isAnimatedExplosion();
	static bool isAnimatedFlame();
	static bool isAnimatedSmoke();
	static bool isVoidParticles();
	static bool isWaterParticles();
	static bool isRainSplash();
	static bool isPortalParticles();
	static bool isDrippingWaterLava();
	static bool isAnimatedTerrain();
	static bool isAnimatedItems();
	static bool isAnimatedTextures();

	// Smooth lighting (AO)
	static float getAmbientOcclusionLevel();
	static float fixAoLight(float light, float defLight);
	static void setLightLevels(float *levels);

	// Cielo / clima / tiempo / agua
	static bool isWeatherEnabled();
	static bool isSkyEnabled();
	static bool isStarsEnabled();
	static bool isSunMoonEnabled();
	static bool isDepthFog();
	static bool isProfilerEnabled();
	static bool isBetterSnow();
	static bool isSwampColors();
	static bool isSmoothBiomes();
	static bool isRandomMobs();
	static bool isCustomColors();
	static bool isConnectedTextures();
	static bool isConnectedTexturesFancy();
	static bool isNaturalTextures();
	static bool isTimeDayOnly();
	static bool isTimeNightOnly();
	static bool isClearWater();

	// Util
	static int_t limit(int_t val, int_t min, int_t max);
	static float limit(float val, float min, float max);
	static int_t intHash(int_t x);
	static int_t getRandom(int_t x, int_t y, int_t z, int_t face);
	static void dbg(const std::string &s);
	static void log(const std::string &s);
	static void sleep(long ms);

private:
	static void checkOpenGlCaps();

	static GameSettings *gameSettings;
	static Minecraft *minecraft;
	static float *lightLevels;
	static int_t iconWidthTerrain;
	static int_t iconWidthItems;
};
