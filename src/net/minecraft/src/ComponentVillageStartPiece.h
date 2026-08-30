#pragma once

#include <memory>
#include <vector>

#include "ComponentVillageWell.h"

class StructureVillagePieceWeight;
class WorldChunkManager;

// net.minecraft.src.ComponentVillageStartPiece
class ComponentVillageStartPiece : public ComponentVillageWell
{
public:
	ComponentVillageStartPiece(WorldChunkManager *manager, int_t type, Random &random,
	                           int_t x, int_t z,
	                           const std::vector<StructureVillagePieceWeight *> &weights,
	                           int_t terrainType);
	~ComponentVillageStartPiece() override;

	WorldChunkManager *getWorldChunkManager() const;

	WorldChunkManager *worldChunkMngr;
	int_t terrainType;
	StructureVillagePieceWeight *structVillagePieceWeight;
	std::vector<StructureVillagePieceWeight *> structureVillageWeightedPieceList;
	std::vector<std::unique_ptr<StructureVillagePieceWeight>> ownedVillagePieceWeights;
	std::vector<StructureComponent *> field_35108_e;
	std::vector<StructureComponent *> field_35106_f;
};
