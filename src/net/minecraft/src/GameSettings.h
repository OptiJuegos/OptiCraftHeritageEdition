#pragma once

#include <string>
#include <vector>
#include "java/Type.h"

class EnumOptions;
class KeyBinding;
class Minecraft;

// net.minecraft.src.GameSettings
class GameSettings
{
public:
	GameSettings(Minecraft *minecraft, const std::string &file);
	GameSettings();
	~GameSettings();

	std::string getKeyBindingDescription(int_t i);
	std::string getOptionDisplayString(int_t i);
	static std::string getKeyDisplayString(int_t keyCode);
	void setKeyBinding(int_t i, int_t j);
	void resetControlBindingsToDefaults();
	void setOptionFloatValue(const EnumOptions *enumoptions, float f);
	void setOptionFloatValue(const EnumOptions &enumoptions, float f) { setOptionFloatValue(&enumoptions, f); }
	void setOptionValue(const EnumOptions *enumoptions, int_t i);
	void setOptionValue(const EnumOptions &enumoptions, int_t i) { setOptionValue(&enumoptions, i); }
	float getOptionFloatValue(const EnumOptions *enumoptions);
	float getOptionFloatValue(const EnumOptions &enumoptions) { return getOptionFloatValue(&enumoptions); }
	bool getOptionOrdinalValue(const EnumOptions *enumoptions);
	bool getOptionOrdinalValue(const EnumOptions &enumoptions) { return getOptionOrdinalValue(&enumoptions); }
	std::string getKeyBinding(const EnumOptions *enumoptions);
	std::string getKeyBinding(const EnumOptions &enumoptions) { return getKeyBinding(&enumoptions); }
	void loadOptions();
	void saveOptions();
	void setLegacyUiEnabled(bool enabled);
	void setAllAnimations(bool flag);
	// Wii only: pushes the per-family (GameCube/Wiimote/Classic) raw button
	// assignments so WiiGameCubePad/WiiRemote read the configured button
	// instead of the hardcoded one. Public so GuiWiiControls can call it right
	// after editing one of the wii*Jump/Sneak/Drop/Inventory fields below,
	// same as GuiDeadzoneSettings calling PlatformUserSettings directly.
	void syncControllerBindingsToPlatform();

private:
	void setDefaults();
	// Rebuild every chunk so options baked into the chunk display lists
	// (smooth lighting / AO, graphics quality, tree/grass detail, view distance)
	// take effect on already-compiled chunks instead of only newly built ones.
	void reloadChunkRenderers();
	void invalidateChunkMeshes();
	// OptiFine: reaplica el brillo (ofBrightness) a la lightBrightnessTable del mundo.
	void updateWorldLightLevels();
	void updateWaterOpacity();
	void refreshTextures();
	float parseFloat(const std::string &s);
	std::string translateKey(const std::string &s);
	static std::string keyName(int_t keyCode);
	// Pushes the current forward/back/left/right/jump/sneak/drop/inventory
	// bindings so PS2/Wii gamepad input mappers synthesize the keys the
	// player actually configured instead of the vanilla defaults.
	void syncKeyBindingsToPlatform();
	int_t legacyGuiScaleRestore;

	static const char *RENDER_DISTANCES[4];
	static const char *DIFFICULTIES[4];
	static const char *GUISCALES[4];
	static const char *PARTICLES[3];
	static const char *LIMIT_FRAMERATES[3];

public:
	float musicVolume;
	float soundVolume;
	float mouseSensitivity;
	bool invertMouse;
	int_t renderDistance;
	bool viewBobbing;
	bool anaglyph;
	bool advancedOpengl;
	int_t limitFramerate;
	bool fancyGraphics;
	bool ambientOcclusion;
	std::string skin;
	KeyBinding *keyBindAttack;
	KeyBinding *keyBindUseItem;
	KeyBinding *keyBindForward;
	KeyBinding *keyBindLeft;
	KeyBinding *keyBindBack;
	KeyBinding *keyBindRight;
	KeyBinding *keyBindJump;
	KeyBinding *keyBindInventory;
	KeyBinding *keyBindDrop;
	KeyBinding *keyBindChat;
	KeyBinding *keyBindPlayerList;
	KeyBinding *keyBindPickBlock;
	KeyBinding *keyBindToggleFog;
	KeyBinding *keyBindSneak;
	std::vector<KeyBinding *> keyBindings;
	Minecraft *mc;
	std::string optionsFile;
	int_t difficulty;
	bool hideGUI;
	int_t thirdPersonView;
	bool showDebugInfo;
	bool showFps;
	bool debugKeepInventory;
	std::string lastServer;
	std::string language;
	// Offline/LAN multiplayer identity. Kept in options.txt so console builds do
	// not need command-line arguments to choose a player name.
	std::string playerName;
	std::string selectedSkin;
	bool legacyUI;
	bool legacyLook;
	int_t renderBackend;
	bool alternativeControllerLayout;
	// Platform-neutral controller settings consumed through PlatformUserSettings.
	float controllerDeadzone;
	// Wii only: raw button assigned to Jump/Sneak/Drop/Inventory/Attack/Use/
	// ThirdPerson, independently per controller family (see WiiButtonBindings.h
	// for why movement isn't here too). Meaningless on other platforms, same
	// as controllerDeadzone.
	int_t wiiGcJump, wiiGcSneak, wiiGcDrop, wiiGcInventory, wiiGcAttack, wiiGcUse, wiiGcThirdPerson;
	int_t wiiWmJump, wiiWmSneak, wiiWmDrop, wiiWmInventory, wiiWmAttack, wiiWmUse, wiiWmThirdPerson;
	int_t wiiCcJump, wiiCcSneak, wiiCcDrop, wiiCcInventory, wiiCcAttack, wiiCcUse, wiiCcThirdPerson;
	// Wii only: vertical deflicker filter on the display copy (see gx_wii.cpp).
	bool wiiDeflicker;
	bool widescreen;
	bool field_22275_C;
	bool smoothCamera;
	bool field_22273_E;
	float field_22272_F;
	int_t guiScale;
	int_t particleSetting;
	float fovSetting;

