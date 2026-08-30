#pragma once

#include "java/Type.h"

class EntityAIBase;
class EntityAITasks;

// net.minecraft.src.EntityAITaskEntry
class EntityAITaskEntry
{
public:
	EntityAITaskEntry(EntityAITasks *tasks, int_t priority, EntityAIBase *action);
	~EntityAITaskEntry();

	EntityAIBase *action;
	int_t priority;
	EntityAITasks *tasks;
};
