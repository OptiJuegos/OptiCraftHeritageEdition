#include "Potion.h"
#include "java/Arithmetic.h"

#include <algorithm>
#include <sstream>

#include "DamageSource.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "PotionEffect.h"
#include "PotionHealth.h"

Potion *Potion::potionTypes[Potion::POTION_TYPE_COUNT] = {};
Potion *Potion::moveSpeed = nullptr;
Potion *Potion::moveSlowdown = nullptr;
Potion *Potion::digSpeed = nullptr;
Potion *Potion::digSlowdown = nullptr;
Potion *Potion::damageBoost = nullptr;
Potion *Potion::heal = nullptr;
Potion *Potion::harm = nullptr;
Potion *Potion::jump = nullptr;
Potion *Potion::confusion = nullptr;
Potion *Potion::regeneration = nullptr;
Potion *Potion::resistance = nullptr;
Potion *Potion::fireResistance = nullptr;
Potion *Potion::waterBreathing = nullptr;
Potion *Potion::invisibility = nullptr;
Potion *Potion::blindness = nullptr;
Potion *Potion::nightVision = nullptr;
Potion *Potion::hunger = nullptr;
Potion *Potion::weakness = nullptr;
Potion *Potion::poison = nullptr;

Potion::Potion(int potionId, bool isBadEffect, int color) :
	id(potionId), name(), statusIconIndex(-1), badEffect(isBadEffect),
	effectiveness(isBadEffect ? 0.5 : 1.0), usable(false), liquidColor(color)
{
	if (id >= 0 && id < POTION_TYPE_COUNT)
		potionTypes[id] = this;
}

void Potion::initPotions()
{
	if (moveSpeed != nullptr)
		return;

	moveSpeed = (new Potion(1, false, 8171462))->setPotionName("potion.moveSpeed")->setIconIndex(0, 0);
	moveSlowdown = (new Potion(2, true, 5926017))->setPotionName("potion.moveSlowdown")->setIconIndex(1, 0);
	digSpeed = (new Potion(3, false, 14270531))->setPotionName("potion.digSpeed")->setIconIndex(2, 0)->setEffectiveness(1.5);
	digSlowdown = (new Potion(4, true, 4866583))->setPotionName("potion.digSlowDown")->setIconIndex(3, 0);
	damageBoost = (new Potion(5, false, 9643043))->setPotionName("potion.damageBoost")->setIconIndex(4, 0);
	heal = (new PotionHealth(6, false, 16262179))->setPotionName("potion.heal");
	harm = (new PotionHealth(7, true, 4393481))->setPotionName("potion.harm");
	jump = (new Potion(8, false, 7889559))->setPotionName("potion.jump")->setIconIndex(2, 1);
	confusion = (new Potion(9, true, 5578058))->setPotionName("potion.confusion")->setIconIndex(3, 1)->setEffectiveness(0.25);
	regeneration = (new Potion(10, false, 13458603))->setPotionName("potion.regeneration")->setIconIndex(7, 0)->setEffectiveness(0.25);
	resistance = (new Potion(11, false, 10044730))->setPotionName("potion.resistance")->setIconIndex(6, 1);
	fireResistance = (new Potion(12, false, 14981690))->setPotionName("potion.fireResistance")->setIconIndex(7, 1);
	waterBreathing = (new Potion(13, false, 3035801))->setPotionName("potion.waterBreathing")->setIconIndex(0, 2);
	invisibility = (new Potion(14, false, 8356754))->setPotionName("potion.invisibility")->setIconIndex(0, 1)->setPotionUnusable();
	blindness = (new Potion(15, true, 2039587))->setPotionName("potion.blindness")->setIconIndex(5, 1)->setEffectiveness(0.25);
	nightVision = (new Potion(16, false, 2039713))->setPotionName("potion.nightVision")->setIconIndex(4, 1)->setPotionUnusable();
	hunger = (new Potion(17, true, 5797459))->setPotionName("potion.hunger")->setIconIndex(1, 1);
	weakness = (new Potion(18, true, 4738376))->setPotionName("potion.weakness")->setIconIndex(5, 0);
	poison = (new Potion(19, true, 5149489))->setPotionName("potion.poison")->setIconIndex(6, 0)->setEffectiveness(0.25);
}

