#pragma once

#include "Material.h"

class MapColor;

// net.minecraft.src.MaterialLogic
class MaterialLogic : public Material
{
public:
	MaterialLogic(MapColor *mapcolor);

	bool isSolid() override;
	bool getCanBlockGrass() override;
	bool getIsSolid() override;
	bool blocksMovement() override;
};
