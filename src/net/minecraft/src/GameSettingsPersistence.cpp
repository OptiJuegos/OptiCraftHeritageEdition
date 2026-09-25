#include "platform/Log.h"
#include "GameSettings.h"
#include "Minecraft.h"
#include "Session.h"
#include "java/String.h"
#include "java/Arithmetic.h"
#ifdef PS2_PLATFORM
#include "util/CompactNumberFormat.h"
#endif

#ifndef PS2_PLATFORM
#include <locale>
#include <sstream>
#endif
#include <stdexcept>
#include <unordered_set>

#include "Config.h"
#include "KeyBinding.h"
#include "platform/GameSettingsBackend.h"
#include "platform/PlatformKeyBindings.h"
#include "platform/PlatformTuning.h"
#include "platform/PlatformUserSettings.h"
#include "platform/Storage.h"
#include "net/minecraft/src/legacy/LegacyUiScalePolicy.h"
#include "skin/SkinManager.h"

#ifdef PS2_PLATFORM
namespace
{
bool nextOptionLine(const std::string& text, std::size_t& position, std::string& line)
{
	if (position >= text.size())
		return false;

	const std::size_t end = text.find('\n', position);
	if (end == std::string::npos)
	{
		line = text.substr(position);
		position = text.size();
	}
	else
	{
		line = text.substr(position, end - position);
		position = end + 1;
	}
	return true;
}

class Ps2OptionWriter
{
public:
	Ps2OptionWriter& operator<<(const char* value)
	{
		if (value != nullptr)
			text += value;
		return *this;
	}

	Ps2OptionWriter& operator<<(const std::string& value)
	{
		text += value;
		return *this;
	}

	Ps2OptionWriter& operator<<(int value)
	{
		text += CompactNumberFormat::integer(value);
		return *this;
	}

	Ps2OptionWriter& operator<<(unsigned int value)
	{
		text += CompactNumberFormat::integer(value);
		return *this;
	}

	Ps2OptionWriter& operator<<(float value)
	{
		text += CompactNumberFormat::javaFloat(value);
		return *this;
	}

	Ps2OptionWriter& operator<<(double value)
	{
		text += CompactNumberFormat::javaDouble(value);
		return *this;
	}

	const std::string& str() const
	{
		return text;
	}

private:
	std::string text;
};
}
#endif

static int_t parseIntJava(const std::string &value)
{
	int_t parsed = 0;
	if (!String::tryParseInt(value, parsed))
		throw std::invalid_argument("Invalid Java integer");
	return parsed;
}

