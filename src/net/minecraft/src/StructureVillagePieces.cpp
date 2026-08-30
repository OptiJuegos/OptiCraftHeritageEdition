#include "StructureVillagePieces.h"

#include <algorithm>
#include <cstdlib>

#include "ComponentVillageChurch.h"
#include "ComponentVillageField.h"
#include "ComponentVillageField2.h"
#include "ComponentVillageHall.h"
#include "ComponentVillageHouse1.h"
#include "ComponentVillageHouse2.h"
#include "ComponentVillageHouse3.h"
#include "ComponentVillageHouse4_Garden.h"
#include "ComponentVillagePathGen.h"
#include "ComponentVillageStartPiece.h"
#include "ComponentVillageTorch.h"
#include "ComponentVillageWoodHut.h"
#include "MapGenVillage.h"
#include "MathHelper.h"
#include "StructureBoundingBox.h"
#include "WorldChunkManager.h"
#include "java/Arithmetic.h"
#include "java/Random.h"

std::vector<StructureVillagePieceWeight *> StructureVillagePieces::getStructureVillageWeightedPieceList(
	Random &random, int_t terrainType)
{
	std::vector<StructureVillagePieceWeight *> weights;
	weights.push_back(new StructureVillagePieceWeight(VillagePieceType::House4Garden, 4,
		MathHelper::getRandomIntegerInRange(random, 2 + terrainType, 4 + terrainType * 2)));
	weights.push_back(new StructureVillagePieceWeight(VillagePieceType::Church, 20,
		MathHelper::getRandomIntegerInRange(random, terrainType, 1 + terrainType)));
	weights.push_back(new StructureVillagePieceWeight(VillagePieceType::House1, 20,
		MathHelper::getRandomIntegerInRange(random, terrainType, 2 + terrainType)));
	weights.push_back(new StructureVillagePieceWeight(VillagePieceType::WoodHut, 3,
		MathHelper::getRandomIntegerInRange(random, 2 + terrainType, 5 + terrainType * 3)));
	weights.push_back(new StructureVillagePieceWeight(VillagePieceType::Hall, 15,
		MathHelper::getRandomIntegerInRange(random, terrainType, 2 + terrainType)));
	weights.push_back(new StructureVillagePieceWeight(VillagePieceType::Field, 3,
		MathHelper::getRandomIntegerInRange(random, 1 + terrainType, 4 + terrainType)));
	weights.push_back(new StructureVillagePieceWeight(VillagePieceType::Field2, 3,
		MathHelper::getRandomIntegerInRange(random, 2 + terrainType, 4 + terrainType * 2)));
	weights.push_back(new StructureVillagePieceWeight(VillagePieceType::House2, 15,
		MathHelper::getRandomIntegerInRange(random, 0, 1 + terrainType)));
	weights.push_back(new StructureVillagePieceWeight(VillagePieceType::House3, 8,
		MathHelper::getRandomIntegerInRange(random, terrainType, 3 + terrainType * 2)));

	for (auto it = weights.begin(); it != weights.end();)
	{
		if ((*it)->villagePiecesLimit == 0)
		{
			delete *it;
			it = weights.erase(it);
		}
		else
		{
			++it;
		}
	}
	return weights;
}

int_t StructureVillagePieces::getAvailablePieceWeight(
	const std::vector<StructureVillagePieceWeight *> &weights)
{
	bool hasAvailable = false;
	int_t totalWeight = 0;
	for (StructureVillagePieceWeight *weight : weights)
	{
		if (weight->villagePiecesLimit > 0 && weight->villagePiecesSpawned < weight->villagePiecesLimit)
			hasAvailable = true;
		totalWeight += weight->villagePieceWeight;
	}
	return hasAvailable ? totalWeight : -1;
}

ComponentVillage *StructureVillagePieces::getVillageComponentFromWeightedPiece(
	StructureVillagePieceWeight *weight, std::vector<StructureComponent *> &components,
	Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	switch (weight->villagePieceType)
	{
	case VillagePieceType::House4Garden:
		return ComponentVillageHouse4_Garden::findValidPlacement(components, random, x, y, z, direction, depth);
	case VillagePieceType::Church:
		return ComponentVillageChurch::findValidPlacement(components, random, x, y, z, direction, depth);
	case VillagePieceType::House1:
		return ComponentVillageHouse1::findValidPlacement(components, random, x, y, z, direction, depth);
	case VillagePieceType::WoodHut:
		return ComponentVillageWoodHut::findValidPlacement(components, random, x, y, z, direction, depth);
	case VillagePieceType::Hall:
		return ComponentVillageHall::findValidPlacement(components, random, x, y, z, direction, depth);
	case VillagePieceType::Field:
		return ComponentVillageField::findValidPlacement(components, random, x, y, z, direction, depth);
	case VillagePieceType::Field2:
		return ComponentVillageField2::findValidPlacement(components, random, x, y, z, direction, depth);
	case VillagePieceType::House2:
		return ComponentVillageHouse2::findValidPlacement(components, random, x, y, z, direction, depth);
	case VillagePieceType::House3:
		return ComponentVillageHouse3::findValidPlacement(components, random, x, y, z, direction, depth);
	}
	return nullptr;
}

