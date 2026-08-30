#pragma once

#include "Material.h"

class MapColor;

// net.minecraft.src.MaterialPortal
class MaterialPortal : public Material
{
public:
	MaterialPortal(MapColor *mapcolor);

	bool isSolid() override;
	bool getCanBlockGrass() override;
	bool getIsSolid() override;
	bool blocksMovement() override;
};
