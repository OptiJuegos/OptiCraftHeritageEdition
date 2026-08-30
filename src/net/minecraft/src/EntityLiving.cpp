#include "EntityLiving.h"

#include <cmath>
#include <cstdlib>
#include <random>
#include <vector>

#include "AxisAlignedBB.h"
#include "Block.h"
#include "BlockVine.h"
#include "DamageSource.h"
#include "DataWatcher.h"
#include "EntityPlayer.h"
#include "EntityWolf.h"
#include "EnchantmentHelper.h"
#include "EntityGhast.h"
#include "EntityCreeper.h"
#include "EntityXPOrb.h"
#include "EntityBodyHelper.h"
#include "EntityJumpHelper.h"
#include "EntityLookHelper.h"
#include "EntityMoveHelper.h"
#include "EntitySenses.h"
#include "PathNavigate.h"
#include "EnumCreatureAttribute.h"
#include "NBTTagList.h"
#include "PotionEffect.h"
#include "Potion.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "java/Math.h"
#include "java/Arithmetic.h"
#include "Material.h"
#include "MovingObjectPosition.h"
#include "platform/PlatformTuning.h"
#include "NBTTagCompound.h"
#include "StepSound.h"
#include "Vec3D.h"
#include "World.h"


#if PLATFORM_PS2
namespace
{
bool isEntityPushCollisionRelevant(const EntityLiving *entity)
{
    if (entity == nullptr || entity->worldObj == nullptr || entity->worldObj->playerEntities.empty())
    {
        return true;
    }

    const EntityPlayer *player = entity->worldObj->playerEntities[0];
    const float dx = (float)(entity->posX - player->posX);
    const float dz = (float)(entity->posZ - player->posZ);
    const float radius = PLATFORM_ENTITY_PUSH_COLLISION_RADIUS_BLOCKS;
    return dx * dx + dz * dz <= radius * radius;
}
}
#endif

EntityLiving::EntityLiving(World *world)
	: Entity(world)
{
	heartsHalvesLife = 20;
	renderYawOffset = 0.0f;
	prevRenderYawOffset = 0.0f;
	rotationYawHead = 0.0f;
	prevRotationYawHead = 0.0f;
	field_9362_u = 0.0f;
	field_9361_v = 0.0f;
	field_9360_w = 0.0f;
	field_9359_x = 0.0f;
	texture = "/mob/char.png";
	field_9353_B = 0.0f;
	field_9351_C = jstring();
	scoreValue = 0;
	experienceValue = 0;
	isMultiplayerEntity = false;
	prevSwingProgress = 0.0f;
	swingProgress = 0.0f;
	livingSoundTime = 0;
	hurtTime = 0;
	maxHurtTime = 0;
	attackedAtYaw = 0.0f;
	deathTime = 0;
	attackTime = 0;
	arrowHitTempCounter = 0;
	arrowHitTimer = 0;
	cameraPitch = 0.0f;
	field_9328_R = 0.0f;
	field_705_Q = 0.0f;
	field_704_R = 0.0f;
	field_703_S = 0.0f;
	newPosRotationIncrements = 0;
	newPosX = newPosY = newPosZ = 0.0;
	newRotationYaw = newRotationPitch = 0.0;
	field_9346_af = 0;
	entityAge = 0;
	isJumping = false;
	jumpTicks = 0;
	moveStrafing = 0.0f;
	moveForward = 0.0f;
	landMovementFactor = 0.1f;
	jumpMovementFactor = 0.02f;
	randomYawVelocity = 0.0f;
	defaultPitch = 0.0f;
	moveSpeed = 0.7f;
	lookHelper = new EntityLookHelper(this);
	moveHelper = new EntityMoveHelper(this);
	jumpHelper = new EntityJumpHelper(this);
	bodyHelper = new EntityBodyHelper(this);
	navigator = new PathNavigate(this, world, 16.0f);
	entitySenses = new EntitySenses(this);
	aiMoveSpeed = 0.0f;
	homePosition = ChunkCoordinates(0, 0, 0);
	maximumHomeDistance = -1.0f;
	attackTargetEntityId = -1;
	lastAttackingEntityId = -1;
	revengeTargetEntityId = -1;
	revengeTimer = 0;
	attackingPlayerEntityId = -1;
	recentlyHit = 0;
	carryoverDamage = 0;
	potionsNeedUpdate = true;
	currentTarget = nullptr;
	numTicksToChaseTarget = 0;
#if PLATFORM_CACHE_NEAREST_PLAYER
	cachedNearestPlayer = nullptr;
	cachedNearestPlayerTick = -1;
	cachedNearestPlayerDistanceSq = 0.0f;
#endif
	health = 20;
	prevHealth = 0;
	preventEntitySpawning = true;
	if (!dataWatcher->hasObject(8))
		dataWatcher->addObject(8, (int_t)0);
	setPosition(posX, posY, posZ);
	rotationYaw = (float)(Math::random() * 3.1415927410125732 * 2.0);
	rotationYawHead = rotationYaw;
	stepHeight = 0.5f;
}

EntityLiving::~EntityLiving()
{
	for (auto &entry : activePotionsMap)
		delete entry.second;
	activePotionsMap.clear();
	activePotionOrder.clear();
	delete lookHelper;
	delete moveHelper;
	delete jumpHelper;
	delete bodyHelper;
	delete navigator;
	delete entitySenses;
}

void EntityLiving::entityInit()
{
	if (!dataWatcher->hasObject(8))
		dataWatcher->addObject(8, (int_t)0);
}

bool EntityLiving::canEntityBeSeen(Entity *entity)
{
	MovingObjectPosition *mop = worldObj->rayTraceBlocks(
	    Vec3D::createVector(posX, posY + (double)getEyeHeight(), posZ),
	    Vec3D::createVector(entity->posX, entity->posY + (double)entity->getEyeHeight(), entity->posZ));
	bool clear = (mop == nullptr);
	delete mop;
	return clear;
}

const char *EntityLiving::getEntityTexture()
{
	return texture.c_str();
}

bool EntityLiving::canBeCollidedWith()
{
	return !isDead;
}

bool EntityLiving::canBePushed()
{
	return !isDead;
}

float EntityLiving::getEyeHeight()
{
	return height * 0.85f;
}

EntityLookHelper *EntityLiving::getLookHelper()
{
	return lookHelper;
}

EntityMoveHelper *EntityLiving::getMoveHelper()
{
	return moveHelper;
}

EntityJumpHelper *EntityLiving::getJumpHelper()
{
	return jumpHelper;
}

PathNavigate *EntityLiving::getNavigator()
{
	return navigator;
}

EntitySenses *EntityLiving::getEntitySenses()
{
	return entitySenses;
}

EntitySenses *EntityLiving::func_48090_aM()
{
	return getEntitySenses();
}

Random &EntityLiving::getRNG()
{
	return rand;
}