ComponentVillage *StructureVillagePieces::getNextVillageComponent(
	ComponentVillageStartPiece *start, std::vector<StructureComponent *> &components,
	Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	const int_t availableWeight = getAvailablePieceWeight(start->structureVillageWeightedPieceList);
	if (availableWeight <= 0)
		return nullptr;

	for (int_t attempt = 0; attempt < 5; ++attempt)
	{
		int_t selection = random.nextInt(availableWeight);
		for (StructureVillagePieceWeight *weight : start->structureVillageWeightedPieceList)
		{
			selection -= weight->villagePieceWeight;
			if (selection >= 0)
				continue;

			if (!weight->canSpawnMoreVillagePiecesOfType(depth) ||
		    (weight == start->structVillagePieceWeight && start->structureVillageWeightedPieceList.size() > 1))
				break;

			ComponentVillage *component = getVillageComponentFromWeightedPiece(
				weight, components, random, x, y, z, direction, depth);
			if (component != nullptr)
			{
				++weight->villagePiecesSpawned;
				start->structVillagePieceWeight = weight;
				if (!weight->canSpawnMoreVillagePieces())
				{
					auto &active = start->structureVillageWeightedPieceList;
					active.erase(std::remove(active.begin(), active.end(), weight), active.end());
				}
				return component;
			}
		}
	}

	auto bounds = ComponentVillageTorch::findValidPlacement(
		components, random, x, y, z, direction);
	if (!bounds)
		return nullptr;
	return new ComponentVillageTorch(depth, random, std::move(bounds), direction);
}

StructureComponent *StructureVillagePieces::getNextVillageStructureComponent(
	ComponentVillageStartPiece *start, std::vector<StructureComponent *> &components,
	Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	if (depth > 50 || JavaArithmetic::intAbs(JavaArithmetic::intSub(x, start->getBoundingBox()->minX)) > 112 ||
	    JavaArithmetic::intAbs(JavaArithmetic::intSub(z, start->getBoundingBox()->minZ)) > 112)
		return nullptr;

	ComponentVillage *component = getNextVillageComponent(
		start, components, random, x, y, z, direction, depth + 1);
	if (component == nullptr)
		return nullptr;

	StructureBoundingBox *bounds = component->getBoundingBox();
	const int_t centerX = (bounds->minX + bounds->maxX) / 2;
	const int_t centerZ = (bounds->minZ + bounds->maxZ) / 2;
	const int_t size = std::max(bounds->maxX - bounds->minX, bounds->maxZ - bounds->minZ);
	if (!start->getWorldChunkManager()->areBiomesViable(
		centerX, centerZ, size / 2 + 4, MapGenVillage::villageSpawnBiomes))
	{
		delete component;
		return nullptr;
	}

	components.push_back(component);
	start->field_35108_e.push_back(component);
	return component;
}

StructureComponent *StructureVillagePieces::getNextComponentVillagePath(
	ComponentVillageStartPiece *start, std::vector<StructureComponent *> &components,
	Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	if (depth > 3 + start->terrainType ||
	    JavaArithmetic::intAbs(JavaArithmetic::intSub(x, start->getBoundingBox()->minX)) > 112 ||
	    JavaArithmetic::intAbs(JavaArithmetic::intSub(z, start->getBoundingBox()->minZ)) > 112)
		return nullptr;

	auto bounds = ComponentVillagePathGen::func_35087_a(start, components, random, x, y, z, direction);
	if (!bounds || bounds->minY <= 10)
		return nullptr;

	auto *path = new ComponentVillagePathGen(depth, random, std::move(bounds), direction);
	StructureBoundingBox *pathBounds = path->getBoundingBox();
	const int_t centerX = (pathBounds->minX + pathBounds->maxX) / 2;
	const int_t centerZ = (pathBounds->minZ + pathBounds->maxZ) / 2;
	const int_t size = std::max(pathBounds->maxX - pathBounds->minX,
	                          pathBounds->maxZ - pathBounds->minZ);
	if (!start->getWorldChunkManager()->areBiomesViable(
		centerX, centerZ, size / 2 + 4, MapGenVillage::villageSpawnBiomes))
	{
		delete path;
		return nullptr;
	}

	components.push_back(path);
	start->field_35106_f.push_back(path);
	return path;
}

StructureComponent *StructureVillagePieces::getNextStructureComponent(
	ComponentVillageStartPiece *start, std::vector<StructureComponent *> &components,
	Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	return getNextVillageStructureComponent(start, components, random, x, y, z, direction, depth);
}

StructureComponent *StructureVillagePieces::getNextStructureComponentVillagePath(
	ComponentVillageStartPiece *start, std::vector<StructureComponent *> &components,
	Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	return getNextComponentVillagePath(start, components, random, x, y, z, direction, depth);
}