	// --- OptiFine (mipmaps excluidos: sin ofMipmapLevel/ofMipmapLinear) ---
	bool ofFogFancy;          // Fog: false=Fast, true=Fancy (requiere GL_NV_fog_distance)
	bool ofFogOff;            // Fog: true = niebla de distancia deshabilitada (extension del port)
	float ofFogStart;         // 0.2 .. 0.8
	bool ofLoadFar;           // cargar chunks lejanos
	int_t ofPreloadedChunks;  // 0..8
	bool ofOcclusionFancy;    // occlusion culling fancy (campo 'h' = advancedOpengl = occlusion on/off)
	bool ofSmoothFps;
	bool ofSmoothInput;
	float ofBrightness;       // 0..1
	float ofAoLevel;          // Smooth Lighting 0..1
	int_t ofClouds;           // 0=Default,1=Fast,2=Fancy,3=OFF
	float ofCloudsHeight;     // 0..1
	int_t ofTrees;            // 0=Default,1=Fast,2=Fancy
	int_t ofGrass;            // 0=Default,1=Fast,2=Fancy
	int_t ofRain;             // 0=Default,1=Fast,2=Fancy,3=OFF
	int_t ofWater;            // 0=Default,1=Fast,2=Fancy,3=OFF
	int_t ofBetterGrass;      // 1=Fast,2=Fancy,3=OFF
	int_t ofAutoSaveTicks;    // 40..40000
	bool ofFastDebugInfo;
	bool ofWeather;
	bool ofSky;
	bool ofStars;
	int_t ofChunkUpdates;     // 1..5
	bool ofChunkUpdatesDynamic;
	bool ofFarView;
	int_t ofTime;             // 0=Default,1=Day,2=Night
	bool ofClearWater;
	bool ofSunMoon;
	bool ofDepthFog;
	bool ofProfiler;
	bool ofBetterSnow;
	bool ofSwampColors;
	bool ofSmoothBiomes;
	bool ofRandomMobs;
	bool ofCustomColors;
	int_t ofConnectedTextures; // 1=Fast, 2=Fancy, 3=OFF
	bool ofNaturalTextures;
	int_t ofMipmapLevel;       // 0=OFF, 1..4 mip levels
	bool ofMipmapLinear;       // false=nearest mip selection, true=linear blend
	int_t ofAaLevel;           // 0,2,4,8,16; applied when the desktop GL context is created
	int_t ofAfLevel;           // 1,2,4,8,16, backend-clamped
	bool ofCustomFonts;
	int_t ofRenderDistanceFine;  // 32..512 blocks; consoles stay platform-clamped
	bool ofVoidParticles;
	bool ofWaterParticles;
	bool ofRainSplash;
	bool ofPortalParticles;
	bool ofDrippingWaterLava;
	bool ofAnimatedTerrain;
	bool ofAnimatedItems;
	bool ofAnimatedTextures;
	int_t ofAnimatedWater;    // 0=ON,1=Dynamic,2=OFF
	int_t ofAnimatedLava;     // 0=ON,1=Dynamic,2=OFF
	bool ofAnimatedFire;
	bool ofAnimatedPortal;
	bool ofAnimatedRedstone;
	bool ofAnimatedExplosion;
	bool ofAnimatedFlame;
	bool ofAnimatedSmoke;
	KeyBinding *ofKeyBindZoom;
};
