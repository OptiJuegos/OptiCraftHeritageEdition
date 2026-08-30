#pragma once
#include <vector>
#include "ComponentStrongholdStairs.h"
class ComponentStrongholdPortalRoom;
class StructureStrongholdPieceWeight;
class ComponentStrongholdStairs2 : public ComponentStrongholdStairs
{
public:
	ComponentStrongholdStairs2(int_t type, Random &random, int_t x, int_t z);
	ChunkPosition *getCenter() const override;
	StructureStrongholdPieceWeight *field_35038_a = nullptr;
	ComponentStrongholdPortalRoom *portalRoom = nullptr;
	std::vector<StructureComponent *> field_35037_b;
};
