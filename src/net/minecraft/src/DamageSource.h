#pragma once

#include "java/String.h"

class Entity;
class EntityLiving;
class EntityPlayer;
class EntityArrow;
class EntityFireball;

// net.minecraft.src.DamageSource
class DamageSource
{
public:
	explicit DamageSource(const jstring &type);
	virtual ~DamageSource() = default;

	static DamageSource causeMobDamage(EntityLiving *entity);
	static DamageSource causePlayerDamage(EntityPlayer *player);
	static DamageSource causeArrowDamage(EntityArrow *arrow, Entity *shooter);
	static DamageSource causeFireballDamage(EntityFireball *fireball, Entity *shooter);
	static DamageSource causeThrownDamage(Entity *projectile, Entity *thrower);
	static DamageSource causeIndirectMagicDamage(Entity *source, Entity *indirectEntity);

	bool isProjectile() const;
	DamageSource &setProjectile();
	bool isUnblockable() const;
	float getHungerDamage() const;
	bool canHarmInCreative() const;
	virtual Entity *getSourceOfDamage() const;
	virtual Entity *getEntity() const;
	DamageSource &setDamageBypassesArmor();
	DamageSource &setDamageAllowedInCreativeMode();
	DamageSource &setFireDamage();
	bool fireDamage() const;
	const jstring &getDamageType() const;

	static DamageSource inFire;
	static DamageSource onFire;
	static DamageSource lava;
	static DamageSource inWall;
	static DamageSource drown;
	static DamageSource starve;
	static DamageSource cactus;
	static DamageSource fall;
	static DamageSource outOfWorld;
	static DamageSource generic;
	static DamageSource explosion;
	static DamageSource magic;

protected:
	DamageSource(const jstring &type, Entity *directSource, Entity *responsibleEntity);

	Entity *directSource;
	Entity *responsibleEntity;

private:
	bool unblockable;
	bool damageAllowedInCreativeMode;
	float hungerDamage;
	bool fire;
	bool projectile;
	jstring damageType;
};
