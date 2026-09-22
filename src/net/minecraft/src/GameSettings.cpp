#include "platform/Log.h"
#include "GameSettings.h"
#include "UiStrings.h"
#include "legacy/LegacyLook.h"
#include "java/String.h"
#include "java/Arithmetic.h"
#include "lwjgl/Keyboard.h"

#include <algorithm>
#include "EnumOptions.h"
#include "EnumOptionsMappingHelper.h"
#include "KeyBinding.h"
#include "StringTranslate.h"
#include "StatCollector.h"
#include "Config.h"
#include "Minecraft.h"
#include "SoundManager.h"
#include "Session.h"
#include "EntityRenderer.h"
#include "RenderGlobal.h"
#include "RenderBlocks.h"
#include "RenderEngine.h"
#include "Block.h"
#include "BlockLeaves.h"
#include "World.h"
#include "IChunkProvider.h"
#include "Chunk.h"
#include "ExtendedBlockStorage.h"
#include "platform/ConsoleAspectRatio.h"
#include "platform/GameDefaults.h"
#include "platform/GameSettingsBackend.h"
#include "platform/PlatformKeyBindings.h"
#include "platform/PlatformTuning.h"
#include "platform/PlatformUserSettings.h"
#include "net/minecraft/src/legacy/LegacyUiPolicy.h"
#include "net/minecraft/src/legacy/LegacyUiScalePolicy.h"
#if PLATFORM_PC
#include "pc/render/PcRenderBackend.h"
#endif


const char *GameSettings::RENDER_DISTANCES[4] = {
	"options.renderDistance.far", "options.renderDistance.normal", "options.renderDistance.short", "options.renderDistance.tiny"
};

const char *GameSettings::DIFFICULTIES[4] = {
	"options.difficulty.peaceful", "options.difficulty.easy", "options.difficulty.normal", "options.difficulty.hard"
};

const char *GameSettings::GUISCALES[4] = {
	"options.guiScale.auto", "options.guiScale.small", "options.guiScale.normal", "options.guiScale.large"
};

const char *GameSettings::PARTICLES[3] = {
	"options.particles.all", "options.particles.decreased", "options.particles.minimal"
};

const char *GameSettings::LIMIT_FRAMERATES[3] = {
	"performance.max", "performance.balanced", "performance.powersaver"
};

GameSettings::GameSettings(Minecraft *minecraft, const std::string &file)
{
	setDefaults();
	mc = minecraft;
	optionsFile = file + "/options.txt";
	loadOptions();
	// A persisted language (options.txt from a prior session, or a hand-
	// edited/PC-authored file) bypasses the language screen's own Latin-only
	// filter entirely, since this runs unconditionally at boot before that
	// screen ever opens. Validate it here too -- cheap (one file, only read
	// when language != en_US), unlike the full list filter the menu uses.
	if (!StringTranslate::isLatin1SafeLanguageOnPs2(language))
		language = "en_US";
	StringTranslate::getInstance()->setLanguage(language);
	Config::setGameSettings(this);
}

GameSettings::GameSettings()
{
	setDefaults();
	guiScale = legacyUiEffectiveGuiScale(legacyUI, legacyGuiScaleRestore);
}

GameSettings::~GameSettings()
{
	Config::setGameSettings(nullptr);
	for (KeyBinding *binding : keyBindings)
		delete binding;
	keyBindings.clear();
	keyBindAttack = keyBindUseItem = nullptr;
	keyBindForward = keyBindLeft = keyBindBack = keyBindRight = nullptr;
	keyBindJump = keyBindInventory = keyBindDrop = keyBindChat = nullptr;
	keyBindPlayerList = keyBindPickBlock = nullptr;
	keyBindToggleFog = keyBindSneak = ofKeyBindZoom = nullptr;
}

