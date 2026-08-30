#include "EntityAIBase.h"

EntityAIBase::EntityAIBase() : mutexBits(0)
{
}

bool EntityAIBase::continueExecuting()
{
	return shouldExecute();
}

bool EntityAIBase::isContinuous()
{
	return true;
}

void EntityAIBase::startExecuting()
{
}

void EntityAIBase::resetTask()
{
}

void EntityAIBase::updateTask()
{
}

void EntityAIBase::setMutexBits(int_t bits)
{
	mutexBits = bits;
}

int_t EntityAIBase::getMutexBits() const
{
	return mutexBits;
}
