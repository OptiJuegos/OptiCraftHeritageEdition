#include "PotionEffect.h"

#include <cstdio>

#include "EntityLiving.h"
#include "Potion.h"

PotionEffect::PotionEffect(int id, int effectDuration, int effectAmplifier) :
	potionID(id), duration(effectDuration), amplifier(effectAmplifier)
{
}

PotionEffect::PotionEffect(const PotionEffect &other) :
	potionID(other.potionID), duration(other.duration), amplifier(other.amplifier)
{
}

void PotionEffect::combine(const PotionEffect &other)
{
	if (potionID != other.potionID)
		std::fprintf(stderr, "This method should only be called for matching effects!\n");

	if (other.amplifier > amplifier)
	{
		amplifier = other.amplifier;
		duration = other.duration;
	}
	else if (other.amplifier == amplifier && duration < other.duration)
	{
		duration = other.duration;
	}
}

int PotionEffect::getPotionID() const
{
	return potionID;
}

int PotionEffect::getDuration() const
{
	return duration;
}

int PotionEffect::getAmplifier() const
{
	return amplifier;
}

bool PotionEffect::onUpdate(EntityLiving *entity)
{
	if (duration > 0)
	{
		Potion *potion = Potion::getPotion(potionID);
		if (potion != nullptr && potion->isReady(duration, amplifier))
			performEffect(entity);
		deincrementDuration();
	}
	return duration > 0;
}

int PotionEffect::deincrementDuration()
{
	return --duration;
}

void PotionEffect::performEffect(EntityLiving *entity)
{
	if (duration <= 0)
		return;
	Potion *potion = Potion::getPotion(potionID);
	if (potion != nullptr)
		potion->performEffect(entity, amplifier);
}

jstring PotionEffect::getEffectName() const
{
	Potion *potion = Potion::getPotion(potionID);
	return potion != nullptr ? potion->getName() : jstring();
}

bool PotionEffect::equals(const PotionEffect &other) const
{
	return potionID == other.potionID && amplifier == other.amplifier && duration == other.duration;
}
