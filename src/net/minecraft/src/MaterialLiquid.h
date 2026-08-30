#pragma once

#include "Material.h"

class MapColor;

// net.minecraft.src.MaterialLiquid
class MaterialLiquid : public Material
{
public:
	MaterialLiquid(MapColor *mapcolor);

	bool getIsLiquid() override;
	bool getIsSolid() override;
	bool blocksMovement() override;
	bool isSolid() override;
};