EntityLiving *EntityLiving::getAITarget()
{
	if (worldObj == nullptr || revengeTargetEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityLiving *>(worldObj->getEntityByID(revengeTargetEntityId));
}

EntityLiving *EntityLiving::getLastAttackingEntity()
{
	if (worldObj == nullptr || lastAttackingEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityLiving *>(worldObj->getEntityByID(lastAttackingEntityId));
}

void EntityLiving::setLastAttackingEntity(Entity *entity)
{
	EntityLiving *living = dynamic_cast<EntityLiving *>(entity);
	lastAttackingEntityId = living != nullptr ? living->entityId : -1;
}

EntityLiving *EntityLiving::getAttackTarget()
{
	if (worldObj == nullptr || attackTargetEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityLiving *>(worldObj->getEntityByID(attackTargetEntityId));
}

void EntityLiving::setAttackTarget(EntityLiving *target)
{
	attackTargetEntityId = target != nullptr ? target->entityId : -1;
}

void EntityLiving::setRevengeTarget(EntityLiving *target)
{
	revengeTargetEntityId = target != nullptr ? target->entityId : -1;
	revengeTimer = target != nullptr ? 60 : 0;
}

int_t EntityLiving::getRevengeTimer() const
{
	return revengeTimer;
}

int_t EntityLiving::getAge() const
{
	return entityAge;
}

void EntityLiving::setMoveForward(float value)
{
	moveForward = value;
}

void EntityLiving::setJumping(bool value)
{
	isJumping = value;
}

void EntityLiving::setAIMoveSpeed(float value)
{
	aiMoveSpeed = value;
	moveForward = value;
}

float EntityLiving::getAIMoveSpeed() const
{
	return aiMoveSpeed;
}

void EntityLiving::func_48098_g(float value)
{
	setAIMoveSpeed(value);
}

float EntityLiving::func_48101_aR() const
{
	return getAIMoveSpeed();
}

bool EntityLiving::isAIEnabled()
{
	return false;
}

bool EntityLiving::isChild()
{
	return false;
}

bool EntityLiving::isBlocking()
{
	return false;
}

float EntityLiving::getRenderSizeModifier()
{
	return 1.0f;
}

std::vector<PotionEffect *> EntityLiving::getActivePotionEffects() const
{
	std::vector<PotionEffect *> effects;
	effects.reserve(activePotionsMap.size());
	const std::vector<int_t> potionIds = activePotionOrder.valuesInIterationOrder();
	for (int_t potionId : potionIds)
	{
		auto it = activePotionsMap.find(potionId);
		if (it != activePotionsMap.end() && it->second != nullptr)
			effects.push_back(it->second);
	}
	return effects;
}

void EntityLiving::renderBrokenItemStack(ItemStack *itemstack)
{
	if (worldObj == nullptr || itemstack == nullptr || itemstack->getItem() == nullptr)
		return;

	worldObj->playSoundAtEntity(this, "random.break", 0.8f, 0.8f + worldObj->rand.nextFloat() * 0.4f);
	for (int_t i = 0; i < 5; ++i)
	{
		Vec3D *motion = Vec3D::createVector(((double)rand.nextFloat() - 0.5) * 0.1,
			Math::random() * 0.1 + 0.1, 0.0);
		motion->rotateAroundX(-rotationPitch * 3.1415927410125732f / 180.0f);
		motion->rotateAroundY(-rotationYaw * 3.1415927410125732f / 180.0f);

		const float positionRandomX = rand.nextFloat();
		const float positionRandomY = rand.nextFloat();
		Vec3D *position = Vec3D::createVector(((double)positionRandomX - 0.5) * 0.3,
			(double)(-positionRandomY) * 0.6 - 0.3, 0.6);
		position->rotateAroundX(-rotationPitch * 3.1415927410125732f / 180.0f);
		position->rotateAroundY(-rotationYaw * 3.1415927410125732f / 180.0f);
		position = position->addVector(posX, posY + (double)getEyeHeight(), posZ);

		worldObj->spawnParticle("iconcrack_" + String::toString(itemstack->getItem()->shiftedIndex),
			position->xCoord, position->yCoord, position->zCoord,
			motion->xCoord, motion->yCoord + 0.05, motion->zCoord);
	}
}

void EntityLiving::eatGrassBonus()
{
}

bool EntityLiving::attackEntityAsMob(Entity *target)
{
	setLastAttackingEntity(target);
	return false;
}

bool EntityLiving::canAttackEntity(EntityLiving *target)
{
	return target != nullptr && dynamic_cast<EntityCreeper *>(target) == nullptr && dynamic_cast<EntityGhast *>(target) == nullptr;
}

bool EntityLiving::func_48100_a(const std::type_info &type) const
{
	return type != typeid(EntityCreeper) && type != typeid(EntityGhast);
}

bool EntityLiving::isWithinHomeDistanceCurrentPosition() const
{
	return isWithinHomeDistance(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ));
}

bool EntityLiving::isWithinHomeDistance(int_t x, int_t y, int_t z) const
{
	if (maximumHomeDistance == -1.0f)
		return true;
	return homePosition.getDistanceSquared(x, y, z) < maximumHomeDistance * maximumHomeDistance;
}

void EntityLiving::setHomeArea(int_t x, int_t y, int_t z, int_t radius)
{
	homePosition.set(x, y, z);
	maximumHomeDistance = (float)radius;
}

ChunkCoordinates EntityLiving::getHomePosition() const
{
	return ChunkCoordinates(&homePosition);
}

float EntityLiving::getMaximumHomeDistance() const
{
	return maximumHomeDistance;
}

void EntityLiving::detachHome()
{
	maximumHomeDistance = -1.0f;
}

bool EntityLiving::hasHome() const
{
	return maximumHomeDistance != -1.0f;
}

int_t EntityLiving::getHealth() const
{
	return health;
}

int_t EntityLiving::getMaxHealth() const
{
	return 20;
}

int_t EntityLiving::getTotalArmorValue() const
{
	return 0;
}

void EntityLiving::setHealth(int_t value)
{
	health = value;
	if (health > getMaxHealth())
		health = getMaxHealth();
}

void EntityLiving::setEntityHealth(int_t value)
{
	health = value;
}

EnumCreatureAttribute EntityLiving::getCreatureAttribute() const
{
	return EnumCreatureAttribute::UNDEFINED;
}

bool EntityLiving::isEntityUndead() const
{
	return getCreatureAttribute() == EnumCreatureAttribute::UNDEAD;
}

bool EntityLiving::isPotionActive(Potion *potion) const
{
	if (potion == nullptr)
		return false;
	return activePotionsMap.find(potion->id) != activePotionsMap.end();
}

PotionEffect *EntityLiving::getActivePotionEffect(Potion *potion) const
{
	if (potion == nullptr)
		return nullptr;
	auto it = activePotionsMap.find(potion->id);
	return it != activePotionsMap.end() ? it->second : nullptr;
}

void EntityLiving::addPotionEffect(PotionEffect *effect)
{
	if (effect == nullptr)
		return;
	if (!isPotionApplicable(effect))
	{
		delete effect;
		return;
	}

	int_t id = effect->getPotionID();
	auto it = activePotionsMap.find(id);
	if (it != activePotionsMap.end())
	{
		it->second->combine(*effect);
		delete effect;
		onChangedPotionEffect(it->second);
	}
	else
	{
		activePotionsMap[id] = effect;
		activePotionOrder.add(id);
		onNewPotionEffect(effect);
	}
}

bool EntityLiving::isPotionApplicable(PotionEffect *effect) const
{
	if (effect == nullptr)
		return false;
	Potion::initPotions();
	if (getCreatureAttribute() == EnumCreatureAttribute::UNDEAD)
	{
		int_t id = effect->getPotionID();
		if ((Potion::regeneration != nullptr && id == Potion::regeneration->id) ||
		    (Potion::poison != nullptr && id == Potion::poison->id))
			return false;
	}
	return true;
}

void EntityLiving::removePotionEffect(int_t potionId)
{
	auto it = activePotionsMap.find(potionId);
	if (it == activePotionsMap.end())
		return;
	PotionEffect *effect = it->second;
	activePotionsMap.erase(it);
	activePotionOrder.remove(potionId);
	onFinishedPotionEffect(effect);
	delete effect;
}

void EntityLiving::clearActivePotions()
{
	if (worldObj != nullptr && worldObj->multiplayerWorld)
		return;
	const std::vector<int_t> potionIds = activePotionOrder.valuesInIterationOrder();
	for (int_t potionId : potionIds)
	{
		auto it = activePotionsMap.find(potionId);
		if (it == activePotionsMap.end())
			continue;
		onFinishedPotionEffect(it->second);
		delete it->second;
	}
	activePotionsMap.clear();
	activePotionOrder.clear();
}

int_t EntityLiving::getTalkInterval()
{
	return 80;
}

void EntityLiving::playLivingSound()
{
	jstring s = getLivingSound();
	if (!s.empty())
	{
		worldObj->playSoundAtEntity(this, s, getSoundVolume(), getSoundPitch());
	}
}

void EntityLiving::onEntityUpdate()
{
	prevSwingProgress = swingProgress;
	Entity::onEntityUpdate();
	if (isEntityAlive() && rand.nextInt(1000) < livingSoundTime++)
	{
		livingSoundTime = -getTalkInterval();
		playLivingSound();
	}
	if (isEntityAlive() && isEntityInsideOpaqueBlock())
	{
		attackEntityFrom(DamageSource::inWall, 1);
	}
	if (immuneToFire || worldObj->multiplayerWorld)
	{
		fire = 0;
	}
	Potion::initPotions();
	if (isEntityAlive() && isInsideOfMaterial(Material::water) && !canBreatheUnderwater() && !isPotionActive(Potion::waterBreathing))
	{
		int_t currentAir = decreaseAirSupply(getAir());
		setAir(currentAir);
		if (currentAir == -20)
		{
			setAir(0);
			for (int_t i = 0; i < 8; i++)
			{
				float f  = rand.nextFloatDifference();
				float f1 = rand.nextFloatDifference();
				float f2 = rand.nextFloatDifference();
				worldObj->spawnParticle("bubble", posX + (double)f, posY + (double)f1, posZ + (double)f2, motionX, motionY, motionZ);
			}
			attackEntityFrom(DamageSource::drown, 2);
		}
		fire = 0;
	}
	else
	{
		setAir(maxAir);
	}
	cameraPitch = field_9328_R;
	if (attackTime > 0)
	{
		attackTime--;
	}
	if (hurtTime > 0)
	{
		hurtTime--;
	}
	if (heartsLife > 0)
	{
		heartsLife--;
	}
	if (health <= 0)
		onDeathUpdate();
	if (recentlyHit > 0)
	{
		--recentlyHit;
	}
	else
	{
		attackingPlayerEntityId = -1;
	}

	EntityLiving *lastAttacker = getLastAttackingEntity();
	if (lastAttackingEntityId >= 0 && (lastAttacker == nullptr || !lastAttacker->isEntityAlive()))
	{
		lastAttackingEntityId = -1;
	}

	EntityLiving *revengeTarget = getAITarget();
	if (revengeTarget != nullptr)
	{
		if (!revengeTarget->isEntityAlive())
		{
			setRevengeTarget(nullptr);
		}
		else if (revengeTimer > 0)
		{
			--revengeTimer;
		}
		else
		{
			setRevengeTarget(nullptr);
		}
	}
	else if (revengeTargetEntityId >= 0 || revengeTimer > 0)
	{
		setRevengeTarget(nullptr);
	}

	updatePotionEffects();
	field_9359_x = field_9360_w;
	prevRenderYawOffset = renderYawOffset;
	prevRotationYawHead = rotationYawHead;
	prevRotationYaw = rotationYaw;
	prevRotationPitch = rotationPitch;
}

void EntityLiving::onDeathUpdate()
{
	++deathTime;
	if (deathTime == 20)
	{
		if (worldObj != nullptr && !worldObj->multiplayerWorld && (recentlyHit > 0 || isPlayer()) && !isChild())
		{
			EntityPlayer *attackingPlayer = nullptr;
			if (attackingPlayerEntityId >= 0)
				attackingPlayer = dynamic_cast<EntityPlayer *>(worldObj->getEntityByID(attackingPlayerEntityId));

			int_t experience = getExperiencePoints(attackingPlayer);
			while (experience > 0)
			{
				const int_t split = EntityXPOrb::getXPSplit(experience);
				experience -= split;
				EntityXPOrb *orb = new EntityXPOrb(worldObj, posX, posY, posZ, split);
				if (!worldObj->spawnEntityInWorld(orb))
					delete orb;
			}
		}

		onEntityDeath();
		setEntityDead();
		for (int_t i = 0; i < 20; ++i)
		{
			const double motionX = rand.nextGaussian() * 0.02;
			const double motionY = rand.nextGaussian() * 0.02;
			const double motionZ = rand.nextGaussian() * 0.02;
			const double particleX = posX + static_cast<double>(rand.nextFloat() * width * 2.0f) - static_cast<double>(width);
			const double particleY = posY + static_cast<double>(rand.nextFloat() * height);
			const double particleZ = posZ + static_cast<double>(rand.nextFloat() * width * 2.0f) - static_cast<double>(width);
			worldObj->spawnParticle("explode", particleX, particleY, particleZ, motionX, motionY, motionZ);
		}
	}
}

int_t EntityLiving::getExperiencePoints(EntityPlayer *player)
{
	(void)player;
	return experienceValue;
}

void EntityLiving::spawnExplosionParticle()
{
	for (int_t i = 0; i < 20; i++)
	{
		double d  = rand.nextGaussian() * 0.02;
		double d1 = rand.nextGaussian() * 0.02;
		double d2 = rand.nextGaussian() * 0.02;
		double d3 = 10.0;
		const double particleX = (posX + (double)(rand.nextFloat() * width * 2.0f)) - (double)width - d * d3;
		const double particleY = (posY + (double)(rand.nextFloat() * height)) - d1 * d3;
		const double particleZ = (posZ + (double)(rand.nextFloat() * width * 2.0f)) - (double)width - d2 * d3;
		worldObj->spawnParticle("explode", particleX, particleY, particleZ, d, d1, d2);
	}
}

void EntityLiving::updateRidden()
{
	Entity::updateRidden();
	field_9362_u = field_9361_v;
	field_9361_v = 0.0f;
	fallDistance = 0.0f;
}

void EntityLiving::setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i)
{
	yOffset = 0.0f;
	newPosX = d;
	newPosY = d1;
	newPosZ = d2;
	newRotationYaw = f;
	newRotationPitch = f1;
	newPosRotationIncrements = i;
}

void EntityLiving::onUpdate()
{
	Entity::onUpdate();
	if (arrowHitTempCounter > 0)
	{
		if (arrowHitTimer <= 0)
			arrowHitTimer = 60;
		--arrowHitTimer;
		if (arrowHitTimer <= 0)
			--arrowHitTempCounter;
	}

	onLivingUpdate();
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	const float d = (float)(posX - prevPosX);
	const float d1 = (float)(posZ - prevPosZ);
	float f = MathHelper::sqrt_float(d * d + d1 * d1);
#else
	double d  = posX - prevPosX;
	double d1 = posZ - prevPosZ;
	float f  = MathHelper::sqrt_double(d * d + d1 * d1);
#endif
	float f1 = renderYawOffset;
	float f2 = 0.0f;
	field_9362_u = field_9361_v;
	float f3 = 0.0f;
	if (f > 0.05f)
	{
		f3 = 1.0f;
		f2 = f * 3.0f;
#if PLATFORM_FLOAT_ENTITY_AI_MATH
		f1 = (std::atan2(d1, d) * 180.0f) / 3.1415927f - 90.0f;
#else
		f1 = ((float)JavaMath::atan2(d1, d) * 180.0f) / 3.1415927f - 90.0f;
#endif
	}
	if (swingProgress > 0.0f)
	{
		f1 = rotationYaw;
	}
	if (!onGround)
	{
		f3 = 0.0f;
	}
	field_9361_v = field_9361_v + (f3 - field_9361_v) * 0.3f;
	if (isAIEnabled())
	{
		bodyHelper->updateRenderAngles();
	}
	else
	{
		float f4;
		for (f4 = f1 - renderYawOffset; f4 < -180.0f; f4 += 360.0f) {}
		for (; f4 >= 180.0f; f4 -= 360.0f) {}
		renderYawOffset += f4 * 0.3f;
		float f5;
		for (f5 = rotationYaw - renderYawOffset; f5 < -180.0f; f5 += 360.0f) {}
		for (; f5 >= 180.0f; f5 -= 360.0f) {}
		bool reverse = f5 < -90.0f || f5 >= 90.0f;
		if (f5 < -75.0f) f5 = -75.0f;
		if (f5 >= 75.0f) f5 = 75.0f;
		renderYawOffset = rotationYaw - f5;
		if (f5 * f5 > 2500.0f)
			renderYawOffset += f5 * 0.2f;
		if (reverse)
			f2 *= -1.0f;
	}
	for (; rotationYaw   - prevRotationYaw   < -180.0f; prevRotationYaw   -= 360.0f) {}
	for (; rotationYaw   - prevRotationYaw   >= 180.0f; prevRotationYaw   += 360.0f) {}
	for (; renderYawOffset - prevRenderYawOffset < -180.0f; prevRenderYawOffset -= 360.0f) {}
	for (; renderYawOffset - prevRenderYawOffset >= 180.0f; prevRenderYawOffset += 360.0f) {}
	for (; rotationPitch - prevRotationPitch < -180.0f; prevRotationPitch -= 360.0f) {}
	for (; rotationPitch - prevRotationPitch >= 180.0f; prevRotationPitch += 360.0f) {}
	for (; rotationYawHead - prevRotationYawHead < -180.0f; prevRotationYawHead -= 360.0f) {}
	for (; rotationYawHead - prevRotationYawHead >= 180.0f; prevRotationYawHead += 360.0f) {}
	field_9360_w += f2;
}

void EntityLiving::setSize(float f, float f1)
{
	Entity::setSize(f, f1);
}

void EntityLiving::heal(int_t i)
{
	if (health <= 0)
	{
		return;
	}
	health = JavaArithmetic::intAdd(health, i);
	if (health > getMaxHealth())
		health = getMaxHealth();
	heartsLife = heartsHalvesLife / 2;
}

bool EntityLiving::attackEntityFrom(Entity *entity, int_t i)
{
	if (worldObj->multiplayerWorld)
	{
		return false;
	}
	entityAge = 0;
	if (health <= 0)
	{
		return false;
	}
	field_704_R = 1.5f;
	bool flag = true;
	if ((float)heartsLife > (float)heartsHalvesLife / 2.0f)
	{
		if (i <= field_9346_af)
		{
			return false;
		}
		damageEntity(i - field_9346_af);
		field_9346_af = i;
		flag = false;
	}
	else
	{
		field_9346_af = i;
		prevHealth = health;
		heartsLife = heartsHalvesLife;
		damageEntity(i);
		hurtTime = maxHurtTime = 10;
	}
	attackedAtYaw = 0.0f;
	if (EntityLiving *livingAttacker = dynamic_cast<EntityLiving *>(entity))
		setRevengeTarget(livingAttacker);
	if (flag)
	{
		worldObj->setEntityState(this, (unsigned char)2);
		setBeenAttacked();
		if (entity != nullptr)
		{
			double d = entity->posX - posX;
			double d1 = entity->posZ - posZ;
			while (d * d + d1 * d1 < 0.0001)
			{
				const double dFirst = Math::random();
				const double dSecond = Math::random();
				d = (dFirst - dSecond) * 0.01;
				const double d1First = Math::random();
				const double d1Second = Math::random();
				d1 = (d1First - d1Second) * 0.01;
			}
			#if PLATFORM_FLOAT_ENTITY_CORE_MATH
			attackedAtYaw = (std::atan2(static_cast<float>(d1), static_cast<float>(d)) * 180.0f) / 3.14159274f - rotationYaw;
#else
			attackedAtYaw = (float)((JavaMath::atan2(d1, d) * 180.0) / 3.1415927410125732) - rotationYaw;
#endif
			knockBack(entity, i, d, d1);
		}
		else
		{
			attackedAtYaw = (float)((int_t)(Math::random() * 2.0) * 180);
		}
	}
	if (health <= 0)
	{
		if (flag)
		{
			worldObj->playSoundAtEntity(this, getDeathSound(), getSoundVolume(), rand.nextFloatDifference() * 0.2f + 1.0f);
		}
		onDeath(entity);
	}
	else if (flag)
	{
		worldObj->playSoundAtEntity(this, getHurtSound(), getSoundVolume(), rand.nextFloatDifference() * 0.2f + 1.0f);
	}
	return true;
}

bool EntityLiving::attackEntityFrom(const DamageSource &source, int_t damage)
{
	Potion::initPotions();
	if (worldObj->multiplayerWorld)
		return false;
	entityAge = 0;
	if (health <= 0)
		return false;
	if (source.fireDamage() && isPotionActive(Potion::fireResistance))
		return false;

	field_704_R = 1.5f;
	bool fullHurt = true;
	if ((float)heartsLife > (float)heartsHalvesLife / 2.0f)
	{
		if (damage <= field_9346_af)
			return false;
		damageEntity(source, damage - field_9346_af);
		field_9346_af = damage;
		fullHurt = false;
	}
	else
	{
		field_9346_af = damage;
		prevHealth = health;
		heartsLife = heartsHalvesLife;
		damageEntity(source, damage);
		hurtTime = maxHurtTime = 10;
	}

	attackedAtYaw = 0.0f;
	Entity *attacker = source.getEntity();
	if (attacker != nullptr)
	{
		if (EntityLiving *living = dynamic_cast<EntityLiving *>(attacker))
			setRevengeTarget(living);
		if (EntityPlayer *player = dynamic_cast<EntityPlayer *>(attacker))
		{
			recentlyHit = 60;
			attackingPlayerEntityId = player->entityId;
		}
		else if (EntityWolf *wolf = dynamic_cast<EntityWolf *>(attacker))
		{
			if (wolf->isTamed())
			{
				recentlyHit = 60;
				attackingPlayerEntityId = -1;
			}
		}
	}

	if (fullHurt)
	{
		worldObj->setEntityState(this, (unsigned char)2);
		setBeenAttacked();
		if (attacker != nullptr)
		{
			double dx = attacker->posX - posX;
			double dz = attacker->posZ - posZ;
			while (dx * dx + dz * dz < 0.0001)
			{
				const double dxFirst = Math::random();
				const double dxSecond = Math::random();
				const double dzFirst = Math::random();
				const double dzSecond = Math::random();
				dx = (dxFirst - dxSecond) * 0.01;
				dz = (dzFirst - dzSecond) * 0.01;
			}
			#if PLATFORM_FLOAT_ENTITY_CORE_MATH
			attackedAtYaw = (std::atan2(static_cast<float>(dz), static_cast<float>(dx)) * 180.0f) / 3.14159274f - rotationYaw;
#else
			attackedAtYaw = (float)((JavaMath::atan2(dz, dx) * 180.0) / 3.1415927410125732) - rotationYaw;
#endif
			knockBack(attacker, damage, dx, dz);
		}
		else
		{
			attackedAtYaw = (float)((int_t)(Math::random() * 2.0) * 180);
		}
	}

	if (health <= 0)
	{
		if (fullHurt)
			worldObj->playSoundAtEntity(this, getDeathSound(), getSoundVolume(), getSoundPitch());
		onDeath(source);
	}
	else if (fullHurt)
	{
		worldObj->playSoundAtEntity(this, getHurtSound(), getSoundVolume(), getSoundPitch());
	}
	return true;
}

void EntityLiving::performHurtAnimation()
{
	hurtTime = maxHurtTime = 10;
	attackedAtYaw = 0.0f;
}

void EntityLiving::damageEntity(int_t i)
{
	health = JavaArithmetic::intSub(health, i);
}

void EntityLiving::damageArmor(int_t)
{
}

int_t EntityLiving::applyArmorCalculations(const DamageSource &source, int_t damage)
{
	if (!source.isUnblockable())
	{
		int_t armorFactor = JavaArithmetic::intSub(25, getTotalArmorValue());
		int_t scaledDamage = JavaArithmetic::intAdd(JavaArithmetic::intMul(damage, armorFactor), carryoverDamage);
		damageArmor(damage);
		damage = scaledDamage / 25;
		carryoverDamage = scaledDamage % 25;
	}
	return damage;
}

int_t EntityLiving::applyPotionDamageCalculations(const DamageSource &, int_t damage)
{
	Potion::initPotions();
	if (isPotionActive(Potion::resistance))
	{
		PotionEffect *effect = getActivePotionEffect(Potion::resistance);
		if (effect != nullptr)
		{
			int_t reduction = JavaArithmetic::intMul(JavaArithmetic::intAdd(effect->getAmplifier(), 1), 5);
			int_t scaledDamage = JavaArithmetic::intAdd(
				JavaArithmetic::intMul(damage, JavaArithmetic::intSub(25, reduction)), carryoverDamage);
			damage = scaledDamage / 25;
			carryoverDamage = scaledDamage % 25;
		}
	}
	return damage;
}

void EntityLiving::damageEntity(const DamageSource &source, int_t damage)
{
	damage = applyArmorCalculations(source, damage);
	damage = applyPotionDamageCalculations(source, damage);
	health = JavaArithmetic::intSub(health, damage);
}

float EntityLiving::getSoundVolume()
{
	return 1.0f;
}

float EntityLiving::getSoundPitch()
{
	return isChild() ? rand.nextFloatDifference() * 0.2f + 1.5f : rand.nextFloatDifference() * 0.2f + 1.0f;
}

jstring EntityLiving::getLivingSound()
{
	return jstring(nullptr);
}

jstring EntityLiving::getHurtSound()
{
	return "damage.hurtflesh";
}

jstring EntityLiving::getDeathSound()
{
	return "damage.hurtflesh";
}

void EntityLiving::knockBack(Entity *entity, int_t i, double d, double d1)
{
	isAirBorne = true;
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
	const float df = (float)d;
	const float d1f = (float)d1;
	const float f = MathHelper::sqrt_float(df * df + d1f * d1f);
#else
	float f = MathHelper::sqrt_double(d * d + d1 * d1);
#endif
	float f1 = 0.4f;
	motionX /= 2.0;
	motionY /= 2.0;
	motionZ /= 2.0;
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
	if (f > 0.0f)
	{
		motionX -= (double)((df / f) * f1);
		motionZ -= (double)((d1f / f) * f1);
	}
#else
	motionX -= (d  / (double)f) * (double)f1;
	motionZ -= (d1 / (double)f) * (double)f1;
#endif
	motionY += 0.40000000596046448;
	if (motionY > 0.40000000596046448)
	{
		motionY = 0.40000000596046448;
	}
}

void EntityLiving::onDeath(Entity *entity)
{
	if (scoreValue >= 0 && entity != nullptr)
	{
		entity->addToPlayerScore(this, scoreValue);
	}
	if (entity != nullptr)
	{
		entity->onKillEntity(this);
	}
	if (!worldObj->multiplayerWorld)
	{
		dropFewItems();
	}
	worldObj->setEntityState(this, (unsigned char)3);
}

void EntityLiving::onDeath(const DamageSource &source)
{
	Entity *attacker = source.getEntity();
	if (scoreValue >= 0 && attacker != nullptr)
		attacker->addToPlayerScore(this, scoreValue);
	if (attacker != nullptr)
		attacker->onKillEntity(this);

	if (!worldObj->multiplayerWorld)
	{
		int_t looting = 0;
		if (EntityPlayer *player = dynamic_cast<EntityPlayer *>(attacker))
			looting = EnchantmentHelper::getLootingModifier(player->inventory);
		if (!isChild())
		{
			dropFewItems(recentlyHit > 0, looting);
			if (recentlyHit > 0)
			{
				int_t rareRoll = rand.nextInt(200) - looting;
				if (rareRoll < 5)
					dropRareDrop(rareRoll <= 0 ? 1 : 0);
			}
		}
	}
	worldObj->setEntityState(this, (unsigned char)3);
}

void EntityLiving::dropFewItems()
{
	int_t i = getDropItemId();
	if (i > 0)
	{
		int_t j = rand.nextInt(3);
		for (int_t k = 0; k < j; k++)
		{
			dropItem(i, 1);
		}
	}
}

void EntityLiving::dropFewItems(bool, int_t lootingLevel)
{
	int_t itemId = getDropItemId();
	if (itemId <= 0)
		return;
	int_t count = rand.nextInt(3);
	if (lootingLevel > 0)
		count += rand.nextInt(lootingLevel + 1);
	for (int_t i = 0; i < count; ++i)
		dropItem(itemId, 1);
}

void EntityLiving::dropRareDrop(int_t)
{
}

int_t EntityLiving::getDropItemId()
{
	return 0;
}

void EntityLiving::fall(float f)
{
	Entity::fall(f);
	int_t i = JavaArithmetic::doubleToInt(std::ceil(static_cast<double>(f - 3.0f)));
	if (i > 0)
	{
		if (i > 4)
			worldObj->playSoundAtEntity(this, "damage.fallbig", 1.0f, 1.0f);
		else
			worldObj->playSoundAtEntity(this, "damage.fallsmall", 1.0f, 1.0f);
		attackEntityFrom(DamageSource::fall, i);
		int_t j = worldObj->getBlockId(MathHelper::floor_double(posX), MathHelper::floor_double(posY - 0.20000000298023224 - (double)yOffset), MathHelper::floor_double(posZ));
		if (j > 0)
		{
			StepSound *stepsound = Block::blocksList[j]->stepSound;
			worldObj->playSoundAtEntity(this, stepsound->getStepSound(), stepsound->getVolume() * 0.5f, stepsound->getPitch() * 0.75f);
		}
	}
}

void EntityLiving::moveEntityWithHeading(float f, float f1)
{
	if (isInWater())
	{
		double d = posY;
		moveFlying(f, f1, isAIEnabled() ? 0.04f : 0.02f);
		moveEntity(motionX, motionY, motionZ);
		motionX *= 0.80000001192092896;
		motionY *= 0.80000001192092896;
		motionZ *= 0.80000001192092896;
		motionY -= 0.02;
		if (isCollidedHorizontally && isOffsetPositionInLiquid(motionX, ((motionY + 0.60000002384185791) - posY) + d, motionZ))
		{
			motionY = 0.30000001192092896;
		}
	}
	else if (handleLavaMovement())
	{
		double d1 = posY;
		moveFlying(f, f1, 0.02f);
		moveEntity(motionX, motionY, motionZ);
		motionX *= 0.5;
		motionY *= 0.5;
		motionZ *= 0.5;
		motionY -= 0.02;
		if (isCollidedHorizontally && isOffsetPositionInLiquid(motionX, ((motionY + 0.60000002384185791) - posY) + d1, motionZ))
		{
			motionY = 0.30000001192092896;
		}
	}
	else
	{
		float f2 = 0.91f;
		if (onGround)
		{
			f2 = 546.0f * 0.1f * 0.1f * 0.1f;
			int_t i = worldObj->getBlockId(MathHelper::floor_double(posX), MathHelper::floor_double(boundingBox->minY) - 1, MathHelper::floor_double(posZ));
			if (i > 0)
			{
				f2 = Block::blocksList[i]->slipperiness * 0.91f;
			}
		}
		float f3 = 0.16277136f / (f2 * f2 * f2);
		float movementFactor = onGround ? (isAIEnabled() ? aiMoveSpeed : landMovementFactor) * f3 : jumpMovementFactor;
		moveFlying(f, f1, movementFactor);
		f2 = 0.91f;
		if (onGround)
		{
			f2 = 546.0f * 0.1f * 0.1f * 0.1f;
			int_t j = worldObj->getBlockId(MathHelper::floor_double(posX), MathHelper::floor_double(boundingBox->minY) - 1, MathHelper::floor_double(posZ));
			if (j > 0)
			{
				f2 = Block::blocksList[j]->slipperiness * 0.91f;
			}
		}
		if (isOnLadder())
		{
			float f4 = 0.15f;
			if (motionX < (double)(-f4)) { motionX = -f4; }
			if (motionX >  (double)f4)   { motionX =  f4; }
			if (motionZ < (double)(-f4)) { motionZ = -f4; }
			if (motionZ >  (double)f4)   { motionZ =  f4; }
			fallDistance = 0.0f;
			if (motionY < -0.14999999999999999)
			{
				motionY = -0.14999999999999999;
			}
			const bool sneakingPlayer = isSneaking() && dynamic_cast<EntityPlayer *>(this) != nullptr;
			if (sneakingPlayer && motionY < 0.0)
			{
				motionY = 0.0;
			}
		}
		moveEntity(motionX, motionY, motionZ);
		if (isCollidedHorizontally && isOnLadder())
		{
			motionY = 0.2;
		}
		motionY -= 0.08;
		motionY *= 0.98000001907348633;
		motionX *= f2;
		motionZ *= f2;
	}
	field_705_Q = field_704_R;
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	const float d2 = (float)(posX - prevPosX);
	const float d3 = (float)(posZ - prevPosZ);
	float f5 = MathHelper::sqrt_float(d2 * d2 + d3 * d3) * 4.0f;
#else
	double d2 = posX - prevPosX;
	double d3 = posZ - prevPosZ;
	float f5 = MathHelper::sqrt_double(d2 * d2 + d3 * d3) * 4.0f;
#endif
	if (f5 > 1.0f) { f5 = 1.0f; }
	field_704_R += (f5 - field_704_R) * 0.4f;
	field_703_S += field_704_R;
}

bool EntityLiving::isOnLadder()
{
	int_t i = MathHelper::floor_double(posX);
	int_t j = MathHelper::floor_double(boundingBox->minY);
	int_t k = MathHelper::floor_double(posZ);
	int_t blockId = worldObj->getBlockId(i, j, k);
	return blockId == Block::ladder->blockID || (Block::vine != nullptr && blockId == Block::vine->blockID);
}

void EntityLiving::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	nbttagcompound->setShort("Health", JavaArithmetic::shortFromBits(static_cast<ushort_t>(health)));
	nbttagcompound->setShort("HurtTime", JavaArithmetic::shortFromBits(static_cast<ushort_t>(hurtTime)));
	nbttagcompound->setShort("DeathTime", JavaArithmetic::shortFromBits(static_cast<ushort_t>(deathTime)));
	nbttagcompound->setShort("AttackTime", JavaArithmetic::shortFromBits(static_cast<ushort_t>(attackTime)));
	if (!activePotionsMap.empty())
	{
		NBTTagList *effects = new NBTTagList();
		const std::vector<int_t> potionIds = activePotionOrder.valuesInIterationOrder();
		for (int_t potionId : potionIds)
		{
			auto it = activePotionsMap.find(potionId);
			PotionEffect *effect = it != activePotionsMap.end() ? it->second : nullptr;
			if (effect == nullptr)
				continue;
			NBTTagCompound *tag = new NBTTagCompound();
			tag->setByte("Id", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(effect->getPotionID())));
			tag->setByte("Amplifier", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(effect->getAmplifier())));
			tag->setInteger("Duration", effect->getDuration());
			effects->appendTag(tag);
		}
		nbttagcompound->setTag("ActiveEffects", effects);
	}
}

