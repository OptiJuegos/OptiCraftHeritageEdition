#pragma once

#include "java/Type.h"
#include "Material.h"
#include "IMob.h"

class EntityAnimal;
class EntityWaterMob;

// net.minecraft.src.EnumCreatureType
// Java's Class field replaced with a tag enum for type identification
enum class EnumCreatureTypeTag { monster_tag, creature_tag, waterCreature_tag };

class EnumCreatureType
{
public:
	static const EnumCreatureType monster;
	static const EnumCreatureType creature;
	static const EnumCreatureType waterCreature;

	EnumCreatureTypeTag getCreatureTag() const { return creatureTag; }
	int_t getMaxNumberOfCreature() const { return maxNumberOfCreature; }
	// Resolve the material live (by tag) instead of returning the pointer captured at
	// static-init time: Material::air/water are still nullptr when these singletons are
	// constructed (they are only set in Material::initialize() at runtime). Capturing them
	// at init made getCreatureMaterial() return nullptr, so SpawnerAnimals' material check
	// (block material != creatureMaterial) always failed and NOTHING ever spawned.
	Material *getCreatureMaterial() const
	{
		return creatureTag == EnumCreatureTypeTag::waterCreature_tag ? Material::water : Material::air;
	}
	bool getPeacefulCreature() const { return isPeacefulCreature; }

private:
	EnumCreatureType(EnumCreatureTypeTag tag, int_t j, Material *material, bool flag)
		: creatureTag(tag), maxNumberOfCreature(j), creatureMaterial(material), isPeacefulCreature(flag) {}

	EnumCreatureTypeTag creatureTag;
	int_t maxNumberOfCreature;
	Material *creatureMaterial;
	bool isPeacefulCreature;
};
