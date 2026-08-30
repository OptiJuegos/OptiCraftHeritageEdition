#include "EnumArt.h"

const EnumArt EnumArt::Kebab        ("Kebab",         16, 16,   0,   0);
const EnumArt EnumArt::Aztec        ("Aztec",         16, 16,  16,   0);
const EnumArt EnumArt::Alban        ("Alban",         16, 16,  32,   0);
const EnumArt EnumArt::Aztec2       ("Aztec2",        16, 16,  48,   0);
const EnumArt EnumArt::Bomb         ("Bomb",          16, 16,  64,   0);
const EnumArt EnumArt::Plant        ("Plant",         16, 16,  80,   0);
const EnumArt EnumArt::Wasteland    ("Wasteland",     16, 16,  96,   0);
const EnumArt EnumArt::Pool         ("Pool",          32, 16,   0,  32);
const EnumArt EnumArt::Courbet      ("Courbet",       32, 16,  32,  32);
const EnumArt EnumArt::Sea          ("Sea",           32, 16,  64,  32);
const EnumArt EnumArt::Sunset       ("Sunset",        32, 16,  96,  32);
const EnumArt EnumArt::Creebet      ("Creebet",       32, 16, 128,  32);
const EnumArt EnumArt::Wanderer     ("Wanderer",      16, 32,   0,  64);
const EnumArt EnumArt::Graham       ("Graham",        16, 32,  16,  64);
const EnumArt EnumArt::Match        ("Match",         32, 32,   0, 128);
const EnumArt EnumArt::Bust         ("Bust",          32, 32,  32, 128);
const EnumArt EnumArt::Stage        ("Stage",         32, 32,  64, 128);
const EnumArt EnumArt::Void         ("Void",          32, 32,  96, 128);
const EnumArt EnumArt::SkullAndRoses("SkullAndRoses", 32, 32, 128, 128);
const EnumArt EnumArt::Fighters     ("Fighters",      64, 32,   0,  96);
const EnumArt EnumArt::Pointer      ("Pointer",       64, 64,   0, 192);
const EnumArt EnumArt::Pigscene     ("Pigscene",      64, 64,  64, 192);
const EnumArt EnumArt::BurningSkull ("BurningSkull",  64, 64, 128, 192);
const EnumArt EnumArt::Skeleton     ("Skeleton",      64, 48, 192,  64);
const EnumArt EnumArt::DonkeyKong   ("DonkeyKong",    64, 48, 192, 112);

const EnumArt *EnumArt::values[25] = {
	&EnumArt::Kebab,    &EnumArt::Aztec,    &EnumArt::Alban,   &EnumArt::Aztec2,
	&EnumArt::Bomb,     &EnumArt::Plant,    &EnumArt::Wasteland, &EnumArt::Pool,
	&EnumArt::Courbet,  &EnumArt::Sea,      &EnumArt::Sunset,  &EnumArt::Creebet,
	&EnumArt::Wanderer, &EnumArt::Graham,   &EnumArt::Match,   &EnumArt::Bust,
	&EnumArt::Stage,    &EnumArt::Void,     &EnumArt::SkullAndRoses, &EnumArt::Fighters,
	&EnumArt::Pointer,  &EnumArt::Pigscene, &EnumArt::BurningSkull, &EnumArt::Skeleton,
	&EnumArt::DonkeyKong
};

const int_t EnumArt::maxArtTitleLength = 13; // "SkullAndRoses".length()
