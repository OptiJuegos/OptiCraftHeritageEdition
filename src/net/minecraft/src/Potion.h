#pragma once

#include "java/String.h"

class EntityLiving;
class PotionEffect;

// net.minecraft.src.Potion
class Potion
{
public:
	Potion(int id, bool badEffect, int liquidColor);
	virtual ~Potion() = default;

	static void initPotions();
	static Potion *getPotion(int id);

	Potion *setIconIndex(int x, int y);
	int getId() const;
	virtual void performEffect(EntityLiving *entity, int amplifier);
	virtual void affectEntity(EntityLiving *source, EntityLiving *target, int amplifier, double strength);
	virtual bool isInstant() const;
	virtual bool isReady(int duration, int amplifier) const;
	Potion *setPotionName(const jstring &name);
	const jstring &getName() const;
	bool hasStatusIcon() const;
	int getStatusIconIndex() const;
	bool getIsBadEffect() const;
	static jstring getDurationString(const PotionEffect &effect);
	Potion *setEffectiveness(double effectiveness);
	double getEffectiveness() const;
	Potion *setPotionUnusable();
	bool isUsable() const;
	int getLiquidColor() const;

	static constexpr int POTION_TYPE_COUNT = 32;
	static Potion *potionTypes[POTION_TYPE_COUNT];
	static Potion *moveSpeed;
	static Potion *moveSlowdown;
	static Potion *digSpeed;
	static Potion *digSlowdown;
	static Potion *damageBoost;
	static Potion *heal;
	static Potion *harm;
	static Potion *jump;
	static Potion *confusion;
	static Potion *regeneration;
	static Potion *resistance;
	static Potion *fireResistance;
	static Potion *waterBreathing;
	static Potion *invisibility;
	static Potion *blindness;
	static Potion *nightVision;
	static Potion *hunger;
	static Potion *weakness;
	static Potion *poison;

	const int id;

protected:
	jstring name;
	int statusIconIndex;
	bool badEffect;
	double effectiveness;
	bool usable;
	int liquidColor;
};