void EntityLiving::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	health = nbttagcompound->getShort("Health");
	if (!nbttagcompound->hasKey("Health"))
		health = getMaxHealth();
	hurtTime   = nbttagcompound->getShort("HurtTime");
	deathTime  = nbttagcompound->getShort("DeathTime");
	attackTime = nbttagcompound->getShort("AttackTime");

	for (auto &entry : activePotionsMap)
		delete entry.second;
	activePotionsMap.clear();
	activePotionOrder.clear();
	if (nbttagcompound->hasKey("ActiveEffects"))
	{
		NBTTagList *effects = nbttagcompound->getTagList("ActiveEffects");
		if (effects != nullptr)
		{
			for (int_t i = 0; i < effects->tagCount(); ++i)
			{
				NBTTagCompound *tag = dynamic_cast<NBTTagCompound *>(effects->tagAt(i));
				if (tag == nullptr)
					continue;
				int_t id = static_cast<int_t>(tag->getByte("Id"));
				int_t amplifier = static_cast<int_t>(tag->getByte("Amplifier"));
				int_t duration = tag->getInteger("Duration");
				auto existing = activePotionsMap.find(id);
				if (existing != activePotionsMap.end())
				{
					delete existing->second;
					existing->second = new PotionEffect(id, duration, amplifier);
				}
				else
				{
					activePotionsMap[id] = new PotionEffect(id, duration, amplifier);
					activePotionOrder.add(id);
				}
			}
		}
	}
	potionsNeedUpdate = true;
}

