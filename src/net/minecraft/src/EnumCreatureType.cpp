#include "EnumCreatureType.h"

const EnumCreatureType EnumCreatureType::monster(EnumCreatureTypeTag::monster_tag, 70, Material::air, false);
const EnumCreatureType EnumCreatureType::creature(EnumCreatureTypeTag::creature_tag, 15, Material::air, true);
const EnumCreatureType EnumCreatureType::waterCreature(EnumCreatureTypeTag::waterCreature_tag, 5, Material::water, true);
