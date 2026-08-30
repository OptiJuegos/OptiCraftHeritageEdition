#include "PotionHealth.h"

PotionHealth::PotionHealth(int id, bool badEffect, int liquidColor) :
	Potion(id, badEffect, liquidColor)
{
}

bool PotionHealth::isInstant() const
{
	return true;
}

bool PotionHealth::isReady(int duration, int) const
{
	return duration >= 1;
}
