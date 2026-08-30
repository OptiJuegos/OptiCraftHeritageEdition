#pragma once

#include "java/String.h"

class EntityLiving;

// net.minecraft.src.PotionEffect
class PotionEffect
{
public:
	PotionEffect(int id, int duration, int amplifier);
	PotionEffect(const PotionEffect &other);

	void combine(const PotionEffect &other);
	int getPotionID() const;
	int getDuration() const;
	int getAmplifier() const;
	bool onUpdate(EntityLiving *entity);
	void performEffect(EntityLiving *entity);
	jstring getEffectName() const;
	bool equals(const PotionEffect &other) const;

private:
	int deincrementDuration();

	int potionID;
	int duration;
	int amplifier;
};
