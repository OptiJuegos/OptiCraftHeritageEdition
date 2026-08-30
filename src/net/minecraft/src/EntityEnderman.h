#pragma once

#include <array>

#include "EntityMob.h"

class EntityPlayer;
class DamageSource;

// net.minecraft.src.EntityEnderman
class EntityEnderman : public EntityMob
{
public:
	explicit EntityEnderman(World *world);

	int_t getMaxHealth() const override;
	void writeEntityToNBT(NBTTagCompound *tag) override;
	void readEntityFromNBT(NBTTagCompound *tag) override;
	void onLivingUpdate() override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	void setCarried(int_t blockId);
	int_t getCarried();
	void setCarryingData(int_t metadata);
	int_t getCarryingData();

	bool isAttacking;

protected:
	void entityInit() override;
	Entity *findPlayerToAttack() override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDropItemId() override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;
	virtual bool teleportRandomly();
	virtual bool teleportToEntity(Entity *entity);
	virtual bool teleportTo(double x, double y, double z);

private:
	bool shouldAttackPlayer(EntityPlayer *player);
	static void initializeCarryableBlocks();

	static std::array<bool, 4096> canCarryBlocks;
	static bool carryableBlocksInitialized;
	int_t teleportDelay;
	int_t stareTimer;
};
