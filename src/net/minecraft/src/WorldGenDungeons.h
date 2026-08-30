#pragma once

#include <string>
#include "WorldGenerator.h"

class ItemStack;

// net.minecraft.src.WorldGenDungeons
class WorldGenDungeons : public WorldGenerator
{
public:
	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;

private:
	ItemStack *pickCheckLootItem(Random &random);
	std::string pickMobSpawner(Random &random);
};
