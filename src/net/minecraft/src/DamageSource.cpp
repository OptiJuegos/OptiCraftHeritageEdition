#include "DamageSource.h"

#include "Entity.h"
#include "EntityArrow.h"
#include "EntityFireball.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"

DamageSource DamageSource::inFire = DamageSource("inFire").setFireDamage();
DamageSource DamageSource::onFire = DamageSource("onFire").setDamageBypassesArmor().setFireDamage();
DamageSource DamageSource::lava = DamageSource("lava").setFireDamage();
DamageSource DamageSource::inWall = DamageSource("inWall").setDamageBypassesArmor();
DamageSource DamageSource::drown = DamageSource("drown").setDamageBypassesArmor();
DamageSource DamageSource::starve = DamageSource("starve").setDamageBypassesArmor();
DamageSource DamageSource::cactus("cactus");
DamageSource DamageSource::fall = DamageSource("fall").setDamageBypassesArmor();
DamageSource DamageSource::outOfWorld = DamageSource("outOfWorld").setDamageBypassesArmor().setDamageAllowedInCreativeMode();
DamageSource DamageSource::generic = DamageSource("generic").setDamageBypassesArmor();
DamageSource DamageSource::explosion("explosion");
DamageSource DamageSource::magic = DamageSource("magic").setDamageBypassesArmor();

DamageSource::DamageSource(const jstring &type)
	: DamageSource(type, nullptr, nullptr)
{
}

DamageSource::DamageSource(const jstring &type, Entity *direct, Entity *responsible)
	: directSource(direct), responsibleEntity(responsible), unblockable(false), damageAllowedInCreativeMode(false),
	  hungerDamage(0.3f), fire(false), projectile(false), damageType(type)
{
}

DamageSource DamageSource::causeMobDamage(EntityLiving *entity)
{
	return DamageSource("mob", entity, entity);
}

DamageSource DamageSource::causePlayerDamage(EntityPlayer *player)
{
	return DamageSource("player", player, player);
}

DamageSource DamageSource::causeArrowDamage(EntityArrow *arrow, Entity *shooter)
{
	DamageSource source("arrow", arrow, shooter);
	return source.setProjectile();
}

DamageSource DamageSource::causeFireballDamage(EntityFireball *fireball, Entity *shooter)
{
	DamageSource source("fireball", fireball, shooter);
	return source.setFireDamage().setProjectile();
}

DamageSource DamageSource::causeThrownDamage(Entity *projectileEntity, Entity *thrower)
{
	DamageSource source("thrown", projectileEntity, thrower);
	return source.setProjectile();
}

DamageSource DamageSource::causeIndirectMagicDamage(Entity *sourceEntity, Entity *indirectEntity)
{
	DamageSource source("indirectMagic", sourceEntity, indirectEntity);
	return source.setDamageBypassesArmor();
}

bool DamageSource::isProjectile() const
{
	return projectile;
}

DamageSource &DamageSource::setProjectile()
{
	projectile = true;
	return *this;
}

bool DamageSource::isUnblockable() const
{
	return unblockable;
}

float DamageSource::getHungerDamage() const
{
	return hungerDamage;
}

bool DamageSource::canHarmInCreative() const
{
	return damageAllowedInCreativeMode;
}

Entity *DamageSource::getSourceOfDamage() const
{
	return directSource != nullptr ? directSource : getEntity();
}

Entity *DamageSource::getEntity() const
{
	return responsibleEntity;
}

DamageSource &DamageSource::setDamageBypassesArmor()
{
	unblockable = true;
	hungerDamage = 0.0f;
	return *this;
}

DamageSource &DamageSource::setDamageAllowedInCreativeMode()
{
	damageAllowedInCreativeMode = true;
	return *this;
}

DamageSource &DamageSource::setFireDamage()
{
	fire = true;
	return *this;
}

bool DamageSource::fireDamage() const
{
	return fire;
}

const jstring &DamageSource::getDamageType() const
{
	return damageType;
}
