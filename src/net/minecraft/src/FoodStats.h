#pragma once

class EntityPlayer;
class ItemFood;
class NBTTagCompound;

// net.minecraft.src.FoodStats
class FoodStats
{
public:
	FoodStats();

	void addStats(int food, float saturationModifier);
	void addStats(ItemFood *food);
	void onUpdate(EntityPlayer *player);
	void readNBT(NBTTagCompound *compound);
	void writeNBT(NBTTagCompound *compound) const;
	int getFoodLevel() const;
	int getPrevFoodLevel() const;
	bool needFood() const;
	void addExhaustion(float exhaustion);
	float getSaturationLevel() const;
	void setFoodLevel(int food);
	void setFoodSaturationLevel(float saturation);

private:
	int foodLevel;
	float foodSaturationLevel;
	float foodExhaustionLevel;
	int foodTimer;
	int prevFoodLevel;
};
