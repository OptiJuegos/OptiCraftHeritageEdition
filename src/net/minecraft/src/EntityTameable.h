#pragma once

#include "EntityAnimal.h"

class EntityAISit;
class EntityLiving;
class NBTTagCompound;

// net.minecraft.src.EntityTameable
class EntityTameable : public EntityAnimal
{
public:
	explicit EntityTameable(World *world);

	void writeEntityToNBT(NBTTagCompound *compound) override;
	void readEntityFromNBT(NBTTagCompound *compound) override;
	void handleHealthUpdate(byte_t state) override;

	bool isTamed() const;
	void setTamed(bool value);
	bool isSitting() const;
	void setSitting(bool value);
	jstring getOwnerName() const;
	void setOwner(const jstring &owner);
	EntityLiving *getOwner() const;
	EntityAISit *getAISit() const;

protected:
	void entityInit() override;
	void showHeartsOrSmokeFX(bool success);

	EntityAISit *aiSit;
};
