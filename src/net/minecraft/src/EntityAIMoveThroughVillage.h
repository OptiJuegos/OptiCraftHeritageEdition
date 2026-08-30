#pragma once

#include <vector>

#include "EntityAIBase.h"
#include "ChunkCoordinates.h"

class EntityCreature;
class PathEntity;
class Village;
class VillageDoorInfo;

// net.minecraft.src.EntityAIMoveThroughVillage
class EntityAIMoveThroughVillage : public EntityAIBase
{
public:
	EntityAIMoveThroughVillage(EntityCreature *entity, float speed, bool noMovementAtDay);
	~EntityAIMoveThroughVillage() override;

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;

private:
	bool selectDoor(Village *village, ChunkCoordinates &result) const;
	bool isDoorRemembered(const VillageDoorInfo *door) const;
	void trimDoorList();

	EntityCreature *theEntity;
	float speed;
	PathEntity *path;
	ChunkCoordinates doorPosition;
	bool hasDoor;
	bool noMovementAtDay;
	std::vector<ChunkCoordinates> recentDoors;
};
