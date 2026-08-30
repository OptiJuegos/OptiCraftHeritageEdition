#pragma once

#include "Material.h"

// net.minecraft.src.MaterialWeb
class MaterialWeb final : public Material
{
public:
	explicit MaterialWeb(MapColor *mapColor) : Material(mapColor) {}
	bool blocksMovement() override { return false; }
};
