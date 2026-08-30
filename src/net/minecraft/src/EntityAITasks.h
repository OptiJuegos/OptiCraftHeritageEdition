#pragma once

#include <memory>
#include <vector>

#include "java/Type.h"

class EntityAIBase;
class EntityAITaskEntry;

// net.minecraft.src.EntityAITasks
class EntityAITasks
{
public:
	EntityAITasks();
	~EntityAITasks();

	void addTask(int_t priority, EntityAIBase *action);
	void onUpdateTasks();

private:
	bool canUse(EntityAITaskEntry *entry) const;
	bool areTasksCompatible(EntityAITaskEntry *first, EntityAITaskEntry *second) const;
	bool isExecuting(const EntityAITaskEntry *entry) const;
	void removeExecuting(EntityAITaskEntry *entry);

	std::vector<std::unique_ptr<EntityAITaskEntry>> tasksToDo;
	std::vector<EntityAITaskEntry *> executingTasks;
};
