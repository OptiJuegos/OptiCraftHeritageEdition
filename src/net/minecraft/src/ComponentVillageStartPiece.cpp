#include "ComponentVillageStartPiece.h"

#include "StructureVillagePieceWeight.h"
#include "WorldChunkManager.h"
#include "java/Random.h"

ComponentVillageStartPiece::ComponentVillageStartPiece(
	WorldChunkManager *manager, int_t, Random &random, int_t x, int_t z,
	const std::vector<StructureVillagePieceWeight *> &weights, int_t terrainTypeValue)
	: ComponentVillageWell(0, random, x, z),
	  worldChunkMngr(manager), terrainType(terrainTypeValue), structVillagePieceWeight(nullptr),
	  structureVillageWeightedPieceList(weights)
{
	ownedVillagePieceWeights.reserve(weights.size());
	for (StructureVillagePieceWeight *weight : weights)
		ownedVillagePieceWeights.emplace_back(weight);
}

ComponentVillageStartPiece::~ComponentVillageStartPiece() = default;

WorldChunkManager *ComponentVillageStartPiece::getWorldChunkManager() const
{
	return worldChunkMngr;
}
