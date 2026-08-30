#include "StructureMineshaftPieces.h"

#include <cstdlib>
#include <memory>

#include "Block.h"
#include "ComponentMineshaftCorridor.h"
#include "ComponentMineshaftCross.h"
#include "ComponentMineshaftStairs.h"
#include "Item.h"
#include "StructureBoundingBox.h"
#include "StructureComponent.h"
#include "StructurePieceTreasure.h"
#include "java/Arithmetic.h"
#include "java/Random.h"

StructureComponent *StructureMineshaftPieces::getRandomComponent(
	std::vector<StructureComponent *> &components, Random &random,
	int x, int y, int z, int direction, int depth)
{
	const int roll = random.nextInt(100);
	std::unique_ptr<StructureBoundingBox> bounds;
	if (roll >= 80)
	{
		bounds = ComponentMineshaftCross::findValidPlacement(components, random, x, y, z, direction);
		if (bounds)
			return new ComponentMineshaftCross(depth, random, std::move(bounds), direction);
	}
	else if (roll >= 70)
	{
		bounds = ComponentMineshaftStairs::findValidPlacement(components, random, x, y, z, direction);
		if (bounds)
			return new ComponentMineshaftStairs(depth, random, std::move(bounds), direction);
	}
	else
	{
		bounds = ComponentMineshaftCorridor::findValidPlacement(components, random, x, y, z, direction);
		if (bounds)
			return new ComponentMineshaftCorridor(depth, random, std::move(bounds), direction);
	}
	return nullptr;
}

StructureComponent *StructureMineshaftPieces::getNextMineShaftComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components,
	Random &random, int x, int y, int z, int direction, int depth)
{
	if (depth > 8 || parent == nullptr || parent->getBoundingBox() == nullptr)
		return nullptr;
	if (JavaArithmetic::intAbs(JavaArithmetic::intSub(x, parent->getBoundingBox()->minX)) > 80 ||
	    JavaArithmetic::intAbs(JavaArithmetic::intSub(z, parent->getBoundingBox()->minZ)) > 80)
		return nullptr;

	StructureComponent *component = getRandomComponent(components, random, x, y, z, direction, depth + 1);
	if (component != nullptr)
	{
		components.push_back(component);
		component->buildComponent(parent, components, random);
	}
	return component;
}

StructureComponent *StructureMineshaftPieces::getNextComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components,
	Random &random, int x, int y, int z, int direction, int depth)
{
	return getNextMineShaftComponent(parent, components, random, x, y, z, direction, depth);
}

const std::vector<StructurePieceTreasure> &StructureMineshaftPieces::getTreasurePieces()
{
	static const std::vector<StructurePieceTreasure> loot = {
		StructurePieceTreasure(Item::ingotIron->shiftedIndex, 0, 1, 5, 10),
		StructurePieceTreasure(Item::ingotGold->shiftedIndex, 0, 1, 3, 5),
		StructurePieceTreasure(Item::redstone->shiftedIndex, 0, 4, 9, 5),
		StructurePieceTreasure(Item::dyePowder->shiftedIndex, 4, 4, 9, 5),
		StructurePieceTreasure(Item::diamond->shiftedIndex, 0, 1, 2, 3),
		StructurePieceTreasure(Item::coal->shiftedIndex, 0, 3, 8, 10),
		StructurePieceTreasure(Item::bread->shiftedIndex, 0, 1, 3, 15),
		StructurePieceTreasure(Item::pickaxeSteel->shiftedIndex, 0, 1, 1, 1),
		StructurePieceTreasure(Block::rail->blockID, 0, 4, 8, 1),
		StructurePieceTreasure(106 + 256, 0, 2, 4, 10),
		StructurePieceTreasure(105 + 256, 0, 2, 4, 10)
	};
	return loot;
}