bool EntityLiving::isEntityAlive()
{
	return !isDead && health > 0;
}

bool EntityLiving::canBreatheUnderwater()
{
	return false;
}

int_t EntityLiving::decreaseAirSupply(int_t airSupply)
{
	return airSupply - 1;
}

void EntityLiving::onLivingUpdate()
{
	if (jumpTicks > 0)
		--jumpTicks;

	if (newPosRotationIncrements > 0)
	{
		double d  = posX + (newPosX - posX) / (double)newPosRotationIncrements;
		double d1 = posY + (newPosY - posY) / (double)newPosRotationIncrements;
		double d2 = posZ + (newPosZ - posZ) / (double)newPosRotationIncrements;
		double d3;
		for (d3 = newRotationYaw - (double)rotationYaw; d3 < -180.0; d3 += 360.0) {}
		for (;                                          d3 >= 180.0; d3 -= 360.0) {}
		rotationYaw += (float)(d3 / (double)newPosRotationIncrements);
		rotationPitch += (float)((newRotationPitch - (double)rotationPitch) / (double)newPosRotationIncrements);
		newPosRotationIncrements--;
		setPosition(d, d1, d2);
		setRotation(rotationYaw, rotationPitch);
		std::vector<AxisAlignedBB *> list1 = worldObj->getCollidingBoundingBoxes(this, boundingBox->contract(0.03125, 0.0, 0.03125));
		if (list1.size() > 0)
		{
			double d4 = 0.0;
			for (size_t j = 0; j < list1.size(); j++)
			{
				AxisAlignedBB *axisalignedbb = list1[j];
				if (axisalignedbb->maxY > d4)
				{
					d4 = axisalignedbb->maxY;
				}
			}
			d1 += d4 - boundingBox->minY;
			setPosition(d, d1, d2);
		}
	}
	if (isMovementBlocked())
	{
		isJumping = false;
		moveStrafing = 0.0f;
		moveForward = 0.0f;
		randomYawVelocity = 0.0f;
	}
	else if (isClientWorld())
	{
		if (isAIEnabled())
		{
			updateAITasks();
		}
		else
		{
			updatePlayerActionState();
			rotationYawHead = rotationYaw;
		}
	}
	bool flag  = isInWater();
	bool flag1 = handleLavaMovement();
	if (isJumping)
	{
		if (flag)
		{
			motionY += 0.039999999105930328;
		}
		else if (flag1)
		{
			motionY += 0.039999999105930328;
		}
		else if (onGround && jumpTicks == 0)
		{
			jump();
			jumpTicks = 10;
		}
	}
	else
	{
		jumpTicks = 0;
	}
	moveStrafing *= 0.98f;
	moveForward *= 0.98f;
	randomYawVelocity *= 0.9f;
	const float previousLandMovementFactor = landMovementFactor;
	landMovementFactor *= getSpeedModifier();
	moveEntityWithHeading(moveStrafing, moveForward);
	landMovementFactor = previousLandMovementFactor;
#if PLATFORM_PS2
	if (!isEntityPushCollisionRelevant(this))
	{
		return;
	}
#endif
	const auto& list = worldObj->getEntitiesWithinAABBExcludingEntity(this, boundingBox->expand(0.20000000298023224, 0.0, 0.20000000298023224));
	if (list.size() > 0)
	{
		for (size_t i = 0; i < list.size(); i++)
		{
			Entity *entity = list[i];
			if (entity->canBePushed())
			{
				entity->applyEntityCollision(this);
			}
		}
	}
}