void GameSettings::loadOptions()
{
	bool loadedLegacyGuiScaleRestore = false;
	std::vector<unsigned char> optionBytes;
	if (PlatformStorage::readFile(optionsFile, optionBytes))
	{
		const std::string optionText(optionBytes.begin(), optionBytes.end());
		std::string s;
#ifdef PS2_PLATFORM
		std::size_t optionPosition = 0;
		while (nextOptionLine(optionText, optionPosition, s))
#else
		std::istringstream bufferedreader(optionText);
		while (std::getline(bufferedreader, s))
#endif
		{
			try
			{
				const std::size_t separator = s.find(':');
				if (separator == std::string::npos)
					continue;
				const std::string key = s.substr(0, separator);
				std::string value = s.substr(separator + 1);
				if (!value.empty() && value.back() == '\r')
					value.pop_back();
				if (key == "music")
					musicVolume = parseFloat(value);
				if (key == "sound")
					soundVolume = parseFloat(value);
				if (key == "mouseSensitivity")
					mouseSensitivity = parseFloat(value);
				if (key == "fov")
					fovSetting = parseFloat(value);
				if (key == "invertYMouse")
					invertMouse = value == "true";
				if (key == "viewDistance")
				{
					renderDistance = platformGameSettingsClampRenderDistance(parseIntJava(value));
					ofRenderDistanceFine = JavaArithmetic::intShl(32, JavaArithmetic::intSub(3, renderDistance));
				}
				if (key == "guiScale")
					guiScale = parseIntJava(value);
				if (key == "particles")
					particleSetting = parseIntJava(value);
				if (key == "bobView")
					viewBobbing = value == "true";
				if (key == "anaglyph3d")
					anaglyph = platformGameSettingsAnaglyphValue(anaglyph, value == "true");
				if (key == "advancedOpengl")
					advancedOpengl = value == "true";
				if (key == "fpsLimit")
					limitFramerate = parseIntJava(value);
				if (key == "difficulty")
					difficulty = parseIntJava(value);
				if (key == "fancyGraphics")
					fancyGraphics = value == "true";
				if (key == "ao")
					ambientOcclusion = value == "true";
				if (key == "skin")
					skin = value;
				if (key == "lastServer")
					lastServer = value;
				if (key == "lang" && !value.empty())
					language = value;
				if (key == "playerName" && !value.empty())
					playerName = value;
				if (key == "selectedSkin" && !value.empty())
				{
					selectedSkin = value;
					SkinManager::setSelectedSkinId(value);
				}
				if (key == "selectedSkinP2" && !value.empty())
				{
					selectedSkinP2 = value;
					SkinManager::setSelectedSkinIdP2(value);
				}
				if (key == "legacyUI")
					legacyUI = value == "true";
				if (key == "legacyLook")
					legacyLook = value == "true";
				if (key == "legacyGuiScaleRestore")
				{
					legacyGuiScaleRestore = parseIntJava(value);
					loadedLegacyGuiScaleRestore = true;
				}
				if (key == "alternativeControllerLayout" || key == "wiiAlternativeControls")
					alternativeControllerLayout = value == "true";
				if (key == "controllerDeadzone" || key == "wiiStickDeadzone")
					controllerDeadzone = Config::limit(parseFloat(value), 0.05f, 0.35f);
				platformGameSettingsLoadOption(*this, key, value);
#if PLATFORM_HAS_ASPECT_RATIO_OPTION
				if (key == "widescreen" || key == "aspectRatio" ||
					key == "options.aspectRatio" || key == "options.aspectratio")
					widescreen = value == "true";
#endif
				for (int_t i = 0; i < (int_t)keyBindings.size(); i++)
				{
					if (key == "key_" + keyBindings[i]->keyDescription)
						keyBindings[i]->keyCode = parseIntJava(value);
				}
				// --- OptiFine (mipmaps excluded) ---
				if (key == "ofFogFancy")
					ofFogFancy = value == "true";
				if (key == "ofFogOff")
					ofFogOff = value == "true";
				if (key == "ofFogStart")
				{
					ofFogStart = parseFloat(value);
					if (ofFogStart < 0.2f)
						ofFogStart = 0.2f;
					if (ofFogStart > 0.81f)
						ofFogStart = 0.8f;
				}
				if (key == "ofLoadFar")
					ofLoadFar = value == "true";
				if (key == "ofPreloadedChunks")
					ofPreloadedChunks = Config::limit((int_t)parseIntJava(value), 0, 8);
				if (key == "ofOcclusionFancy")
					ofOcclusionFancy = value == "true";
				if (key == "ofSmoothFps")
					ofSmoothFps = value == "true";
				if (key == "ofSmoothInput")
					ofSmoothInput = value == "true";
				if (key == "ofBrightness")
					ofBrightness = Config::limit(parseFloat(value), 0.0f, 1.0f);
				if (key == "ofAoLevel")
				{
					ofAoLevel = Config::limit(parseFloat(value), 0.0f, 1.0f);
					ambientOcclusion = (ofAoLevel > 0.0f);
				}
				if (key == "ofClouds")
					ofClouds = Config::limit((int_t)parseIntJava(value), 0, 3);
				if (key == "ofCloudsHeight")
					ofCloudsHeight = Config::limit(parseFloat(value), 0.0f, 1.0f);
				if (key == "ofTrees")
					ofTrees = Config::limit((int_t)parseIntJava(value), 0, 2);
				if (key == "ofGrass")
					ofGrass = Config::limit((int_t)parseIntJava(value), 0, 2);
				if (key == "ofRain")
					ofRain = Config::limit((int_t)parseIntJava(value), 0, 3);
				if (key == "ofWater")
					ofWater = Config::limit((int_t)parseIntJava(value), 0, 3);
				if (key == "ofAnimatedWater")
					ofAnimatedWater = Config::limit((int_t)parseIntJava(value), 0, 2);
				if (key == "ofAnimatedLava")
					ofAnimatedLava = Config::limit((int_t)parseIntJava(value), 0, 2);
				if (key == "ofAnimatedFire")
					ofAnimatedFire = value == "true";
				if (key == "ofAnimatedPortal")
					ofAnimatedPortal = value == "true";
				if (key == "ofAnimatedRedstone")
					ofAnimatedRedstone = value == "true";
				if (key == "ofAnimatedExplosion")
					ofAnimatedExplosion = value == "true";
				if (key == "ofAnimatedFlame")
					ofAnimatedFlame = value == "true";
				if (key == "ofAnimatedSmoke")
					ofAnimatedSmoke = value == "true";
				if (key == "ofFastDebugInfo")
					ofFastDebugInfo = value == "true";
				if (key == "ofAutoSaveTicks")
					ofAutoSaveTicks = Config::limit((int_t)parseIntJava(value), 40, 40000);
				if (key == "ofBetterGrass")
					ofBetterGrass = Config::limit((int_t)parseIntJava(value), 1, 3);
				if (key == "ofWeather")
					ofWeather = value == "true";
				if (key == "ofSky")
					ofSky = value == "true";
				if (key == "ofStars")
					ofStars = value == "true";
				if (key == "ofChunkUpdates")
					ofChunkUpdates = Config::limit((int_t)parseIntJava(value), 1, 5);
				if (key == "ofChunkUpdatesDynamic")
					ofChunkUpdatesDynamic = value == "true";
				if (key == "ofFarView")
					ofFarView = value == "true";
				if (key == "ofTime")
					ofTime = Config::limit((int_t)parseIntJava(value), 0, 2);
				if (key == "ofClearWater")
				{
					ofClearWater = value == "true";
					updateWaterOpacity();
				}
				if (key == "ofSunMoon")
					ofSunMoon = value == "true";
				if (key == "ofDepthFog")
					ofDepthFog = value == "true";
				if (key == "ofProfiler")
					ofProfiler = value == "true";
				if (key == "ofBetterSnow")
					ofBetterSnow = value == "true";
				if (key == "ofSwampColors")
					ofSwampColors = value == "true";
				if (key == "ofSmoothBiomes")
					ofSmoothBiomes = value == "true";
				if (key == "ofRandomMobs")
					ofRandomMobs = value == "true";
				if (key == "ofCustomColors")
					ofCustomColors = value == "true";
				if (key == "ofConnectedTextures")
					ofConnectedTextures = Config::limit((int_t)parseIntJava(value), 1, 3);
				if (key == "ofNaturalTextures")
					ofNaturalTextures = value == "true";
				if (key == "ofMipmapLevel")
					ofMipmapLevel = Config::limit((int_t)parseIntJava(value), 0, 4);
				if (key == "ofMipmapLinear")
					ofMipmapLinear = value == "true";
				if (key == "ofAaLevel")
					ofAaLevel = Config::limit((int_t)parseIntJava(value), 0, 16);
				if (key == "ofAfLevel")
					ofAfLevel = Config::limit((int_t)parseIntJava(value), 1, 16);
				if (key == "ofCustomFonts")
					ofCustomFonts = value == "true";
				if (key == "ofRenderDistanceFine")
					ofRenderDistanceFine = Config::limit((int_t)parseIntJava(value), 32, Config::getMaxRenderDistanceFine());
				if (key == "ofVoidParticles")
					ofVoidParticles = value == "true";
				if (key == "ofWaterParticles")
					ofWaterParticles = value == "true";
				if (key == "ofRainSplash")
					ofRainSplash = value == "true";
				if (key == "ofPortalParticles")
					ofPortalParticles = value == "true";
				if (key == "ofDrippingWaterLava")
					ofDrippingWaterLava = value == "true";
				if (key == "ofAnimatedTerrain")
					ofAnimatedTerrain = value == "true";
				if (key == "ofAnimatedItems")
					ofAnimatedItems = value == "true";
				if (key == "ofAnimatedTextures")
					ofAnimatedTextures = value == "true";
			}
			catch (...)
			{
				MC_LOG_WARN("settings", "Skipping bad option: %s\n", s.c_str());
			}
		}
	}

	legacyGuiScaleRestore = legacyUiRestoreScaleAfterLoad(
		legacyUI, guiScale, loadedLegacyGuiScaleRestore, legacyGuiScaleRestore);
	guiScale = legacyUiEffectiveGuiScale(legacyUI, legacyGuiScaleRestore);

	if (particleSetting < 0 || particleSetting > 2)
		particleSetting = 0;
	fovSetting = Config::limit(fovSetting, 0.0f, 1.0f);

	ofRenderDistanceFine = Config::limit(ofRenderDistanceFine, 32, Config::getMaxRenderDistanceFine());
	platformGameSettingsFinalizeLoad(*this);
	PlatformUserSettings::setControllerDeadzone(controllerDeadzone);
	syncKeyBindingsToPlatform();
	syncControllerBindingsToPlatform();

	// Minecraft::start creates Session before GameSettings. Apply the persisted
	// offline name after options.txt has been read so the next handshake uses it.
	if (mc != nullptr && mc->session != nullptr)
		mc->session->username = playerName;
}

