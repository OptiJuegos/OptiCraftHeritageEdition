#include "EnumOptions.h"

EnumOptions *EnumOptions::MUSIC = new EnumOptions(0, "options.music", true, false);
EnumOptions *EnumOptions::SOUND = new EnumOptions(1, "options.sound", true, false);
EnumOptions *EnumOptions::INVERT_MOUSE = new EnumOptions(2, "options.invertMouse", false, true);
EnumOptions *EnumOptions::SENSITIVITY = new EnumOptions(3, "options.sensitivity", true, false);
EnumOptions *EnumOptions::RENDER_DISTANCE = new EnumOptions(4, "options.renderDistance", false, false);
EnumOptions *EnumOptions::VIEW_BOBBING = new EnumOptions(5, "options.viewBobbing", false, true);
EnumOptions *EnumOptions::ANAGLYPH = new EnumOptions(6, "options.anaglyph", false, true);
EnumOptions *EnumOptions::ADVANCED_OPENGL = new EnumOptions(7, "options.advancedOpengl", false, true);
EnumOptions *EnumOptions::FRAMERATE_LIMIT = new EnumOptions(8, "options.framerateLimit", false, false);
EnumOptions *EnumOptions::DIFFICULTY = new EnumOptions(9, "options.difficulty", false, false);
EnumOptions *EnumOptions::GRAPHICS = new EnumOptions(10, "options.graphics", false, false);
EnumOptions *EnumOptions::AMBIENT_OCCLUSION = new EnumOptions(11, "options.ao", false, true);
EnumOptions *EnumOptions::GUI_SCALE = new EnumOptions(12, "options.guiScale", false, false);
// Appended after the port-specific options to preserve existing saved/control ordinals.
EnumOptions *EnumOptions::PARTICLES = new EnumOptions(47, "options.particles", false, false);
EnumOptions *EnumOptions::FOV = new EnumOptions(48, "options.fov", true, false);
EnumOptions *EnumOptions::SUN_MOON = new EnumOptions(49, "Sun & Moon", false, false);
EnumOptions *EnumOptions::DEPTH_FOG = new EnumOptions(50, "Depth Fog", false, false);
EnumOptions *EnumOptions::PROFILER = new EnumOptions(51, "Debug Profiler", false, false);
EnumOptions *EnumOptions::BETTER_SNOW = new EnumOptions(52, "Better Snow", false, false);
EnumOptions *EnumOptions::SWAMP_COLORS = new EnumOptions(53, "Swamp Colors", false, false);
EnumOptions *EnumOptions::SMOOTH_BIOMES = new EnumOptions(54, "Smooth Biomes", false, false);
EnumOptions *EnumOptions::VOID_PARTICLES = new EnumOptions(55, "Void Particles", false, false);
EnumOptions *EnumOptions::WATER_PARTICLES = new EnumOptions(56, "Water Particles", false, false);
EnumOptions *EnumOptions::RAIN_SPLASH = new EnumOptions(57, "Rain Splash", false, false);
EnumOptions *EnumOptions::PORTAL_PARTICLES = new EnumOptions(58, "Portal Particles", false, false);
EnumOptions *EnumOptions::DRIPPING_WATER_LAVA = new EnumOptions(59, "Dripping Water/Lava", false, false);
EnumOptions *EnumOptions::ANIMATED_TERRAIN = new EnumOptions(60, "Terrain Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_ITEMS = new EnumOptions(61, "Items Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_TEXTURES = new EnumOptions(62, "Textures Animated", false, false);
EnumOptions *EnumOptions::RANDOM_MOBS = new EnumOptions(63, "Random Mobs", false, false);
EnumOptions *EnumOptions::CUSTOM_COLORS = new EnumOptions(64, "Custom Colors", false, false);
EnumOptions *EnumOptions::CONNECTED_TEXTURES = new EnumOptions(65, "Connected Textures", false, false);
EnumOptions *EnumOptions::NATURAL_TEXTURES = new EnumOptions(66, "Natural Textures", false, false);
EnumOptions *EnumOptions::MIPMAP_LEVEL = new EnumOptions(67, "Mipmap Level", false, false);
EnumOptions *EnumOptions::MIPMAP_TYPE = new EnumOptions(68, "Mipmap Type", false, false);
EnumOptions *EnumOptions::CUSTOM_FONTS = new EnumOptions(69, "Custom Fonts", false, false);
EnumOptions *EnumOptions::AA_LEVEL = new EnumOptions(70, "Antialiasing", false, false);
EnumOptions *EnumOptions::AF_LEVEL = new EnumOptions(71, "Anisotropic Filtering", false, false);
EnumOptions *EnumOptions::RENDER_DISTANCE_FINE = new EnumOptions(72, "Render Distance", true, false);

// --- OptiFine ---
// Ordinales secuenciales por orden de declaracion (ht::c() == ordinal()).
EnumOptions *EnumOptions::FOG_FANCY = new EnumOptions(13, "Fog", false, false);
EnumOptions *EnumOptions::FOG_START = new EnumOptions(14, "Fog Start", false, false);
EnumOptions *EnumOptions::LOAD_FAR = new EnumOptions(15, "Load Far", false, false);
EnumOptions *EnumOptions::PRELOADED_CHUNKS = new EnumOptions(16, "Preloaded Chunks", false, false);
EnumOptions *EnumOptions::SMOOTH_FPS = new EnumOptions(17, "Smooth FPS", false, false);
EnumOptions *EnumOptions::BRIGHTNESS = new EnumOptions(18, "Brightness", true, false);
EnumOptions *EnumOptions::CLOUDS = new EnumOptions(19, "Clouds", false, false);
EnumOptions *EnumOptions::CLOUD_HEIGHT = new EnumOptions(20, "Cloud Height", true, false);
EnumOptions *EnumOptions::TREES = new EnumOptions(21, "Trees", false, false);
EnumOptions *EnumOptions::GRASS = new EnumOptions(22, "Grass", false, false);
EnumOptions *EnumOptions::RAIN = new EnumOptions(23, "Rain & Snow", false, false);
EnumOptions *EnumOptions::WATER = new EnumOptions(24, "Water", false, false);
EnumOptions *EnumOptions::ANIMATED_WATER = new EnumOptions(25, "Water Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_LAVA = new EnumOptions(26, "Lava Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_FIRE = new EnumOptions(27, "Fire Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_PORTAL = new EnumOptions(28, "Portal Animated", false, false);
EnumOptions *EnumOptions::AO_LEVEL = new EnumOptions(29, "Smooth Lighting", true, false);
EnumOptions *EnumOptions::FAST_DEBUG_INFO = new EnumOptions(30, "Fast Debug Info", false, false);
EnumOptions *EnumOptions::AUTOSAVE_TICKS = new EnumOptions(31, "Autosave", false, false);
EnumOptions *EnumOptions::BETTER_GRASS = new EnumOptions(32, "Better Grass", false, false);
EnumOptions *EnumOptions::ANIMATED_REDSTONE = new EnumOptions(33, "Redstone Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_EXPLOSION = new EnumOptions(34, "Explosion Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_FLAME = new EnumOptions(35, "Flame Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_SMOKE = new EnumOptions(36, "Smoke Animated", false, false);
EnumOptions *EnumOptions::WEATHER = new EnumOptions(37, "Weather", false, false);
EnumOptions *EnumOptions::SKY = new EnumOptions(38, "Sky", false, false);
EnumOptions *EnumOptions::STARS = new EnumOptions(39, "Stars", false, false);
EnumOptions *EnumOptions::FAR_VIEW = new EnumOptions(40, "Far View", false, false);
EnumOptions *EnumOptions::CHUNK_UPDATES = new EnumOptions(41, "Chunk Updates", false, false);
EnumOptions *EnumOptions::CHUNK_UPDATES_DYNAMIC = new EnumOptions(42, "Dynamic Updates", false, false);
EnumOptions *EnumOptions::TIME = new EnumOptions(43, "Time", false, false);
EnumOptions *EnumOptions::CLEAR_WATER = new EnumOptions(44, "Clear Water", false, false);
EnumOptions *EnumOptions::SMOOTH_INPUT = new EnumOptions(45, "Smooth Input", false, false);
EnumOptions *EnumOptions::ASPECT_RATIO = new EnumOptions(46, "options.aspectRatio", false, false);

static EnumOptions *s_allOptions[] = {
    EnumOptions::MUSIC, EnumOptions::SOUND, EnumOptions::INVERT_MOUSE,
    EnumOptions::SENSITIVITY, EnumOptions::RENDER_DISTANCE, EnumOptions::VIEW_BOBBING,
    EnumOptions::ANAGLYPH, EnumOptions::ADVANCED_OPENGL, EnumOptions::FRAMERATE_LIMIT,
    EnumOptions::DIFFICULTY, EnumOptions::GRAPHICS, EnumOptions::AMBIENT_OCCLUSION,
    EnumOptions::GUI_SCALE,
    EnumOptions::FOG_FANCY, EnumOptions::FOG_START, EnumOptions::LOAD_FAR,
    EnumOptions::PRELOADED_CHUNKS, EnumOptions::SMOOTH_FPS, EnumOptions::BRIGHTNESS,
    EnumOptions::CLOUDS, EnumOptions::CLOUD_HEIGHT, EnumOptions::TREES,
    EnumOptions::GRASS, EnumOptions::RAIN, EnumOptions::WATER,
    EnumOptions::ANIMATED_WATER, EnumOptions::ANIMATED_LAVA, EnumOptions::ANIMATED_FIRE,
    EnumOptions::ANIMATED_PORTAL, EnumOptions::AO_LEVEL, EnumOptions::FAST_DEBUG_INFO,
    EnumOptions::AUTOSAVE_TICKS, EnumOptions::BETTER_GRASS, EnumOptions::ANIMATED_REDSTONE,
    EnumOptions::ANIMATED_EXPLOSION, EnumOptions::ANIMATED_FLAME, EnumOptions::ANIMATED_SMOKE,
    EnumOptions::WEATHER, EnumOptions::SKY, EnumOptions::STARS,
    EnumOptions::FAR_VIEW, EnumOptions::CHUNK_UPDATES, EnumOptions::CHUNK_UPDATES_DYNAMIC,
	EnumOptions::TIME, EnumOptions::CLEAR_WATER, EnumOptions::SMOOTH_INPUT,
	EnumOptions::ASPECT_RATIO, EnumOptions::PARTICLES, EnumOptions::FOV,
	EnumOptions::SUN_MOON, EnumOptions::DEPTH_FOG, EnumOptions::PROFILER, EnumOptions::BETTER_SNOW,
	EnumOptions::SWAMP_COLORS, EnumOptions::SMOOTH_BIOMES, EnumOptions::VOID_PARTICLES, EnumOptions::WATER_PARTICLES,
	EnumOptions::RAIN_SPLASH, EnumOptions::PORTAL_PARTICLES, EnumOptions::DRIPPING_WATER_LAVA,
	EnumOptions::ANIMATED_TERRAIN, EnumOptions::ANIMATED_ITEMS, EnumOptions::ANIMATED_TEXTURES,
	EnumOptions::RANDOM_MOBS, EnumOptions::CUSTOM_COLORS, EnumOptions::CONNECTED_TEXTURES, EnumOptions::NATURAL_TEXTURES,
	EnumOptions::MIPMAP_LEVEL, EnumOptions::MIPMAP_TYPE, EnumOptions::CUSTOM_FONTS,
	EnumOptions::AA_LEVEL, EnumOptions::AF_LEVEL, EnumOptions::RENDER_DISTANCE_FINE
};

EnumOptions *EnumOptions::getEnumOptions(int_t i)
{
    for (EnumOptions *opt : s_allOptions)
        if (opt->returnEnumOrdinal() == i)
            return opt;
    return nullptr;
}
