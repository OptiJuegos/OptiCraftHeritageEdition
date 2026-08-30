#pragma once

#include "EntityGolem.h"

class Village;

// net.minecraft.src.EntityIronGolem
class EntityIronGolem : public EntityGolem
{
public:
	explicit EntityIronGolem(World *world);

	static constexpr int_t CLASS_ID = 29;
	int_t getEntityClassID() const override { return CLASS_ID; }

	bool isAIEnabled() override;
	int_t getMaxHealth() const;
	void onLivingUpdate() override;
	void writeEntityToNBT(NBTTagCompound *tag) override;
	void readEntityFromNBT(NBTTagCompound *tag) override;
	bool attackEntityAsMob(Entity *target) override;
	bool canAttackEntity(EntityLiving *target) override;
	bool func_48100_a(const std::type_info &type) const override;
	void handleHealthUpdate(byte_t state) override;

	Village *getVillage() const;
	int_t getAttackTimer() const;
	int_t func_48114_ab() const;
	void setHoldingRose(bool value);
	void func_48116_a(bool value);
	int_t getHoldRoseTick() const;
	int_t func_48117_D_() const;
	bool isPlayerCreated() const;
	bool func_48112_E_() const;
	void setPlayerCreated(bool value);
	void func_48115_b(bool value);

protected:
	void entityInit() override;
	int_t decreaseAirSupply(int_t airSupply) override;
	void updateAITick() override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	void playStepSound(int_t x, int_t y, int_t z, int_t blockId) override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;

private:
	Village *resolveVillage() const;

	int_t homeCheckTimer;
	int_t attackTimer;
	int_t holdRoseTick;
	int_t villageCenterX;
	int_t villageCenterY;
	int_t villageCenterZ;
	bool hasVillageCenter;
};