void GameSettings::setDefaults()
{
	musicVolume = 1.0f;
	soundVolume = 1.0f;
	mouseSensitivity = 0.5f;
	invertMouse = false;
	renderDistance = 0;
	particleSetting = 0;
	fovSetting = 0.0f;
	viewBobbing = true;
	anaglyph = false;
	advancedOpengl = false;
	limitFramerate = 1;
	fancyGraphics = true;
	ambientOcclusion = true;
	skin = "Default"; // Resource identifier, not a UI label.
	keyBindAttack = new KeyBinding("key.attack", -100);
	keyBindUseItem = new KeyBinding("key.use", -99);
	keyBindForward = new KeyBinding("key.forward", 17);
	keyBindLeft = new KeyBinding("key.left", 30);
	keyBindBack = new KeyBinding("key.back", 31);
	keyBindRight = new KeyBinding("key.right", 32);
	keyBindJump = new KeyBinding("key.jump", 57);
	keyBindInventory = new KeyBinding("key.inventory", 18);
	keyBindDrop = new KeyBinding("key.drop", 16);
	keyBindChat = new KeyBinding("key.chat", 20);
	keyBindPlayerList = new KeyBinding("key.playerlist", 15);
	keyBindPickBlock = new KeyBinding("key.pickItem", -98);
	keyBindToggleFog = new KeyBinding("key.fog", 33);
	keyBindSneak = new KeyBinding("key.sneak", 42);
	platformGameSettingsInitialize(*this);
	keyBindings = {
		keyBindAttack, keyBindUseItem, keyBindForward, keyBindLeft, keyBindBack, keyBindRight,
		keyBindJump, keyBindSneak, keyBindDrop, keyBindInventory, keyBindChat, keyBindPlayerList,
		keyBindPickBlock, keyBindToggleFog
	};
	mc = nullptr;
	optionsFile = "options.txt";
	difficulty = 2;
	hideGUI = false;
	thirdPersonView = 0;
	showDebugInfo = false;
	showFps = false;
	debugKeepInventory = false;
	lastServer = "";
	language = "en_US";
	playerName = "Player";
	selectedSkin = "LegacySteve";
	legacyUI = legacyUiDefaultEnabled();
	legacyLook = legacyLookDefaultEnabled();
	alternativeControllerLayout = false;
	controllerDeadzone = 0.20f;
	wiiDeflicker = true;
	widescreen = ConsoleAspectRatio::getDefaultWidescreen();
	field_22275_C = false;
	smoothCamera = false;
	field_22273_E = false;
	field_22272_F = 1.0f;
	guiScale = 0;
	legacyGuiScaleRestore = 0;

	// --- OptiFine defaults (kv ctor) ---
	ofFogFancy = false;
	ofFogOff = false;
	ofFogStart = 0.8f;
	ofLoadFar = false;
	ofPreloadedChunks = 0;
	ofOcclusionFancy = false;
	ofSmoothFps = false;
	ofSmoothInput = false;
	ofBrightness = 0.0f;
	ofAoLevel = 0.0f;
	ofClouds = 0;
	ofCloudsHeight = 0.0f;
	ofTrees = 0;
	ofGrass = 0;
	ofRain = 0;
	ofWater = 0;
	ofBetterGrass = 3;
	ofAutoSaveTicks = 4000;
	ofFastDebugInfo = false;
	ofWeather = true;
	ofSky = true;
	ofStars = true;
	ofChunkUpdates = 1;
	ofChunkUpdatesDynamic = true;
	ofFarView = false;
	ofTime = 0;
	ofClearWater = false;
	ofSunMoon = true;
	ofDepthFog = true;
	ofProfiler = false;
	ofBetterSnow = false;
	ofSwampColors = true;
	ofSmoothBiomes = true;
	ofRandomMobs = PLATFORM_OPTIFINE_RANDOM_MOBS != 0;
	ofCustomColors = true;
	ofConnectedTextures = platformGameSettingsDefaultConnectedTextures();
	ofNaturalTextures = false;
	ofMipmapLevel = 0;
	ofMipmapLinear = false;
	ofAaLevel = 0;
	ofAfLevel = 1;
	ofCustomFonts = PLATFORM_OPTIFINE_CUSTOM_FONTS != 0;
	ofRenderDistanceFine = 128;
	ofVoidParticles = true;
	ofWaterParticles = true;
	ofRainSplash = true;
	ofPortalParticles = true;
	ofDrippingWaterLava = true;
	ofAnimatedTerrain = true;
	ofAnimatedItems = true;
	ofAnimatedTextures = true;
	ofAnimatedWater = 0;
	ofAnimatedLava = 0;
	ofAnimatedFire = true;
	ofAnimatedPortal = true;
	ofAnimatedRedstone = true;
	ofAnimatedExplosion = true;
	ofAnimatedFlame = true;
	ofAnimatedSmoke = true;
	ofKeyBindZoom = new KeyBinding("Zoom", 46);
	keyBindings.push_back(ofKeyBindZoom);

const PlatformGameDefaults& platformDefaults = platformGameDefaults();
	if (platformDefaults.usePerformanceProfile)
	{
		renderDistance = platformDefaults.renderDistance;
		ofRenderDistanceFine = platformGameSettingsClampFineRenderDistance(
			JavaArithmetic::intShl(32, JavaArithmetic::intSub(3, renderDistance)));
		particleSetting = platformDefaults.particleSetting;
		fancyGraphics = platformDefaults.fancyGraphics;
		ambientOcclusion = platformDefaults.ambientOcclusion;
		advancedOpengl = false;
		limitFramerate = platformDefaults.limitFramerate;
		viewBobbing = platformDefaults.viewBobbing;

		ofFogFancy = false;
		ofFogOff = platformDefaults.fogOff;
		ofBrightness = platformDefaults.brightness;
		ofLoadFar = false;
		ofPreloadedChunks = 0;
		ofOcclusionFancy = false;
		ofSmoothFps = platformDefaults.smoothFps;
		ofSmoothInput = false;
		ofAoLevel = platformDefaults.aoLevel;
		ofClouds = platformDefaults.clouds;
		ofTrees = 1;
		ofGrass = 1;
		ofRain = 1;
		ofWater = 1;
		ofBetterGrass = 3;
		ofAutoSaveTicks = platformDefaults.autoSaveTicks;
		ofWeather = platformDefaults.weather;
		ofSky = platformDefaults.sky;
		ofSunMoon = platformDefaults.sunMoon;
		ofStars = platformDefaults.stars;
		ofChunkUpdates = platformGameSettingsDefaultChunkUpdates();
		ofChunkUpdatesDynamic = platformDefaults.chunkUpdatesDynamic;
		ofMipmapLevel = platformDefaults.mipmapLevel;
		ofFarView = false;
		ofClearWater = false;
		ofAnimatedWater = 2;
		ofAnimatedLava = 2;
		ofAnimatedFire = false;
		ofAnimatedPortal = false;
		ofAnimatedRedstone = false;
		ofAnimatedExplosion = false;
		ofAnimatedFlame = false;
		ofAnimatedSmoke = false;
#if PLATFORM_PC_LEGACY
		ofSmoothBiomes = false;
		ofRandomMobs = false;
		ofCustomColors = false;
		ofVoidParticles = false;
		ofWaterParticles = false;
		ofRainSplash = false;
		ofPortalParticles = false;
		ofDrippingWaterLava = false;
#endif
	}

	KeyBinding::resetKeyBindingArrayAndHash();
	syncKeyBindingsToPlatform();
	syncControllerBindingsToPlatform();
}

