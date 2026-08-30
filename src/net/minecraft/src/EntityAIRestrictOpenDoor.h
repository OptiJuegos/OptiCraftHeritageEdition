#pragma once

#include "EntityAIBase.h"

class EntityCreature;
class VillageDoorInfo;

// net.minecraft.src.EntityAIRestrictOpenDoor
class EntityAIRestrictOpenDoor : public EntityAIBase
{
public:
	explicit EntityAIRestrictOpenDoor(EntityCreature *entity);
	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	EntityCreature *entityObj;
	VillageDoorInfo *resolveFrontDoor() const;

	int_t doorX;
	int_t doorY;
	int_t doorZ;
	bool hasDoor;
};
