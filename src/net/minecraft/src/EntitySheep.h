#pragma once

#include "EntityAnimal.h"
#include "java/Random.h"

class World;
class NBTTagCompound;
class Entity;
class EntityPlayer;
class EntityAIEatGrass;

// net.minecraft.src.EntitySheep
class EntitySheep : public EntityAnimal
{
public:
	explicit EntitySheep(World *world);

	static constexpr int_t CLASS_ID = 12;
	int_t getEntityClassID() const override { return CLASS_ID; }

	bool isAIEnabled() override;
	int_t getMaxHealth() const;
	void onLivingUpdate() override;
	bool attackEntityFrom(Entity *entity, int_t i) override { return EntityAnimal::attackEntityFrom(entity, i); }
	bool interact(EntityPlayer *entityplayer) override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	void handleHealthUpdate(byte_t byte0) override;
	float getHeadRotationPointY(float partialTick) const;
	float getHeadRotationAngleX(float partialTick) const;
	int_t getFleeceColor();
	void setFleeceColor(int_t i);
	bool getSheared();
	void setSheared(bool flag);
	EntityAnimal *spawnBabyAnimal(EntityAnimal *mate) override;
	void eatGrassBonus() override;

	static int_t getRandomFleeceColor(Random &random);
	static const float fleeceColorTable[16][3];

protected:
	void entityInit() override;
	void updateAITasks() override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;
	int_t getDropItemId() override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;

private:
	int_t sheepTimer;
	EntityAIEatGrass *aiEatGrass;
};
