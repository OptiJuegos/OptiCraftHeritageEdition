#pragma once

#include "EntityLiving.h"
#include "IMob.h"

class World;
class NBTTagCompound;
class EntityPlayer;

// net.minecraft.src.EntitySlime
class EntitySlime : public EntityLiving, public IMob
{
public:
	explicit EntitySlime(World *world);

	int_t getMaxHealth() const override;
	void setSlimeSize(int_t size);
	int_t getSlimeSize();
	void writeEntityToNBT(NBTTagCompound *tag) override;
	void readEntityFromNBT(NBTTagCompound *tag) override;
	void onUpdate() override;
	void setEntityDead() override;
	void onCollideWithPlayer(EntityPlayer *player) override;
	bool getCanSpawnHere() override;
	int_t getVerticalFaceSpeed() override;

protected:
	void entityInit() override;
	void updatePlayerActionState() override;
	virtual jstring getSlimeParticle();
	virtual jstring getSlimeSound();
	virtual void updateSquish();
	virtual int_t getJumpDelay();
	virtual EntitySlime *createInstance();
	virtual bool canDamagePlayer();
	virtual int_t getAttackStrength();
	virtual bool makesSoundOnJump();
	virtual bool makesSoundOnLand();
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDropItemId() override;
	float getSoundVolume() override;

public:
	float field_40139_a;
	float field_768_a;
	float field_767_b;

private:
	int_t slimeJumpDelay;
};
