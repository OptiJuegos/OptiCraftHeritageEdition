#include "StructureStart.h"

#include <algorithm>

#include "StructureBoundingBox.h"
#include "StructureComponent.h"
#include "World.h"
#include "java/Random.h"
#include "java/Arithmetic.h"

StructureStart::StructureStart() = default;

StructureStart::~StructureStart()
{
	for (StructureComponent *component : components)
		delete component;
}

StructureBoundingBox *StructureStart::getBoundingBox() const
{
	return boundingBox.get();
}

std::vector<StructureComponent *> &StructureStart::getComponents()
{
	return components;
}

const std::vector<StructureComponent *> &StructureStart::getComponents() const
{
	return components;
}

void StructureStart::addComponent(StructureComponent *component)
{
	if (component != nullptr)
		components.push_back(component);
}

void StructureStart::generateStructure(World *world, Random &random,
                                       const StructureBoundingBox &chunkBounds)
{
	for (auto it = components.begin(); it != components.end();)
	{
		StructureComponent *component = *it;
		if (component != nullptr && component->getBoundingBox() != nullptr &&
		    component->getBoundingBox()->intersectsWith(chunkBounds) &&
		    !component->addComponentParts(world, random, chunkBounds))
		{
			delete component;
			it = components.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void StructureStart::updateBoundingBox()
{
	if (components.empty())
	{
		boundingBox.reset();
		return;
	}

	boundingBox = std::make_unique<StructureBoundingBox>(StructureBoundingBox::getNewBoundingBox());
	for (StructureComponent *component : components)
	{
		if (component != nullptr && component->getBoundingBox() != nullptr)
			boundingBox->expandTo(*component->getBoundingBox());
	}
}

void StructureStart::markAvailableHeight(World *, Random &random, int_t offset)
{
	if (!boundingBox)
		return;

	const int_t availableHeight = JavaArithmetic::intSub(63, offset);
	int_t targetHeight = JavaArithmetic::intAdd(boundingBox->getYSize(), 1);
	if (targetHeight < availableHeight)
		targetHeight = JavaArithmetic::intAdd(targetHeight, random.nextInt(JavaArithmetic::intSub(availableHeight, targetHeight)));

	const int_t yOffset = JavaArithmetic::intSub(targetHeight, boundingBox->maxY);
	boundingBox->offset(0, yOffset, 0);
	for (StructureComponent *component : components)
	{
		if (component != nullptr && component->getBoundingBox() != nullptr)
			component->getBoundingBox()->offset(0, yOffset, 0);
	}
}

void StructureStart::setRandomHeight(World *, Random &random, int_t minY, int_t maxY)
{
	if (!boundingBox)
		return;

	const int_t available = JavaArithmetic::intSub(JavaArithmetic::intAdd(JavaArithmetic::intSub(maxY, minY), 1), boundingBox->getYSize());
	const int_t targetY = available > 1 ? JavaArithmetic::intAdd(minY, random.nextInt(available)) : minY;
	const int_t yOffset = JavaArithmetic::intSub(targetY, boundingBox->minY);
	boundingBox->offset(0, yOffset, 0);
	for (StructureComponent *component : components)
	{
		if (component != nullptr && component->getBoundingBox() != nullptr)
			component->getBoundingBox()->offset(0, yOffset, 0);
	}
}

bool StructureStart::isSizeableStructure() const
{
	return true;
}