float GameSettings::parseFloat(const std::string &s)
{
	if (s == "true")
		return 1.0f;
	if (s == "false")
		return 0.0f;

	// Float.parseFloat is locale-independent and rejects trailing garbage.
	const std::string value = String::trimJava(s);
	if (value.empty())
		throw std::invalid_argument("empty float");
	float result = 0.0f;
#ifdef PS2_PLATFORM
	if (!CompactNumberFormat::parseFloatExact(value, result))
		throw std::invalid_argument("invalid float");
#else
	std::istringstream in(value);
	in.imbue(std::locale::classic());
	in >> std::noskipws >> result;
	if (!in || in.peek() != std::char_traits<char>::eof())
		throw std::invalid_argument("invalid float");
#endif
	return result;
}

void GameSettings::saveOptions()
{
	std::unordered_set<std::string> knownKeys = {
		"music", "sound", "invertYMouse", "mouseSensitivity", "fov", "viewDistance",
		"guiScale", "particles", "bobView", "anaglyph3d", "advancedOpengl", "fpsLimit",
		"difficulty", "fancyGraphics", "ao", "skin", "lastServer", "lang", "playerName", "selectedSkin", "selectedSkinP2", "legacyUI",
		"legacyLook", "legacyGuiScaleRestore",
		"alternativeControllerLayout", "wiiAlternativeControls", "controllerDeadzone", "wiiStickDeadzone",
		"ofFogFancy", "ofFogOff", "ofFogStart", "ofLoadFar", "ofPreloadedChunks", "ofOcclusionFancy",
		"ofSmoothFps", "ofSmoothInput", "ofBrightness", "ofAoLevel", "ofClouds",
		"ofCloudsHeight", "ofTrees", "ofGrass", "ofRain", "ofWater",
		"ofAnimatedWater", "ofAnimatedLava", "ofAnimatedFire", "ofAnimatedPortal",
		"ofAnimatedRedstone", "ofAnimatedExplosion", "ofAnimatedFlame",
		"ofAnimatedSmoke", "ofFastDebugInfo", "ofAutoSaveTicks", "ofBetterGrass",
		"ofWeather", "ofSky", "ofStars", "ofChunkUpdates", "ofChunkUpdatesDynamic",
		"ofFarView", "ofTime", "ofClearWater", "ofSunMoon", "ofDepthFog",
		"ofProfiler", "ofBetterSnow", "ofSwampColors", "ofSmoothBiomes", "ofRandomMobs", "ofCustomColors", "ofConnectedTextures", "ofNaturalTextures",
		"ofMipmapLevel", "ofMipmapLinear", "ofAaLevel", "ofAfLevel", "ofCustomFonts", "ofRenderDistanceFine",
		"ofVoidParticles", "ofWaterParticles", "ofRainSplash", "ofPortalParticles",
		"ofDrippingWaterLava", "ofAnimatedTerrain", "ofAnimatedItems", "ofAnimatedTextures"
	};
#if PLATFORM_HAS_ASPECT_RATIO_OPTION
	knownKeys.insert("widescreen");
	knownKeys.insert("aspectRatio");
	knownKeys.insert("options.aspectRatio");
	knownKeys.insert("options.aspectratio");
#endif
	platformGameSettingsAddKnownKeys(knownKeys);
	for (KeyBinding *binding : keyBindings)
		knownKeys.insert("key_" + binding->keyDescription);

	std::vector<std::string> preservedLines;
	std::vector<unsigned char> existingBytes;
	if (PlatformStorage::readFile(optionsFile, existingBytes))
	{
		const std::string existingText(existingBytes.begin(), existingBytes.end());
		std::string line;
#ifdef PS2_PLATFORM
		std::size_t existingPosition = 0;
		while (nextOptionLine(existingText, existingPosition, line))
#else
		std::istringstream existing(existingText);
		while (std::getline(existing, line))
#endif
		{
			const std::size_t pos = line.find(':');
			if (pos == std::string::npos || knownKeys.find(line.substr(0, pos)) == knownKeys.end())
				preservedLines.push_back(line);
		}
	}

#ifdef PS2_PLATFORM
	Ps2OptionWriter printwriter;
#else
	std::ostringstream printwriter;
	printwriter.imbue(std::locale::classic());
#endif
	for (const std::string &line : preservedLines)
		printwriter << line << "\n";

	printwriter << "music:" << musicVolume << "\n";
	printwriter << "sound:" << soundVolume << "\n";
	printwriter << "invertYMouse:" << (invertMouse ? "true" : "false") << "\n";
	printwriter << "mouseSensitivity:" << mouseSensitivity << "\n";
	printwriter << "fov:" << fovSetting << "\n";
	printwriter << "viewDistance:" << renderDistance << "\n";
	printwriter << "guiScale:" << guiScale << "\n";
	printwriter << "particles:" << particleSetting << "\n";
	printwriter << "bobView:" << (viewBobbing ? "true" : "false") << "\n";
	printwriter << "anaglyph3d:" << (anaglyph ? "true" : "false") << "\n";
	printwriter << "advancedOpengl:" << (advancedOpengl ? "true" : "false") << "\n";
	printwriter << "fpsLimit:" << limitFramerate << "\n";
	printwriter << "difficulty:" << difficulty << "\n";
	printwriter << "fancyGraphics:" << (fancyGraphics ? "true" : "false") << "\n";
	printwriter << "ao:" << (ambientOcclusion ? "true" : "false") << "\n";
	printwriter << "skin:" << skin << "\n";
	printwriter << "lastServer:" << lastServer << "\n";
	printwriter << "lang:" << language << "\n";
	printwriter << "playerName:" << playerName << "\n";
	printwriter << "selectedSkin:" << selectedSkin << "\n";
	printwriter << "selectedSkinP2:" << selectedSkinP2 << "\n";
	printwriter << "legacyUI:" << (legacyUI ? "true" : "false") << "\n";
	printwriter << "legacyLook:" << (legacyLook ? "true" : "false") << "\n";
	printwriter << "legacyGuiScaleRestore:" << legacyGuiScaleRestore << "\n";
	printwriter << "alternativeControllerLayout:" << (alternativeControllerLayout ? "true" : "false") << "\n";
	printwriter << "controllerDeadzone:" << controllerDeadzone << "\n";
#ifndef PS2_PLATFORM
	platformGameSettingsWriteOptions(*this, printwriter);
#endif
#if PLATFORM_HAS_ASPECT_RATIO_OPTION
	printwriter << "widescreen:" << (widescreen ? "true" : "false") << "\n";
#endif
	for (int_t i = 0; i < (int_t)keyBindings.size(); i++)
		printwriter << "key_" << keyBindings[i]->keyDescription << ":" << keyBindings[i]->keyCode << "\n";
	// --- OptiFine (mipmaps excluded) ---
	printwriter << "ofFogFancy:" << (ofFogFancy ? "true" : "false") << "\n";
	printwriter << "ofFogOff:" << (ofFogOff ? "true" : "false") << "\n";
	printwriter << "ofFogStart:" << ofFogStart << "\n";
	printwriter << "ofLoadFar:" << (ofLoadFar ? "true" : "false") << "\n";
	printwriter << "ofPreloadedChunks:" << ofPreloadedChunks << "\n";
	printwriter << "ofOcclusionFancy:" << (ofOcclusionFancy ? "true" : "false") << "\n";
	printwriter << "ofSmoothFps:" << (ofSmoothFps ? "true" : "false") << "\n";
	printwriter << "ofSmoothInput:" << (ofSmoothInput ? "true" : "false") << "\n";
	printwriter << "ofBrightness:" << ofBrightness << "\n";
	printwriter << "ofAoLevel:" << ofAoLevel << "\n";
	printwriter << "ofClouds:" << ofClouds << "\n";
	printwriter << "ofCloudsHeight:" << ofCloudsHeight << "\n";
	printwriter << "ofTrees:" << ofTrees << "\n";
	printwriter << "ofGrass:" << ofGrass << "\n";
	printwriter << "ofRain:" << ofRain << "\n";
	printwriter << "ofWater:" << ofWater << "\n";
	printwriter << "ofAnimatedWater:" << ofAnimatedWater << "\n";
	printwriter << "ofAnimatedLava:" << ofAnimatedLava << "\n";
	printwriter << "ofAnimatedFire:" << (ofAnimatedFire ? "true" : "false") << "\n";
	printwriter << "ofAnimatedPortal:" << (ofAnimatedPortal ? "true" : "false") << "\n";
	printwriter << "ofAnimatedRedstone:" << (ofAnimatedRedstone ? "true" : "false") << "\n";
	printwriter << "ofAnimatedExplosion:" << (ofAnimatedExplosion ? "true" : "false") << "\n";
	printwriter << "ofAnimatedFlame:" << (ofAnimatedFlame ? "true" : "false") << "\n";
	printwriter << "ofAnimatedSmoke:" << (ofAnimatedSmoke ? "true" : "false") << "\n";
	printwriter << "ofFastDebugInfo:" << (ofFastDebugInfo ? "true" : "false") << "\n";
	printwriter << "ofAutoSaveTicks:" << ofAutoSaveTicks << "\n";
	printwriter << "ofBetterGrass:" << ofBetterGrass << "\n";
	printwriter << "ofWeather:" << (ofWeather ? "true" : "false") << "\n";
	printwriter << "ofSky:" << (ofSky ? "true" : "false") << "\n";
	printwriter << "ofStars:" << (ofStars ? "true" : "false") << "\n";
	printwriter << "ofChunkUpdates:" << ofChunkUpdates << "\n";
	printwriter << "ofChunkUpdatesDynamic:" << (ofChunkUpdatesDynamic ? "true" : "false") << "\n";
	printwriter << "ofFarView:" << (ofFarView ? "true" : "false") << "\n";
	printwriter << "ofTime:" << ofTime << "\n";
	printwriter << "ofClearWater:" << (ofClearWater ? "true" : "false") << "\n";
	printwriter << "ofSunMoon:" << (ofSunMoon ? "true" : "false") << "\n";
	printwriter << "ofDepthFog:" << (ofDepthFog ? "true" : "false") << "\n";
	printwriter << "ofProfiler:" << (ofProfiler ? "true" : "false") << "\n";
	printwriter << "ofBetterSnow:" << (ofBetterSnow ? "true" : "false") << "\n";
	printwriter << "ofSwampColors:" << (ofSwampColors ? "true" : "false") << "\n";
	printwriter << "ofSmoothBiomes:" << (ofSmoothBiomes ? "true" : "false") << "\n";
	printwriter << "ofRandomMobs:" << (ofRandomMobs ? "true" : "false") << "\n";
	printwriter << "ofCustomColors:" << (ofCustomColors ? "true" : "false") << "\n";
	printwriter << "ofConnectedTextures:" << ofConnectedTextures << "\n";
	printwriter << "ofNaturalTextures:" << (ofNaturalTextures ? "true" : "false") << "\n";
	printwriter << "ofMipmapLevel:" << ofMipmapLevel << "\n";
	printwriter << "ofMipmapLinear:" << (ofMipmapLinear ? "true" : "false") << "\n";
	printwriter << "ofAaLevel:" << ofAaLevel << "\n";
	printwriter << "ofAfLevel:" << ofAfLevel << "\n";
	printwriter << "ofCustomFonts:" << (ofCustomFonts ? "true" : "false") << "\n";
	printwriter << "ofRenderDistanceFine:" << ofRenderDistanceFine << "\n";
	printwriter << "ofVoidParticles:" << (ofVoidParticles ? "true" : "false") << "\n";
	printwriter << "ofWaterParticles:" << (ofWaterParticles ? "true" : "false") << "\n";
	printwriter << "ofRainSplash:" << (ofRainSplash ? "true" : "false") << "\n";
	printwriter << "ofPortalParticles:" << (ofPortalParticles ? "true" : "false") << "\n";
	printwriter << "ofDrippingWaterLava:" << (ofDrippingWaterLava ? "true" : "false") << "\n";
	printwriter << "ofAnimatedTerrain:" << (ofAnimatedTerrain ? "true" : "false") << "\n";
	printwriter << "ofAnimatedItems:" << (ofAnimatedItems ? "true" : "false") << "\n";
	printwriter << "ofAnimatedTextures:" << (ofAnimatedTextures ? "true" : "false") << "\n";

	const std::string output = printwriter.str();
	bool saved = false;
	if (PlatformStorage::supportsAtomicRename())
	{
		const std::string temporaryFile = optionsFile + ".tmp";
		if (PlatformStorage::writeFile(temporaryFile, output.data(), output.size()))
		{
			saved = PlatformStorage::renameFile(temporaryFile, optionsFile);
			if (!saved)
				PlatformStorage::removeFile(temporaryFile);
		}
	}
	if (!saved)
		saved = PlatformStorage::writeFile(optionsFile, output.data(), output.size());
	if (!saved)
		MC_LOG_WARN("settings", "Failed to save options: %s\n", optionsFile.c_str());
}