void EntityLiving::updatePotionEffects()
{
	const std::vector<int_t> potionIds = activePotionOrder.valuesInIterationOrder();
	for (int_t potionId : potionIds)
	{
		auto it = activePotionsMap.find(potionId);
		if (it == activePotionsMap.end())
			continue;
		PotionEffect *effect = it->second;
		if (effect == nullptr || (!effect->onUpdate(this) && !worldObj->multiplayerWorld))
		{
			if (effect != nullptr)
				onFinishedPotionEffect(effect);
			delete effect;
			activePotionsMap.erase(it);
			activePotionOrder.remove(potionId);
		}
	}

	if (potionsNeedUpdate)
	{
		if (!worldObj->multiplayerWorld)
		{
			int_t color = 0;
			if (!activePotionsMap.empty())
			{
				float red = 0.0f;
				float green = 0.0f;
				float blue = 0.0f;
				float weight = 0.0f;
				const std::vector<int_t> colorPotionIds = activePotionOrder.valuesInIterationOrder();
				for (int_t potionId : colorPotionIds)
				{
					auto it = activePotionsMap.find(potionId);
					PotionEffect *effect = it != activePotionsMap.end() ? it->second : nullptr;
					Potion *potion = effect != nullptr ? Potion::getPotion(effect->getPotionID()) : nullptr;
					if (potion == nullptr)
						continue;
					int_t liquid = potion->getLiquidColor();
					for (int_t i = 0; i <= effect->getAmplifier(); ++i)
					{
						red += (float)((liquid >> 16) & 255) / 255.0f;
						green += (float)((liquid >> 8) & 255) / 255.0f;
						blue += (float)(liquid & 255) / 255.0f;
						weight += 1.0f;
					}
				}
				if (weight > 0.0f)
				{
					int_t r = (int_t)(red / weight * 255.0f);
					int_t g = (int_t)(green / weight * 255.0f);
					int_t b = (int_t)(blue / weight * 255.0f);
					color = (r << 16) | (g << 8) | b;
				}
			}
			dataWatcher->updateObject(8, color);
		}
		potionsNeedUpdate = false;
	}

	if (rand.nextBoolean())
	{
		int_t color = dataWatcher->getWatchableObjectInt(8);
		if (color > 0)
		{
			double red = (double)((color >> 16) & 255) / 255.0;
			double green = (double)((color >> 8) & 255) / 255.0;
			double blue = (double)(color & 255) / 255.0;
			const double particleX = posX + (rand.nextDouble() - 0.5) * (double)width;
			const double particleY = posY + rand.nextDouble() * (double)height - (double)yOffset;
			const double particleZ = posZ + (rand.nextDouble() - 0.5) * (double)width;
			worldObj->spawnParticle("mobSpell", particleX, particleY, particleZ, red, green, blue);
		}
	}
}

