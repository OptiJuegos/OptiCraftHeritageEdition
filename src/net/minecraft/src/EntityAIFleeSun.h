#pragma once

#include "EntityAIBase.h"

class EntityCreature;
class Vec3D;
class World;

// net.minecraft.src.EntityAIFleeSun
class EntityAIFleeSun : public EntityAIBase
{
public:
	EntityAIFleeSun(EntityCreature *entity, float speed);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;

private:
	Vec3D *findPossibleShelter();

	EntityCreature *theCreature;
	World *theWorld;
	double shelterX;
	double shelterY;
	double shelterZ;
	float speed;
};
