#pragma once

#include "Potion.h"

// net.minecraft.src.PotionHealth
class PotionHealth : public Potion
{
public:
	PotionHealth(int id, bool badEffect, int liquidColor);
	bool isInstant() const override;
	bool isReady(int duration, int amplifier) const override;
};