void EntityLiving::onNewPotionEffect(PotionEffect *)
{
	potionsNeedUpdate = true;
}

void EntityLiving::onChangedPotionEffect(PotionEffect *)
{
	potionsNeedUpdate = true;
}

void EntityLiving::onFinishedPotionEffect(PotionEffect *)
{
	potionsNeedUpdate = true;
}

float EntityLiving::getSpeedModifier()
{
	Potion::initPotions();
	float modifier = 1.0f;
	PotionEffect *speed = getActivePotionEffect(Potion::moveSpeed);
	if (speed != nullptr)
		modifier *= 1.0f + 0.2f * (float)(speed->getAmplifier() + 1);
	PotionEffect *slow = getActivePotionEffect(Potion::moveSlowdown);
	if (slow != nullptr)
		modifier *= 1.0f - 0.15f * (float)(slow->getAmplifier() + 1);
	return modifier;
}

bool EntityLiving::isMovementBlocked()
{
	return health <= 0;
}

bool EntityLiving::isClientWorld() const
{
	return worldObj != nullptr && !worldObj->multiplayerWorld;
}

void EntityLiving::jump()
{
	motionY = 0.41999998688697815;
	Potion::initPotions();
	PotionEffect *jumpEffect = getActivePotionEffect(Potion::jump);
	if (jumpEffect != nullptr)
		motionY += (double)((float)(jumpEffect->getAmplifier() + 1) * 0.1f);

	if (isSprinting())
	{
		float yawRadians = rotationYaw * (3.1415927410125732f / 180.0f);
		motionX -= (double)(MathHelper::sin(yawRadians) * 0.2f);
		motionZ += (double)(MathHelper::cos(yawRadians) * 0.2f);
	}

	isAirBorne = true;
}

