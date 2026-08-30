#pragma once

#include "java/Type.h"

// net.minecraft.src.EntityAIBase
class EntityAIBase
{
public:
	EntityAIBase();
	virtual ~EntityAIBase() = default;

	virtual bool shouldExecute() = 0;
	virtual bool continueExecuting();
	virtual bool isContinuous();
	virtual void startExecuting();
	virtual void resetTask();
	virtual void updateTask();

	void setMutexBits(int_t bits);
	int_t getMutexBits() const;

private:
	int_t mutexBits;
};
