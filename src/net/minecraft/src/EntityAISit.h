#pragma once

#include "EntityAIBase.h"

class EntityTameable;

// net.minecraft.src.EntityAISit
class EntityAISit : public EntityAIBase
{
public:
	explicit EntityAISit(EntityTameable *entity);

	bool shouldExecute() override;
	void startExecuting() override;
	void resetTask() override;
	void setSitting(bool value);
	void func_48407_a(bool value);

private:
	EntityTameable *theEntity;
	bool sittingRequested;
};