void GameSettings::syncKeyBindingsToPlatform()
{
	PlatformKeyBindings::set({
		keyBindForward->keyCode, keyBindBack->keyCode,
		keyBindLeft->keyCode, keyBindRight->keyCode,
		keyBindJump->keyCode, keyBindSneak->keyCode,
		keyBindDrop->keyCode, keyBindInventory->keyCode,
	});
}

void GameSettings::syncControllerBindingsToPlatform()
{
	platformGameSettingsSyncControllerBindings(*this);
}

void GameSettings::reloadChunkRenderers()
{
	if (mc != nullptr && mc->renderGlobal != nullptr)
		mc->renderGlobal->loadRenderers();
}

void GameSettings::invalidateChunkMeshes()
{
	if (mc == nullptr || mc->renderGlobal == nullptr)
		return;
	// RenderGlobal::loadRenderers() normally applies this state before creating
	// replacement meshes. Mesh-only invalidation keeps the renderer grid, so
	// preserve that side effect explicitly before marking existing meshes dirty.
	Block::leaves->setGraphicsLevel(Config::isTreesFancy());
	mc->renderGlobal->markAllRenderersDirty();
}

// OptiFine: GameSettings.updateWorldLightLevels() -> mc.entityRenderer.updateWorldLightLevels()
void GameSettings::updateWorldLightLevels()
{
	if (mc != nullptr && mc->entityRenderer != nullptr)
		mc->entityRenderer->updateWorldLightLevels();
}

void GameSettings::refreshTextures()
{
	if (mc != nullptr)
		mc->refreshResources();
}

void GameSettings::updateWaterOpacity()
{
	const int_t opacity = ofClearWater ? 1 : 3;
	if (Block::waterMoving != nullptr)
	{
		Block::lightOpacity[Block::waterMoving->blockID] = opacity;
		Block::lightOpacityExplicit[Block::waterMoving->blockID] = true;
	}
	if (Block::waterStill != nullptr)
	{
		Block::lightOpacity[Block::waterStill->blockID] = opacity;
		Block::lightOpacityExplicit[Block::waterStill->blockID] = true;
	}

	// OptiFine C6 clears every loaded section's skylight nibbles before asking
	// the chunk to regenerate its skylight map. Rebuilding meshes alone leaves
	// old light values resident and makes Clear Water appear to update only when
	// a chunk is unloaded/reloaded later. Use the provider snapshot instead of
	// C6's 1024x1024 coordinate scan so bounded consoles touch only resident data.
	IChunkProvider *chunkProvider = mc != nullptr && mc->theWorld != nullptr ? mc->theWorld->getIChunkProvider() : nullptr;
	if (chunkProvider != nullptr)
	{
		const std::vector<Chunk *> loadedChunks = chunkProvider->getLoadedChunksSnapshot();
		for (Chunk *chunk : loadedChunks)
		{
			if (chunk == nullptr || chunk->isEmptyChunk())
				continue;
			ExtendedBlockStorage **storage = chunk->getBlockStorageArray();
			for (int_t section = 0; section < Chunk::SECTION_COUNT; ++section)
			{
				ExtendedBlockStorage *ebs = storage[section];
				if (ebs == nullptr)
					continue;
				std::fill(ebs->getSkylightArray().data.begin(), ebs->getSkylightArray().data.end(), static_cast<byte_t>(0));
			}
			chunk->generateSkylightMap();
		}
	}

	invalidateChunkMeshes();
}

void GameSettings::setAllAnimations(bool flag)
{
	const int_t animationMode = flag ? 0 : 2;
	ofAnimatedWater = animationMode;
	ofAnimatedLava = animationMode;
	ofAnimatedFire = flag;
	ofAnimatedPortal = flag;
	ofAnimatedRedstone = flag;
	ofAnimatedExplosion = flag;
	ofAnimatedFlame = flag;
	ofAnimatedSmoke = flag;
	ofVoidParticles = flag;
	ofWaterParticles = flag;
	ofRainSplash = flag;
	ofPortalParticles = flag;
	particleSetting = flag ? 0 : 2;
	ofDrippingWaterLava = flag;
	ofAnimatedTerrain = flag;
	ofAnimatedItems = flag;
	ofAnimatedTextures = flag;
	refreshTextures();
}

std::string GameSettings::getKeyBindingDescription(int_t i)
{
	return translateKey(keyBindings[i]->keyDescription);
}

std::string GameSettings::getOptionDisplayString(int_t i)
{
	return getKeyDisplayString(keyBindings[i]->keyCode);
}

std::string GameSettings::getKeyDisplayString(int_t keyCode)
{
	return keyName(keyCode);
}

void GameSettings::setKeyBinding(int_t i, int_t j)
{
	keyBindings[i]->keyCode = j;
	KeyBinding::resetKeyBindingArrayAndHash();
	syncKeyBindingsToPlatform();
	saveOptions();
}

void GameSettings::resetControlBindingsToDefaults()
{
	keyBindAttack->keyCode = -100;
	keyBindUseItem->keyCode = -99;
	keyBindForward->keyCode = 17;
	keyBindLeft->keyCode = 30;
	keyBindBack->keyCode = 31;
	keyBindRight->keyCode = 32;
	keyBindJump->keyCode = 57;
	keyBindInventory->keyCode = 18;
	keyBindDrop->keyCode = 16;
	keyBindChat->keyCode = 20;
	keyBindPlayerList->keyCode = 15;
	keyBindPickBlock->keyCode = -98;
	keyBindToggleFog->keyCode = 33;
	keyBindSneak->keyCode = 42;
	if (ofKeyBindZoom != nullptr)
		ofKeyBindZoom->keyCode = 46;

	platformGameSettingsResetControlBindings(*this);
	KeyBinding::resetKeyBindingArrayAndHash();
	syncKeyBindingsToPlatform();
	syncControllerBindingsToPlatform();
	saveOptions();
}

