#include "StructureVillageStart.h"

#include "ComponentVillageRoadPiece.h"
#include "ComponentVillageStartPiece.h"
#include "StructureVillagePieces.h"
#include "World.h"
#include "java/Arithmetic.h"
#include "java/Random.h"

StructureVillageStart::StructureVillageStart(
	World *world, Random &random, int_t chunkX, int_t chunkZ, int_t terrainType)
	: hasMoreThanTwoComponents(false)
{
	std::vector<StructureVillagePieceWeight *> weights =
		StructureVillagePieces::getStructureVillageWeightedPieceList(random, terrainType);
	const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkX, 16), 2);
	const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkZ, 16), 2);
	auto *start = new ComponentVillageStartPiece(
		world->getWorldChunkManager(), 0, random, x, z, weights, terrainType);
	addComponent(start);
	start->buildComponent(start, components, random);

	auto &pendingRoads = start->field_35106_f;
	auto &pendingBuildings = start->field_35108_e;
	while (!pendingRoads.empty() || !pendingBuildings.empty())
	{
		std::vector<StructureComponent *> &pending =
			!pendingRoads.empty() ? pendingRoads : pendingBuildings;
		const int_t index = random.nextInt(static_cast<int_t>(pending.size()));
		StructureComponent *component = pending[static_cast<std::size_t>(index)];
		pending.erase(pending.begin() + index);
		component->buildComponent(start, components, random);
	}

	updateBoundingBox();
	int_t buildingCount = 0;
	for (StructureComponent *component : components)
	{
		if (dynamic_cast<ComponentVillageRoadPiece *>(component) == nullptr)
			++buildingCount;
	}
	hasMoreThanTwoComponents = buildingCount > 2;
}

bool StructureVillageStart::isSizeableStructure() const
{
	return hasMoreThanTwoComponents;
}