Potion *Potion::getPotion(int potionId)
{
	initPotions();
	return potionId >= 0 && potionId < POTION_TYPE_COUNT ? potionTypes[potionId] : nullptr;
}

Potion *Potion::setIconIndex(int x, int y)
{
	statusIconIndex = x + y * 8;
	return this;
}

int Potion::getId() const
{
	return id;
}

void Potion::performEffect(EntityLiving *entity, int amplifier)
{
	if (entity == nullptr)
		return;
	initPotions();

	if (id == regeneration->id)
	{
		if (entity->getHealth() < entity->getMaxHealth())
			entity->heal(1);
	}
	else if (id == poison->id)
	{
		if (entity->getHealth() > 1)
			entity->attackEntityFrom(DamageSource::magic, 1);
	}
	else if (id == hunger->id && entity->isPlayer())
	{
		static_cast<EntityPlayer *>(entity)->addExhaustion(0.025f * (float)(amplifier + 1));
	}
	else if ((id != heal->id || entity->isEntityUndead()) && (id != harm->id || !entity->isEntityUndead()))
	{
		if ((id == harm->id && !entity->isEntityUndead()) || (id == heal->id && entity->isEntityUndead()))
			entity->attackEntityFrom(DamageSource::magic, JavaArithmetic::intShl(6, amplifier));
	}
	else
	{
		entity->heal(JavaArithmetic::intShl(6, amplifier));
	}
}

void Potion::affectEntity(EntityLiving *source, EntityLiving *target, int amplifier, double strength)
{
	if (target == nullptr)
		return;
	initPotions();

	int amount;
	if ((id != heal->id || target->isEntityUndead()) && (id != harm->id || !target->isEntityUndead()))
	{
		if ((id == harm->id && !target->isEntityUndead()) || (id == heal->id && target->isEntityUndead()))
		{
			amount = JavaArithmetic::doubleToInt(strength * (double)(JavaArithmetic::intShl(6, amplifier)) + 0.5);
			if (source == nullptr)
				target->attackEntityFrom(DamageSource::magic, amount);
			else
				target->attackEntityFrom(DamageSource::causeIndirectMagicDamage(target, source), amount);
		}
	}
	else
	{
		amount = JavaArithmetic::doubleToInt(strength * (double)(JavaArithmetic::intShl(6, amplifier)) + 0.5);
		target->heal(amount);
	}
}

bool Potion::isInstant() const
{
	return false;
}

bool Potion::isReady(int duration, int amplifier) const
{
	if (id != 10 && id != 19)
		return id == 17;
	int interval = JavaArithmetic::intShr(25, amplifier);
	return interval > 0 ? duration % interval == 0 : true;
}

Potion *Potion::setPotionName(const jstring &potionName)
{
	name = potionName;
	return this;
}

const jstring &Potion::getName() const
{
	return name;
}

bool Potion::hasStatusIcon() const
{
	return statusIconIndex >= 0;
}

int Potion::getStatusIconIndex() const
{
	return statusIconIndex;
}

bool Potion::getIsBadEffect() const
{
	return badEffect;
}

jstring Potion::getDurationString(const PotionEffect &effect)
{
	int seconds = effect.getDuration() / 20;
	int minutes = seconds / 60;
	seconds %= 60;
	std::ostringstream stream;
	stream << minutes << ':';
	if (seconds < 10)
		stream << '0';
	stream << seconds;
	return stream.str();
}

Potion *Potion::setEffectiveness(double value)
{
	effectiveness = value;
	return this;
}

double Potion::getEffectiveness() const
{
	return effectiveness;
}

Potion *Potion::setPotionUnusable()
{
	usable = true;
	return this;
}

bool Potion::isUsable() const
{
	return usable;
}

int Potion::getLiquidColor() const
{
	return liquidColor;
}
