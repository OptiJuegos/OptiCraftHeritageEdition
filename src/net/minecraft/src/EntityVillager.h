#pragma once

#include "EntityAgeable.h"

// net.minecraft.src.EntityVillager
class Village;

class EntityVillager : public EntityAgeable
{
public:
	explicit EntityVillager(World *world);
	EntityVillager(World *world, int_t profession);

	static constexpr int_t CLASS_ID = 28;
	int_t getEntityClassID() const override { return CLASS_ID; }

	const char *getEntityTexture() override;
	void writeEntityToNBT(NBTTagCompound *tag) override;
	void readEntityFromNBT(NBTTagCompound *tag) override;
	void onLivingUpdate() override;
	bool attackEntityFrom(Entity *entity, int_t damage) override;
	void setRevengeTarget(EntityLiving *target) override;
	bool isAIEnabled() override;
	int_t getMaxHealth() const;

	void setProfession(int_t profession);
	int_t getProfession();
	bool getIsMatingFlag() const;
	void setIsMatingFlag(bool value);
	void setIsPlayingFlag(bool value);
	bool getIsPlayingFlag() const;

protected:
	void entityInit() override;
	void updateAITick() override;
	bool canDespawn() override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;

private:
	int_t randomTickDivider;
	bool isMatingFlag;
	bool isPlayingFlag;
};
