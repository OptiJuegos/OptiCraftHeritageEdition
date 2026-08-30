#include "EntityAITasks.h"

#include <algorithm>

#include "EntityAIBase.h"
#include "EntityAITaskEntry.h"

EntityAITasks::EntityAITasks() = default;

EntityAITasks::~EntityAITasks() = default;

void EntityAITasks::addTask(int_t priority, EntityAIBase *action)
{
	if (action == nullptr)
		return;
	tasksToDo.push_back(std::unique_ptr<EntityAITaskEntry>(new EntityAITaskEntry(this, priority, action)));
}

void EntityAITasks::onUpdateTasks()
{
	std::vector<EntityAITaskEntry *> startingTasks;
	for (const std::unique_ptr<EntityAITaskEntry> &ownedEntry : tasksToDo)
	{
		EntityAITaskEntry *entry = ownedEntry.get();
		bool executing = isExecuting(entry);
		if (executing)
		{
			if (!canUse(entry) || !entry->action->continueExecuting())
			{
				entry->action->resetTask();
				removeExecuting(entry);
				executing = false;
			}
		}

		if (!executing && canUse(entry) && entry->action->shouldExecute())
		{
			startingTasks.push_back(entry);
			executingTasks.push_back(entry);
		}
	}

	for (EntityAITaskEntry *entry : startingTasks)
		entry->action->startExecuting();
	for (EntityAITaskEntry *entry : executingTasks)
		entry->action->updateTask();
}

bool EntityAITasks::canUse(EntityAITaskEntry *entry) const
{
	for (const std::unique_ptr<EntityAITaskEntry> &ownedOther : tasksToDo)
	{
		EntityAITaskEntry *other = ownedOther.get();
		if (other == entry)
			continue;

		if (entry->priority >= other->priority)
		{
			if (isExecuting(other) && !areTasksCompatible(entry, other))
				return false;
		}
		else if (isExecuting(other) && !other->action->isContinuous())
		{
			return false;
		}
	}
	return true;
}

bool EntityAITasks::areTasksCompatible(EntityAITaskEntry *first, EntityAITaskEntry *second) const
{
	return (first->action->getMutexBits() & second->action->getMutexBits()) == 0;
}

bool EntityAITasks::isExecuting(const EntityAITaskEntry *entry) const
{
	return std::find(executingTasks.begin(), executingTasks.end(), entry) != executingTasks.end();
}

void EntityAITasks::removeExecuting(EntityAITaskEntry *entry)
{
	auto it = std::find(executingTasks.begin(), executingTasks.end(), entry);
	if (it != executingTasks.end())
		executingTasks.erase(it);
}
