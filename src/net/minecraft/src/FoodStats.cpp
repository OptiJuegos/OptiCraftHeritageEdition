#include "FoodStats.h"

#include <algorithm>

#include "DamageSource.h"
#include "EntityPlayer.h"
#include "ItemFood.h"
#include "NBTTagCompound.h"
#include "World.h"

FoodStats::FoodStats() :
	foodLevel(20),
	foodSaturationLevel(5.0f),
	foodExhaustionLevel(0.0f),
	foodTimer(0),
	prevFoodLevel(20)
{
}

void FoodStats::addStats(int food, float saturationModifier)
{
	foodLevel = std::min(foodLevel + food, 20);
	foodSaturationLevel = std::min(foodSaturationLevel + (float)food * saturationModifier * 2.0f, (float)foodLevel);
}

void FoodStats::addStats(ItemFood *food)
{
	if (food != nullptr)
		addStats(food->getHealAmount(), food->getSaturationModifier());
}

void FoodStats::onUpdate(EntityPlayer *player)
{
	if (player == nullptr || player->worldObj == nullptr)
		return;

	int difficulty = player->worldObj->difficultySetting;
	prevFoodLevel = foodLevel;

	if (foodExhaustionLevel > 4.0f)
	{
		foodExhaustionLevel -= 4.0f;
		if (foodSaturationLevel > 0.0f)
			foodSaturationLevel = std::max(foodSaturationLevel - 1.0f, 0.0f);
		else if (difficulty > 0)
			foodLevel = std::max(foodLevel - 1, 0);
	}

	if (foodLevel >= 18 && player->shouldHeal())
	{
		++foodTimer;
		if (foodTimer >= 80)
		{
			player->heal(1);
			foodTimer = 0;
		}
	}
	else if (foodLevel <= 0)
	{
		++foodTimer;
		if (foodTimer >= 80)
		{
			int health = player->getHealth();
			if (health > 10 || difficulty >= 3 || (health > 1 && difficulty >= 2))
				player->attackEntityFrom(DamageSource::starve, 1);
			foodTimer = 0;
		}
	}
	else
	{
		foodTimer = 0;
	}
}

void FoodStats::readNBT(NBTTagCompound *compound)
{
	if (compound == nullptr || !compound->hasKey("foodLevel"))
		return;

	foodLevel = compound->getInteger("foodLevel");
	foodTimer = compound->getInteger("foodTickTimer");
	foodSaturationLevel = compound->getFloat("foodSaturationLevel");
	foodExhaustionLevel = compound->getFloat("foodExhaustionLevel");
}

void FoodStats::writeNBT(NBTTagCompound *compound) const
{
	if (compound == nullptr)
		return;
	compound->setInteger("foodLevel", foodLevel);
	compound->setInteger("foodTickTimer", foodTimer);
	compound->setFloat("foodSaturationLevel", foodSaturationLevel);
	compound->setFloat("foodExhaustionLevel", foodExhaustionLevel);
}

int FoodStats::getFoodLevel() const
{
	return foodLevel;
}

int FoodStats::getPrevFoodLevel() const
{
	return prevFoodLevel;
}

bool FoodStats::needFood() const
{
	return foodLevel < 20;
}

void FoodStats::addExhaustion(float exhaustion)
{
	foodExhaustionLevel = std::min(foodExhaustionLevel + exhaustion, 40.0f);
}

float FoodStats::getSaturationLevel() const
{
	return foodSaturationLevel;
}

void FoodStats::setFoodLevel(int food)
{
	foodLevel = food;
}

void FoodStats::setFoodSaturationLevel(float saturation)
{
	foodSaturationLevel = saturation;
}
