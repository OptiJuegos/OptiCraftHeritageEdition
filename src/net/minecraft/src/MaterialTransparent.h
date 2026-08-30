#pragma once

#include "Material.h"

class MapColor;

// net.minecraft.src.MaterialTransparent
class MaterialTransparent : public Material
{
public:
	MaterialTransparent(MapColor *mapcolor);

	bool isSolid() override;
	bool getCanBlockGrass() override;
	bool getIsSolid() override;
	bool blocksMovement() override;
};