void GameSettings::setOptionFloatValue(const EnumOptions *enumoptions, float f)
{
	const bool audioOption = enumoptions == EnumOptions::MUSIC || enumoptions == EnumOptions::SOUND;
	if (enumoptions == EnumOptions::MUSIC)
		musicVolume = f;
	if (enumoptions == EnumOptions::SOUND)
		soundVolume = f;
	if (enumoptions == EnumOptions::SENSITIVITY)
		mouseSensitivity = f;
	if (enumoptions == EnumOptions::FOV)
		fovSetting = f;
	// --- OptiFine ---
	if (enumoptions == EnumOptions::BRIGHTNESS)
	{
		ofBrightness = f;
		updateWorldLightLevels();
	}
	if (enumoptions == EnumOptions::CLOUD_HEIGHT)
		ofCloudsHeight = f;
	if (enumoptions == EnumOptions::AO_LEVEL)
	{
		ofAoLevel = f;
		ambientOcclusion = (ofAoLevel > 0.0f); // 'k'
		invalidateChunkMeshes();
	}
	if (enumoptions == EnumOptions::RENDER_DISTANCE_FINE)
	{
		const int_t maxRenderDistance = Config::getMaxRenderDistanceFine();
		ofRenderDistanceFine = 32 + (int_t)(f * (float)(maxRenderDistance - 32));
		ofRenderDistanceFine = (ofRenderDistanceFine >> 4) << 4;
		ofRenderDistanceFine = Config::limit(ofRenderDistanceFine, 32, maxRenderDistance);
		platformGameSettingsUpdateRenderDistanceFromFine(ofRenderDistanceFine, renderDistance);
		reloadChunkRenderers();
	}
	if (audioOption && mc != nullptr && mc->sndManager != nullptr)
		mc->sndManager->onSoundOptionsChanged();
	saveOptions();
}

void GameSettings::setLegacyUiEnabled(bool enabled)
{
	if (enabled == legacyUI)
	{
		guiScale = legacyUiEffectiveGuiScale(legacyUI, legacyGuiScaleRestore);
		return;
	}

	if (enabled)
		legacyGuiScaleRestore = legacyUiClampGuiScale(guiScale);

	legacyUI = enabled;
	guiScale = legacyUiEffectiveGuiScale(legacyUI, legacyGuiScaleRestore);
}

