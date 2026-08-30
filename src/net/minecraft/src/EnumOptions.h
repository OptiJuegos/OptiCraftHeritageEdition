#pragma once

#include "java/String.h"
#include "java/Type.h"
#include <array>

// net.minecraft.src.EnumOptions
class EnumOptions
{
public:
	static EnumOptions *MUSIC;
	static EnumOptions *SOUND;
	static EnumOptions *INVERT_MOUSE;
	static EnumOptions *SENSITIVITY;
	static EnumOptions *RENDER_DISTANCE;
	static EnumOptions *VIEW_BOBBING;
	static EnumOptions *ANAGLYPH;
	static EnumOptions *ADVANCED_OPENGL;
	static EnumOptions *FRAMERATE_LIMIT;
	static EnumOptions *DIFFICULTY;
	static EnumOptions *GRAPHICS;
	static EnumOptions *AMBIENT_OCCLUSION;
	static EnumOptions *GUI_SCALE;
	static EnumOptions *PARTICLES;
	static EnumOptions *FOV;

	// --- OptiFine ---
	static EnumOptions *FOG_FANCY;
	static EnumOptions *FOG_START;
	static EnumOptions *LOAD_FAR;
	static EnumOptions *PRELOADED_CHUNKS;
	static EnumOptions *SMOOTH_FPS;
	static EnumOptions *BRIGHTNESS;
	static EnumOptions *CLOUDS;
	static EnumOptions *CLOUD_HEIGHT;
	static EnumOptions *TREES;
	static EnumOptions *GRASS;
	static EnumOptions *RAIN;
	static EnumOptions *WATER;
	static EnumOptions *ANIMATED_WATER;
	static EnumOptions *ANIMATED_LAVA;
	static EnumOptions *ANIMATED_FIRE;
	static EnumOptions *ANIMATED_PORTAL;
	static EnumOptions *AO_LEVEL;
	static EnumOptions *FAST_DEBUG_INFO;
	static EnumOptions *AUTOSAVE_TICKS;
	static EnumOptions *BETTER_GRASS;
	static EnumOptions *ANIMATED_REDSTONE;
	static EnumOptions *ANIMATED_EXPLOSION;
	static EnumOptions *ANIMATED_FLAME;
	static EnumOptions *ANIMATED_SMOKE;
	static EnumOptions *WEATHER;
	static EnumOptions *SKY;
	static EnumOptions *STARS;
	static EnumOptions *FAR_VIEW;
	static EnumOptions *CHUNK_UPDATES;
	static EnumOptions *CHUNK_UPDATES_DYNAMIC;
	static EnumOptions *TIME;
	static EnumOptions *CLEAR_WATER;
	static EnumOptions *SMOOTH_INPUT;
	static EnumOptions *ASPECT_RATIO;
	static EnumOptions *SUN_MOON;
	static EnumOptions *DEPTH_FOG;
	static EnumOptions *PROFILER;
	static EnumOptions *BETTER_SNOW;
	static EnumOptions *SWAMP_COLORS;
	static EnumOptions *SMOOTH_BIOMES;
	static EnumOptions *VOID_PARTICLES;
	static EnumOptions *WATER_PARTICLES;
	static EnumOptions *RAIN_SPLASH;
	static EnumOptions *PORTAL_PARTICLES;
	static EnumOptions *DRIPPING_WATER_LAVA;
	static EnumOptions *ANIMATED_TERRAIN;
	static EnumOptions *ANIMATED_ITEMS;
	static EnumOptions *ANIMATED_TEXTURES;
	static EnumOptions *RANDOM_MOBS;
	static EnumOptions *CUSTOM_COLORS;
	static EnumOptions *CONNECTED_TEXTURES;
	static EnumOptions *NATURAL_TEXTURES;
	static EnumOptions *MIPMAP_LEVEL;
	static EnumOptions *MIPMAP_TYPE;
	static EnumOptions *CUSTOM_FONTS;
	static EnumOptions *AA_LEVEL;
	static EnumOptions *AF_LEVEL;
	static EnumOptions *RENDER_DISTANCE_FINE;

	static EnumOptions *getEnumOptions(int_t i);

	bool getEnumFloat() const { return enumFloat; }
	bool getEnumBoolean() const { return enumBoolean; }
	int_t returnEnumOrdinal() const { return ordinal; }
	jstring getEnumString() const { return enumString; }

private:
	EnumOptions(int_t ord, const jstring &s, bool flag, bool flag1)
		: ordinal(ord), enumString(s), enumFloat(flag), enumBoolean(flag1) {}

	int_t ordinal;
	jstring enumString;
	bool enumFloat;
	bool enumBoolean;
};
