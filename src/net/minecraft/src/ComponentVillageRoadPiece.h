#pragma once

#include "ComponentVillage.h"

// net.minecraft.src.ComponentVillageRoadPiece
class ComponentVillageRoadPiece : public ComponentVillage
{
protected:
	explicit ComponentVillageRoadPiece(int_t componentType) : ComponentVillage(componentType) {}
};