void GameSettings::setOptionValue(const EnumOptions *enumoptions, int_t i)
{
	if (enumoptions == EnumOptions::INVERT_MOUSE)
		invertMouse = !invertMouse;
	if (enumoptions == EnumOptions::RENDER_DISTANCE)
	{
		renderDistance = platformGameSettingsCycleRenderDistance(renderDistance, i);
		ofRenderDistanceFine = JavaArithmetic::intShl(32, JavaArithmetic::intSub(3, renderDistance));
		ofRenderDistanceFine = platformGameSettingsClampFineRenderDistance(ofRenderDistanceFine);
	}
	if (enumoptions == EnumOptions::GUI_SCALE)
	{
		if (legacyUI)
		{
			guiScale = legacyUiLargeGuiScale();
		}
		else
		{
			guiScale = guiScale + i & 3;
			legacyGuiScaleRestore = legacyUiClampGuiScale(guiScale);
		}
	}
	if (enumoptions == EnumOptions::PARTICLES)
		particleSetting = (particleSetting + i + 3) % 3;
	if (enumoptions == EnumOptions::VIEW_BOBBING)
		viewBobbing = !viewBobbing;
	// ADVANCED_OPENGL = occlusion culling (OFF -> Fast -> Fancy -> OFF), campo 'h'
	if (enumoptions == EnumOptions::ADVANCED_OPENGL)
	{
		if (!Config::isOcclusionAvailable())
		{
			ofOcclusionFancy = false;
			advancedOpengl = false;
		}
		else if (!advancedOpengl)
		{
			advancedOpengl = true;
			ofOcclusionFancy = false;
		}
		else if (!ofOcclusionFancy)
		{
			ofOcclusionFancy = true;
		}
		else
		{
			ofOcclusionFancy = false;
			advancedOpengl = false;
		}
		if (mc != nullptr && mc->renderGlobal != nullptr)
			mc->renderGlobal->setAllRenderersVisible();
	}
	if (enumoptions == EnumOptions::ANAGLYPH)
	{
		anaglyph = platformGameSettingsAnaglyphValue(anaglyph, !anaglyph);
		refreshTextures();
	}
	if (enumoptions == EnumOptions::FRAMERATE_LIMIT)
		limitFramerate = (limitFramerate + i + 3) % 3;
	if (enumoptions == EnumOptions::DIFFICULTY)
		difficulty = difficulty + i & 3;
	if (enumoptions == EnumOptions::GRAPHICS)
	{
		fancyGraphics = !fancyGraphics;
		invalidateChunkMeshes();
	}
	if (enumoptions == EnumOptions::AMBIENT_OCCLUSION)
	{
		ambientOcclusion = !ambientOcclusion;
		invalidateChunkMeshes();
	}
#if PLATFORM_HAS_ASPECT_RATIO_OPTION
	if (enumoptions == EnumOptions::ASPECT_RATIO)
		widescreen = !widescreen;
#endif
	// --- OptiFine ---
	if (enumoptions == EnumOptions::FOG_FANCY)
	{
		// Ciclo tri-estado: Fast -> Fancy -> OFF -> Fast.
		// (Si Fancy no esta disponible: Fast -> OFF -> Fast.)
		if (ofFogOff)
		{
			// OFF -> Fast
			ofFogOff = false;
			ofFogFancy = false;
		}
		else if (ofFogFancy)
		{
			// Fancy -> OFF
			ofFogOff = true;
			ofFogFancy = false;
		}
		else if (Config::isFancyFogAvailable())
		{
			// Fast -> Fancy
			ofFogFancy = true;
		}
		else
		{
			// Fast -> OFF (Fancy no disponible)
			ofFogOff = true;
		}
	}
	if (enumoptions == EnumOptions::FOG_START)
	{
		ofFogStart += 0.2f;
		if (ofFogStart > 0.81f)
			ofFogStart = 0.2f;
	}
	if (enumoptions == EnumOptions::LOAD_FAR)
	{
		ofLoadFar = !ofLoadFar;
		reloadChunkRenderers();
	}
	if (enumoptions == EnumOptions::PRELOADED_CHUNKS)
	{
		ofPreloadedChunks += 2;
		if (ofPreloadedChunks > 8)
			ofPreloadedChunks = 0;
		reloadChunkRenderers();
	}
#if PLATFORM_PC_LEGACY && defined(MC_WIN32)
	if (enumoptions == EnumOptions::RENDER_BACKEND)
	{
		renderBackend = renderBackend == static_cast<int_t>(PcRenderBackendType::Direct3D9)
			? static_cast<int_t>(PcRenderBackendType::OpenGL)
			: static_cast<int_t>(PcRenderBackendType::Direct3D9);
		pcRenderBackendSetRequested(renderBackend == static_cast<int_t>(PcRenderBackendType::Direct3D9)
			? PcRenderBackendType::Direct3D9
			: PcRenderBackendType::OpenGL);
	}
#endif
	if (enumoptions == EnumOptions::SMOOTH_FPS)
		ofSmoothFps = !ofSmoothFps;
	if (enumoptions == EnumOptions::SMOOTH_INPUT)
		ofSmoothInput = !ofSmoothInput;
	if (enumoptions == EnumOptions::CLOUDS)
	{
		ofClouds++;
		if (ofClouds > 3)
			ofClouds = 0;
	}
	if (enumoptions == EnumOptions::TREES)
	{
		ofTrees++;
		if (ofTrees > 2)
			ofTrees = 0;
		invalidateChunkMeshes();
	}
	if (enumoptions == EnumOptions::GRASS)
	{
		ofGrass++;
		if (ofGrass > 2)
			ofGrass = 0;
		RenderBlocks::fancyGrass = Config::isGrassFancy();
		invalidateChunkMeshes();
	}
	if (enumoptions == EnumOptions::RAIN)
	{
		ofRain++;
		if (ofRain > 3)
			ofRain = 0;
	}
	if (enumoptions == EnumOptions::WATER)
	{
		ofWater++;
		if (ofWater > 2)
			ofWater = 0;
	}
	if (enumoptions == EnumOptions::ANIMATED_WATER)
	{
		ofAnimatedWater++;
		if (ofAnimatedWater > 2)
			ofAnimatedWater = 0;
		refreshTextures();
	}
	if (enumoptions == EnumOptions::ANIMATED_LAVA)
	{
		ofAnimatedLava++;
		if (ofAnimatedLava > 2)
			ofAnimatedLava = 0;
		refreshTextures();
	}
	if (enumoptions == EnumOptions::ANIMATED_FIRE)
	{
		ofAnimatedFire = !ofAnimatedFire;
		refreshTextures();
	}
	if (enumoptions == EnumOptions::ANIMATED_PORTAL)
	{
		ofAnimatedPortal = !ofAnimatedPortal;
		refreshTextures();
	}
	if (enumoptions == EnumOptions::ANIMATED_REDSTONE)
		ofAnimatedRedstone = !ofAnimatedRedstone;
	if (enumoptions == EnumOptions::ANIMATED_EXPLOSION)
		ofAnimatedExplosion = !ofAnimatedExplosion;
	if (enumoptions == EnumOptions::ANIMATED_FLAME)
		ofAnimatedFlame = !ofAnimatedFlame;
	if (enumoptions == EnumOptions::ANIMATED_SMOKE)
		ofAnimatedSmoke = !ofAnimatedSmoke;
	if (enumoptions == EnumOptions::FAST_DEBUG_INFO)
		ofFastDebugInfo = !ofFastDebugInfo;
	if (enumoptions == EnumOptions::AUTOSAVE_TICKS)
	{
		ofAutoSaveTicks *= 10;
		if (ofAutoSaveTicks > 40000)
			ofAutoSaveTicks = 40;
	}
	if (enumoptions == EnumOptions::BETTER_GRASS)
	{
		ofBetterGrass++;
		if (ofBetterGrass > 3)
			ofBetterGrass = 1;
		invalidateChunkMeshes();
	}
	if (enumoptions == EnumOptions::WEATHER)
		ofWeather = !ofWeather;
	if (enumoptions == EnumOptions::SKY)
		ofSky = !ofSky;
	if (enumoptions == EnumOptions::STARS)
		ofStars = !ofStars;
	if (enumoptions == EnumOptions::CHUNK_UPDATES)
	{
		ofChunkUpdates++;
		if (ofChunkUpdates > 5)
			ofChunkUpdates = 1;
	}
	if (enumoptions == EnumOptions::CHUNK_UPDATES_DYNAMIC)
		ofChunkUpdatesDynamic = !ofChunkUpdatesDynamic;
	if (enumoptions == EnumOptions::FAR_VIEW)
	{
		ofFarView = !ofFarView;
		reloadChunkRenderers();
	}
	if (enumoptions == EnumOptions::TIME)
	{
		ofTime++;
		if (ofTime > 2)
			ofTime = 0;
	}
	if (enumoptions == EnumOptions::CLEAR_WATER)
	{
		ofClearWater = !ofClearWater;
		updateWaterOpacity();
	}
	if (enumoptions == EnumOptions::SUN_MOON)
		ofSunMoon = !ofSunMoon;
	if (enumoptions == EnumOptions::DEPTH_FOG)
		ofDepthFog = !ofDepthFog;
	if (enumoptions == EnumOptions::PROFILER)
		ofProfiler = !ofProfiler;
	if (enumoptions == EnumOptions::BETTER_SNOW)
	{
		ofBetterSnow = !ofBetterSnow;
		invalidateChunkMeshes();
	}
	if (enumoptions == EnumOptions::SWAMP_COLORS)
	{
		ofSwampColors = !ofSwampColors;
		invalidateChunkMeshes();
	}
	if (enumoptions == EnumOptions::SMOOTH_BIOMES)
	{
		ofSmoothBiomes = !ofSmoothBiomes;
		invalidateChunkMeshes();
	}
	if (enumoptions == EnumOptions::RANDOM_MOBS)
	{
		ofRandomMobs = !ofRandomMobs;
		refreshTextures();
	}
	if (enumoptions == EnumOptions::CUSTOM_COLORS)
	{
		ofCustomColors = !ofCustomColors;
		refreshTextures();
		invalidateChunkMeshes();
	}
	if (enumoptions == EnumOptions::CONNECTED_TEXTURES)
	{
		ofConnectedTextures++;
		if (ofConnectedTextures > 3)
			ofConnectedTextures = 1;
		invalidateChunkMeshes();
	}
	if (enumoptions == EnumOptions::NATURAL_TEXTURES)
	{
		ofNaturalTextures = !ofNaturalTextures;
		refreshTextures();
		invalidateChunkMeshes();
	}
	if (enumoptions == EnumOptions::MIPMAP_LEVEL)
	{
		ofMipmapLevel = (ofMipmapLevel + 1) % 5;
		refreshTextures();
	}
	if (enumoptions == EnumOptions::MIPMAP_TYPE)
	{
		ofMipmapLinear = !ofMipmapLinear;
		refreshTextures();
	}
	if (enumoptions == EnumOptions::CUSTOM_FONTS)
	{
		ofCustomFonts = !ofCustomFonts;
		refreshTextures();
	}
	if (enumoptions == EnumOptions::AA_LEVEL)
	{
		static const int_t levels[] = {0, 2, 4, 8, 16};
		int_t index = 0;
		while (index < 4 && levels[index] != ofAaLevel) ++index;
		ofAaLevel = levels[(index + 1) % 5];
	}
	if (enumoptions == EnumOptions::AF_LEVEL)
	{
		ofAfLevel *= 2;
		if (ofAfLevel > 16) ofAfLevel = 1;
		refreshTextures();
	}
	if (enumoptions == EnumOptions::VOID_PARTICLES)
		ofVoidParticles = !ofVoidParticles;
	if (enumoptions == EnumOptions::WATER_PARTICLES)
		ofWaterParticles = !ofWaterParticles;
	if (enumoptions == EnumOptions::RAIN_SPLASH)
		ofRainSplash = !ofRainSplash;
	if (enumoptions == EnumOptions::PORTAL_PARTICLES)
		ofPortalParticles = !ofPortalParticles;
	if (enumoptions == EnumOptions::DRIPPING_WATER_LAVA)
		ofDrippingWaterLava = !ofDrippingWaterLava;
	if (enumoptions == EnumOptions::ANIMATED_TERRAIN)
	{
		ofAnimatedTerrain = !ofAnimatedTerrain;
		refreshTextures();
	}
	if (enumoptions == EnumOptions::ANIMATED_ITEMS)
	{
		ofAnimatedItems = !ofAnimatedItems;
		refreshTextures();
	}
	if (enumoptions == EnumOptions::ANIMATED_TEXTURES)
	{
		ofAnimatedTextures = !ofAnimatedTextures;
	}
	saveOptions();
}

