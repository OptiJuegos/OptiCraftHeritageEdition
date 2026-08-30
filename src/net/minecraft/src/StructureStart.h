#pragma once

#include "java/Type.h"

#include <memory>
#include <vector>

class Random;
class StructureBoundingBox;
class StructureComponent;
class World;

// net.minecraft.src.StructureStart
class StructureStart
{
public:
	StructureStart();
	virtual ~StructureStart();

	StructureBoundingBox *getBoundingBox() const;
	std::vector<StructureComponent *> &getComponents();
	const std::vector<StructureComponent *> &getComponents() const;
	void generateStructure(World *world, Random &random, const StructureBoundingBox &chunkBounds);
	virtual bool isSizeableStructure() const;

protected:
	void addComponent(StructureComponent *component);
	void updateBoundingBox();
	void markAvailableHeight(World *world, Random &random, int_t offset);
	void setRandomHeight(World *world, Random &random, int_t minY, int_t maxY);

	std::vector<StructureComponent *> components;
	std::unique_ptr<StructureBoundingBox> boundingBox;
};