bool EntityLiving::canDespawn()
{
	return true;
}

#if PLATFORM_CACHE_NEAREST_PLAYER
void EntityLiving::refreshNearestPlayerCache()
{
	if (cachedNearestPlayerTick == ticksExisted)
	{
		return;
	}

	cachedNearestPlayerTick = ticksExisted;
	cachedNearestPlayer = nullptr;
	cachedNearestPlayerDistanceSq = 0.0f;
	if (worldObj == nullptr || worldObj->playerEntities.empty())
	{
		return;
	}

	cachedNearestPlayer = worldObj->playerEntities[0];
	const float dx = (float)(cachedNearestPlayer->posX - posX);
	const float dy = (float)(cachedNearestPlayer->posY - posY);
	const float dz = (float)(cachedNearestPlayer->posZ - posZ);
	cachedNearestPlayerDistanceSq = dx * dx + dy * dy + dz * dz;
}

EntityPlayer *EntityLiving::getCachedNearestPlayer()
{
	refreshNearestPlayerCache();
	return cachedNearestPlayer;
}

float EntityLiving::getCachedNearestPlayerDistanceSq()
{
	refreshNearestPlayerCache();
	return cachedNearestPlayerDistanceSq;
}
#endif

void EntityLiving::despawnEntity()
{
#if PLATFORM_CACHE_NEAREST_PLAYER
	EntityPlayer *entityplayer = getCachedNearestPlayer();
#else
	EntityPlayer *entityplayer = worldObj->getClosestPlayerToEntity(this, -1.0);
#endif
	if (entityplayer != nullptr)
	{
#if PLATFORM_FLOAT_ENTITY_AI_MATH
#if PLATFORM_CACHE_NEAREST_PLAYER
		const float d3 = getCachedNearestPlayerDistanceSq();
#else
		const float d = (float)(((Entity *)entityplayer)->posX - posX);
		const float d1 = (float)(((Entity *)entityplayer)->posY - posY);
		const float d2 = (float)(((Entity *)entityplayer)->posZ - posZ);
		const float d3 = d * d + d1 * d1 + d2 * d2;
#endif
#else
		double d  = ((Entity *)entityplayer)->posX - posX;
		double d1 = ((Entity *)entityplayer)->posY - posY;
		double d2 = ((Entity *)entityplayer)->posZ - posZ;
		double d3 = d * d + d1 * d1 + d2 * d2;
#endif
		if (canDespawn() && d3 > 16384.0)
		{
			setEntityDead();
		}
		if (entityAge > 600 && rand.nextInt(800) == 0 && d3 > 1024.0 && canDespawn())
		{
			setEntityDead();
		}
		else if (d3 < 1024.0)
		{
			entityAge = 0;
		}
	}
}

void EntityLiving::updateAITasks()
{
	++entityAge;
	despawnEntity();
	if (entitySenses != nullptr)
		entitySenses->clearSensingCache();
	targetTasks.onUpdateTasks();
	tasks.onUpdateTasks();
	if (navigator != nullptr)
		navigator->onUpdateNavigation();
	updateAITick();
	if (moveHelper != nullptr)
		moveHelper->onUpdateMoveHelper();
	if (lookHelper != nullptr)
		lookHelper->onUpdateLook();
	if (jumpHelper != nullptr)
		jumpHelper->doJump();
}

void EntityLiving::updateAITick()
{
}

void EntityLiving::updatePlayerActionState()
{
	entityAge++;
#if !PLATFORM_CACHE_NEAREST_PLAYER
	(void)worldObj->getClosestPlayerToEntity(this, -1.0);
#endif
	despawnEntity();
	moveStrafing = 0.0f;
	moveForward = 0.0f;
	float f = 8.0f;
	if (rand.nextFloat() < 0.02f)
	{
#if PLATFORM_CACHE_NEAREST_PLAYER
		EntityPlayer *entityplayer1 = getCachedNearestPlayerDistanceSq() < f * f ? getCachedNearestPlayer() : nullptr;
#else
		EntityPlayer *entityplayer1 = worldObj->getClosestPlayerToEntity(this, f);
#endif
		if (entityplayer1 != nullptr)
		{
			currentTarget = entityplayer1;
			numTicksToChaseTarget = 10 + rand.nextInt(20);
		}
		else
		{
			randomYawVelocity = (rand.nextFloat() - 0.5f) * 20.0f;
		}
	}
	if (currentTarget != nullptr)
	{
		faceEntity(currentTarget, 10.0f, (float)getVerticalFaceSpeed());
#if PLATFORM_FLOAT_ENTITY_AI_MATH
		const float targetDx = (float)(currentTarget->posX - posX);
		const float targetDy = (float)(currentTarget->posY - posY);
		const float targetDz = (float)(currentTarget->posZ - posZ);
		const float targetDistanceSq = targetDx * targetDx + targetDy * targetDy + targetDz * targetDz;
		if (numTicksToChaseTarget-- <= 0 || currentTarget->isDead || targetDistanceSq > f * f)
#else
		if (numTicksToChaseTarget-- <= 0 || currentTarget->isDead || currentTarget->getDistanceSqToEntity(this) > (double)(f * f))
#endif
		{
			currentTarget = nullptr;
		}
	}
	else
	{
		if (rand.nextFloat() < 0.05f)
		{
			randomYawVelocity = (rand.nextFloat() - 0.5f) * 20.0f;
		}
		rotationYaw += randomYawVelocity;
		rotationPitch = defaultPitch;
	}
	bool flag  = isInWater();
	bool flag1 = handleLavaMovement();
	if (flag || flag1)
	{
		isJumping = rand.nextFloat() < 0.8f;
	}
}