float GameSettings::getOptionFloatValue(const EnumOptions *enumoptions)
{
	if (enumoptions == EnumOptions::MUSIC)
		return musicVolume;
	if (enumoptions == EnumOptions::SOUND)
		return soundVolume;
	if (enumoptions == EnumOptions::SENSITIVITY)
		return mouseSensitivity;
	if (enumoptions == EnumOptions::FOV)
		return fovSetting;
	if (enumoptions == EnumOptions::BRIGHTNESS)
		return ofBrightness;
	if (enumoptions == EnumOptions::CLOUD_HEIGHT)
		return ofCloudsHeight;
	if (enumoptions == EnumOptions::AO_LEVEL)
		return ofAoLevel;
	if (enumoptions == EnumOptions::RENDER_DISTANCE_FINE)
		return (float)(ofRenderDistanceFine - 32) /
		       (float)(Config::getMaxRenderDistanceFine() - 32);
	return 0.0f;
}

bool GameSettings::getOptionOrdinalValue(const EnumOptions *enumoptions)
{
	switch (EnumOptionsMappingHelper::enumOptionsMappingHelperArray[enumoptions->returnEnumOrdinal()])
	{
	case 1:
		return invertMouse;
	case 2:
		return viewBobbing;
	case 3:
		return anaglyph;
	case 4:
		return advancedOpengl;
	case 5:
		return ambientOcclusion;
	}
	return false;
}

std::string GameSettings::getKeyBinding(const EnumOptions *enumoptions)
{
	std::string s = enumoptions == EnumOptions::ASPECT_RATIO
		? uiText("Aspect Ratio") + ": "
		: uiText(translateKey(enumoptions->getEnumString())) + ": ";
	if (enumoptions->getEnumFloat())
	{
		float f = getOptionFloatValue(enumoptions);
		if (enumoptions == EnumOptions::SENSITIVITY)
		{
			if (f == 0.0f)
				return s + translateKey("options.sensitivity.min");
			if (f == 1.0f)
				return s + translateKey("options.sensitivity.max");
			return s + std::to_string((int_t)(f * 200.0f)) + "%";
		}
		if (enumoptions == EnumOptions::FOV)
		{
			if (f == 0.0f)
				return s + translateKey("options.fov.min");
			if (f == 1.0f)
				return s + translateKey("options.fov.max");
			return s + std::to_string((int_t)(70.0f + f * 40.0f));
		}
		if (enumoptions == EnumOptions::RENDER_DISTANCE_FINE)
		{
			std::string label = uiText("Tiny");
			int_t baseDistance = 32;
			if (ofRenderDistanceFine >= 64) { label = uiText("Short"); baseDistance = 64; }
			if (ofRenderDistanceFine >= 128) { label = uiText("Normal"); baseDistance = 128; }
			if (ofRenderDistanceFine >= 256) { label = uiText("Far"); baseDistance = 256; }
			if (ofRenderDistanceFine >= 512) { label = uiText("Extreme"); baseDistance = 512; }
			const int_t difference = ofRenderDistanceFine - baseDistance;
			return difference == 0 ? s + label : s + label + " +" + std::to_string(difference);
		}
		if (f == 0.0f)
			return s + translateKey("options.off");
		return s + std::to_string((int_t)(f * 100.0f)) + "%";
	}
	// ADVANCED_OPENGL = occlusion (OFF / Fast / Fancy), antes del branch booleano
	if (enumoptions == EnumOptions::ADVANCED_OPENGL)
	{
		if (!advancedOpengl)
			return s + uiText("OFF");
		if (ofOcclusionFancy)
			return s + uiText("Fancy");
		return s + uiText("Fast");
	}
	if (enumoptions->getEnumBoolean())
	{
		bool flag = getOptionOrdinalValue(enumoptions);
		if (flag)
			return s + translateKey("options.on");
		return s + translateKey("options.off");
	}
	if (enumoptions == EnumOptions::RENDER_DISTANCE)
		return s + translateKey(RENDER_DISTANCES[renderDistance]);
	if (enumoptions == EnumOptions::DIFFICULTY)
		return s + translateKey(DIFFICULTIES[difficulty]);
	if (enumoptions == EnumOptions::GUI_SCALE)
		return s + translateKey(GUISCALES[guiScale]);
	if (enumoptions == EnumOptions::PARTICLES)
		return s + translateKey(PARTICLES[particleSetting]);
	if (enumoptions == EnumOptions::FRAMERATE_LIMIT)
		return s + translateKey(LIMIT_FRAMERATES[limitFramerate]);
	if (enumoptions == EnumOptions::GRAPHICS)
	{
		if (fancyGraphics)
			return s + translateKey("options.graphics.fancy");
		return s + translateKey("options.graphics.fast");
	}
	if (enumoptions == EnumOptions::ASPECT_RATIO)
		return s + (widescreen ? "16:9" : "4:3");
	// --- OptiFine ---
	if (enumoptions == EnumOptions::FOG_FANCY)
		return s + (ofFogOff ? uiText("OFF") : (ofFogFancy ? uiText("Fancy") : uiText("Fast")));
	if (enumoptions == EnumOptions::FOG_START)
		return s + std::to_string(ofFogStart);
	if (enumoptions == EnumOptions::LOAD_FAR)
		return s + (ofLoadFar ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::PRELOADED_CHUNKS)
		return s + (ofPreloadedChunks == 0 ? std::string(uiText("OFF")) : std::to_string(ofPreloadedChunks));
#if PLATFORM_PC_LEGACY && defined(MC_WIN32)
	if (enumoptions == EnumOptions::RENDER_BACKEND)
	{
		const PcRenderBackendType backend = renderBackend == static_cast<int_t>(PcRenderBackendType::Direct3D9)
			? PcRenderBackendType::Direct3D9
			: PcRenderBackendType::OpenGL;
		std::string label = s + pcRenderBackendDisplayName(backend);
		if (backend != pcRenderBackendGetActive())
			label += " (restart)";
		return label;
	}
#endif
	if (enumoptions == EnumOptions::SMOOTH_FPS)
		return s + (ofSmoothFps ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::SMOOTH_INPUT)
		return s + (ofSmoothInput ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::CLOUDS)
	{
		switch (ofClouds)
		{
		case 1: return s + uiText("Fast");
		case 2: return s + uiText("Fancy");
		case 3: return s + uiText("OFF");
		}
		return s + uiText("Default");
	}
	if (enumoptions == EnumOptions::TREES)
	{
		switch (ofTrees)
		{
		case 1: return s + uiText("Fast");
		case 2: return s + uiText("Fancy");
		}
		return s + uiText("Default");
	}
	if (enumoptions == EnumOptions::GRASS)
	{
		switch (ofGrass)
		{
		case 1: return s + uiText("Fast");
		case 2: return s + uiText("Fancy");
		}
		return s + uiText("Default");
	}
	if (enumoptions == EnumOptions::RAIN)
	{
		switch (ofRain)
		{
		case 1: return s + uiText("Fast");
		case 2: return s + uiText("Fancy");
		case 3: return s + uiText("OFF");
		}
		return s + uiText("Default");
	}
	if (enumoptions == EnumOptions::WATER)
	{
		switch (ofWater)
		{
		case 1: return s + uiText("Fast");
		case 2: return s + uiText("Fancy");
		case 3: return s + uiText("OFF");
		}
		return s + uiText("Default");
	}
	if (enumoptions == EnumOptions::ANIMATED_WATER)
	{
		switch (ofAnimatedWater)
		{
		case 1: return s + "Dynamic";
		case 2: return s + uiText("OFF");
		}
		return s + uiText("ON");
	}
	if (enumoptions == EnumOptions::ANIMATED_LAVA)
	{
		switch (ofAnimatedLava)
		{
		case 1: return s + "Dynamic";
		case 2: return s + uiText("OFF");
		}
		return s + uiText("ON");
	}
	if (enumoptions == EnumOptions::ANIMATED_FIRE)
		return s + (ofAnimatedFire ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::ANIMATED_PORTAL)
		return s + (ofAnimatedPortal ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::ANIMATED_REDSTONE)
		return s + (ofAnimatedRedstone ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::ANIMATED_EXPLOSION)
		return s + (ofAnimatedExplosion ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::ANIMATED_FLAME)
		return s + (ofAnimatedFlame ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::ANIMATED_SMOKE)
		return s + (ofAnimatedSmoke ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::FAST_DEBUG_INFO)
		return s + (ofFastDebugInfo ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::AUTOSAVE_TICKS)
	{
		if (ofAutoSaveTicks <= 40)
			return s + "Default (2s)";
		if (ofAutoSaveTicks <= 400)
			return s + "20s";
		if (ofAutoSaveTicks <= 4000)
			return s + "3min";
		return s + "30min";
	}
	if (enumoptions == EnumOptions::BETTER_GRASS)
	{
		switch (ofBetterGrass)
		{
		case 1: return s + uiText("Fast");
		case 2: return s + uiText("Fancy");
		}
		return s + uiText("OFF");
	}
	if (enumoptions == EnumOptions::WEATHER)
		return s + (ofWeather ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::SKY)
		return s + (ofSky ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::STARS)
		return s + (ofStars ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::CHUNK_UPDATES)
		return s + std::to_string(ofChunkUpdates);
	if (enumoptions == EnumOptions::CHUNK_UPDATES_DYNAMIC)
		return s + (ofChunkUpdatesDynamic ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::FAR_VIEW)
		return s + (ofFarView ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::TIME)
	{
		if (ofTime == 1)
			return s + uiText("Day Only");
		if (ofTime == 2)
			return s + uiText("Night Only");
		return s + uiText("Default");
	}
	if (enumoptions == EnumOptions::CLEAR_WATER)
		return s + (ofClearWater ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::SUN_MOON)
		return s + (ofSunMoon ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::DEPTH_FOG)
		return s + (ofDepthFog ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::PROFILER)
		return s + (ofProfiler ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::BETTER_SNOW)
		return s + (ofBetterSnow ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::SWAMP_COLORS)
		return s + (ofSwampColors ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::SMOOTH_BIOMES)
		return s + (ofSmoothBiomes ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::RANDOM_MOBS)
		return s + (ofRandomMobs ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::CUSTOM_COLORS)
		return s + (ofCustomColors ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::CONNECTED_TEXTURES)
	{
		if (ofConnectedTextures == 1) return s + uiText("Fast");
		if (ofConnectedTextures == 2) return s + uiText("Fancy");
		return s + uiText("OFF");
	}
	if (enumoptions == EnumOptions::NATURAL_TEXTURES)
		return s + (ofNaturalTextures ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::MIPMAP_LEVEL)
	{
		if (ofMipmapLevel == 0) return s + uiText("OFF");
		if (ofMipmapLevel == 4) return s + "Max";
		return s + std::to_string(ofMipmapLevel);
	}
	if (enumoptions == EnumOptions::MIPMAP_TYPE)
		return s + (ofMipmapLinear ? "Linear" : "Nearest");
	if (enumoptions == EnumOptions::CUSTOM_FONTS)
		return s + (ofCustomFonts ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::AA_LEVEL)
		return s + (ofAaLevel == 0 ? std::string(uiText("OFF")) : std::to_string(ofAaLevel) + "x (restart)");
	if (enumoptions == EnumOptions::AF_LEVEL)
		return s + (ofAfLevel <= 1 ? std::string(uiText("OFF")) : std::to_string(ofAfLevel) + "x");
	if (enumoptions == EnumOptions::VOID_PARTICLES)
		return s + (ofVoidParticles ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::WATER_PARTICLES)
		return s + (ofWaterParticles ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::RAIN_SPLASH)
		return s + (ofRainSplash ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::PORTAL_PARTICLES)
		return s + (ofPortalParticles ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::DRIPPING_WATER_LAVA)
		return s + (ofDrippingWaterLava ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::ANIMATED_TERRAIN)
		return s + (ofAnimatedTerrain ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::ANIMATED_ITEMS)
		return s + (ofAnimatedItems ? uiText("ON") : uiText("OFF"));
	if (enumoptions == EnumOptions::ANIMATED_TEXTURES)
		return s + (ofAnimatedTextures ? uiText("ON") : uiText("OFF"));
	return s;
}

std::string GameSettings::translateKey(const std::string &s)
{
	return StringTranslate::getInstance()->translateKey(s);
}


std::string GameSettings::keyName(int_t keyCode)
{
	if (keyCode < 0)
		return StatCollector::translateToLocalFormatted("key.mouseButton", std::to_string(keyCode + 101));
	return lwjgl::Keyboard::getKeyName(keyCode);
}