void EntityLiving::updateEntityActionState()
{
	updatePlayerActionState();
}


int_t EntityLiving::getVerticalFaceSpeed()
{
	return 40;
}

void EntityLiving::faceEntity(Entity *entity, float f, float f1)
{
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	const float d = (float)(entity->posX - posX);
	const float d2 = (float)(entity->posZ - posZ);
	float d1;
	if (entity->isLiving())
	{
		EntityLiving *entityliving = static_cast<EntityLiving *>(entity);
		d1 = (float)((posY + (double)getEyeHeight()) -
		             (entityliving->posY + (double)entityliving->getEyeHeight()));
	}
	else
	{
		d1 = (float)(((entity->boundingBox->minY + entity->boundingBox->maxY) * 0.5) -
		             (posY + (double)getEyeHeight()));
	}
	const float d3 = MathHelper::sqrt_float(d * d + d2 * d2);
	const float f2 = (std::atan2(d2, d) * 180.0f) / 3.14159274f - 90.0f;
	const float f3 = -(std::atan2(d1, d3) * 180.0f) / 3.14159274f;
#else
	double d  = entity->posX - posX;
	double d2 = entity->posZ - posZ;
	double d1;
	if (entity->isLiving())
	{
		EntityLiving *entityliving = static_cast<EntityLiving *>(entity);
		d1 = (posY + (double)getEyeHeight()) - (entityliving->posY + (double)entityliving->getEyeHeight());
	}
	else
	{
		d1 = (entity->boundingBox->minY + entity->boundingBox->maxY) / 2.0 - (posY + (double)getEyeHeight());
	}
	double d3 = MathHelper::sqrt_double(d * d + d2 * d2);
	float f2 = (float)((JavaMath::atan2(d2, d) * 180.0) / 3.1415927410125732) - 90.0f;
	float f3 = (float)(-((JavaMath::atan2(d1, d3) * 180.0) / 3.1415927410125732));
#endif
	rotationPitch = -updateRotation(rotationPitch, f3, f1);
	rotationYaw   =  updateRotation(rotationYaw,   f2, f);
}

bool EntityLiving::hasCurrentTarget()
{
	return currentTarget != nullptr;
}

Entity *EntityLiving::getCurrentTarget()
{
	return currentTarget;
}

float EntityLiving::updateRotation(float f, float f1, float f2)
{
	float f3;
	for (f3 = f1 - f; f3 < -180.0f; f3 += 360.0f) {}
	for (;             f3 >= 180.0f; f3 -= 360.0f) {}
	if (f3 >  f2) { f3 =  f2; }
	if (f3 < -f2) { f3 = -f2; }
	return f + f3;
}

void EntityLiving::onEntityDeath()
{
}

bool EntityLiving::getCanSpawnHere()
{
	return worldObj->checkIfAABBIsClear(boundingBox) && worldObj->getCollidingBoundingBoxes(this, boundingBox).size() == 0 && !worldObj->getIsAnyLiquid(boundingBox);
}

void EntityLiving::kill()
{
	attackEntityFrom(DamageSource::outOfWorld, 4);
}

float EntityLiving::getSwingProgress(float f)
{
	float f1 = swingProgress - prevSwingProgress;
	if (f1 < 0.0f)
	{
		f1++;
	}
	return prevSwingProgress + f1 * f;
}

Vec3D *EntityLiving::getPosition(float f)
{
	if (f == 1.0f)
	{
		return Vec3D::createVector(posX, posY, posZ);
	}
	double d  = prevPosX + (posX - prevPosX) * (double)f;
	double d1 = prevPosY + (posY - prevPosY) * (double)f;
	double d2 = prevPosZ + (posZ - prevPosZ) * (double)f;
	return Vec3D::createVector(d, d1, d2);
}

Vec3D *EntityLiving::getLookVec()
{
	return getLook(1.0f);
}

Vec3D *EntityLiving::getLook(float f)
{
	if (f == 1.0f)
	{
		float f1 = MathHelper::cos(-rotationYaw   * 0.017453292f - 3.1415927f);
		float f3 = MathHelper::sin(-rotationYaw   * 0.017453292f - 3.1415927f);
		float f5 = -MathHelper::cos(-rotationPitch * 0.017453292f);
		float f7 = MathHelper::sin(-rotationPitch * 0.017453292f);
		return Vec3D::createVector(f3 * f5, f7, f1 * f5);
	}
	float f2 = prevRotationPitch + (rotationPitch - prevRotationPitch) * f;
	float f4 = prevRotationYaw   + (rotationYaw   - prevRotationYaw)   * f;
	float f6 = MathHelper::cos(-f4 * 0.017453292f - 3.1415927f);
	float f8 = MathHelper::sin(-f4 * 0.017453292f - 3.1415927f);
	float f9 = -MathHelper::cos(-f2 * 0.017453292f);
	float f10 = MathHelper::sin(-f2 * 0.017453292f);
	return Vec3D::createVector(f8 * f9, f10, f6 * f9);
}

MovingObjectPosition *EntityLiving::rayTrace(double d, float f)
{
	Vec3D *vec3d  = getPosition(f);
	Vec3D *vec3d1 = getLook(f);
	Vec3D *vec3d2 = vec3d->addVector(vec3d1->xCoord * d, vec3d1->yCoord * d, vec3d1->zCoord * d);
	return worldObj->rayTraceBlocks(vec3d, vec3d2);
}

int_t EntityLiving::getMaxSpawnedInChunk()
{
	return 4;
}

ItemStack *EntityLiving::getHeldItem()
{
	return nullptr;
}

void EntityLiving::handleHealthUpdate(byte_t byte0)
{
	if (byte0 == 2)
	{
		field_704_R = 1.5f;
		heartsLife = heartsHalvesLife;
		hurtTime = maxHurtTime = 10;
		attackedAtYaw = 0.0f;
		worldObj->playSoundAtEntity(this, getHurtSound(), getSoundVolume(), rand.nextFloatDifference() * 0.2f + 1.0f);
		attackEntityFrom(DamageSource::generic, 0);
	}
	else if (byte0 == 3)
	{
		worldObj->playSoundAtEntity(this, getDeathSound(), getSoundVolume(), rand.nextFloatDifference() * 0.2f + 1.0f);
		health = 0;
		onDeath(DamageSource::generic);
	}
	else
	{
		Entity::handleHealthUpdate(byte0);
	}
}

bool EntityLiving::isPlayerSleeping()
{
	return false;
}

int_t EntityLiving::getItemIcon(ItemStack *itemstack)
{
	return getItemIcon(itemstack, 0);
}

int_t EntityLiving::getItemIcon(ItemStack *itemstack, int_t renderPass)
{
	(void)renderPass;
	return itemstack->getIconIndex();
}

void EntityLiving::func_48079_f(float yaw)
{
	rotationYawHead = yaw;
}

void EntityLiving::setPositionAndUpdate(double x, double y, double z)
{
	setLocationAndAngles(x, y, z, rotationYaw, rotationPitch);
}
